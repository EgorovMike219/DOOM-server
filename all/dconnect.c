#include "dconnect.h"








// ========================================================================== //
// INTERNAL FUNCTIONS, VARIABLES AND CONSTANTS
// ========================================================================== //

// Session net data
struct All_Net_data_t {
	int socket;
	struct sockaddr_in binder;
} D_NET_DATA;


// Client connection data
struct Client_Net_data_t {
	struct sockaddr_in s_addr;
} D_CLIENT_NET_DATA;




int d_compare_hr(const HR_ADDRESS *a, const HR_ADDRESS *b) {
	if (a ->port != b ->port) {
		return -1;
	}
	else if (strcmp(a ->ip, b ->ip) != 0) {
		return -1;
	}
	else {
		return 0;
	}
}




// Transform HR to sockaddr_in
int transform_from_hr_(const HR_ADDRESS* hr, struct sockaddr_in* sock) {
	bzero(sock, sizeof(*sock));
	if (inet_aton(hr ->ip, &(sock ->sin_addr)) < 0) {
		return -1;
	}
	else {
		sock ->sin_family = AF_INET;
		sock ->sin_port = htons(hr ->port);
		return 0;
	}
}




// Transform sockaddr_in to HR
int transform_to_hr_(const struct sockaddr_in* sock, HR_ADDRESS* hr) {
	strcpy(hr ->ip, inet_ntoa(sock ->sin_addr));
	hr ->port = ntohs(sock ->sin_port);
	return 0;
}








// ========================================================================== //
// CUSTOM NETWORK TYPES
// ========================================================================== //

void* make_UPACK (size_t data_size) {
	return malloc(UPACK_SIZE(data_size));
}








// ========================================================================== //
// COMMON NETWORK FUNCTIONS
// ========================================================================== //

void d_all_delay(float time) {
	float F_CLOCKS_PER_SEC = (float)CLOCKS_PER_SEC;
	float clk_start_sec = ((float)clock()) / F_CLOCKS_PER_SEC;
	float clk_sec = ((float)clock()) / F_CLOCKS_PER_SEC;
	while (clk_sec < clk_start_sec + time) {
		clk_sec = ((float)clock()) / F_CLOCKS_PER_SEC;
	}
}




int d_all_connect(int type) {
	if ((D_NET_DATA.socket = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
		fprintf(stderr, "Net failure: Socket creation\n");
		return -1;
	}

	bzero(&D_NET_DATA.binder, sizeof(D_NET_DATA.binder));
	D_NET_DATA.binder.sin_family = AF_INET;

	if (type == 0) {
		D_NET_DATA.binder.sin_port = htons(NET_PORT);
	}
	else {
		D_NET_DATA.binder.sin_port = htons(0);
	}

	D_NET_DATA.binder.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(D_NET_DATA.socket, (struct  sockaddr*) &D_NET_DATA.binder,
				sizeof(D_NET_DATA.binder)) < 0) {
		fprintf(stderr, "Net failure: Binding\n");
		close(D_NET_DATA.socket);
		return -1;
	}

	return 0;
}




void d_all_disconnect() {
	close(D_NET_DATA.socket);
}




int d_all_sendraw(const void *data, size_t data_length,
		const struct sockaddr_in *address, socklen_t address_length,
		size_t repeats) {
	size_t i;
	size_t failed_sends = 0;

	if ((data == NULL) || (address == NULL)) {
		fprintf(stderr, "SendRaw failure: NULL pointer\n");
		return -1;
	}
	if ((data_length == 0) || (address_length == 0)) {
		fprintf(stderr, "SendRaw failure: Incorrect length\n");
		return -1;
	}
	if (data_length >= UDP_MAX_PACKET_SIZE) {
		fprintf(stderr, "SendRaw failure: Data too long\n");
		return -1;
	}


	for (i = 0; i < repeats; i++) {
		if (sendto(D_NET_DATA.socket,
				data, data_length,
				0,
				(struct sockaddr*) address, address_length) < 0) {
			failed_sends += 1;
		}
	}
	if (failed_sends == repeats) {
		return -1;
	}

	return 0;
}




int d_all_recvraw(void* data, size_t data_length,
		struct sockaddr_in* address, socklen_t* address_length) {
	ssize_t recieved_size;
	if (address_length != NULL) {
		*address_length = sizeof(struct sockaddr_in);
	}
	if ((recieved_size = recvfrom(D_NET_DATA.socket,
			data, data_length,
			MSG_DONTWAIT,
			(struct sockaddr*) address, address_length)) < 0) {
		if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
			errno = 0;
			return -2;
		}
		else {
			return -1;
		}
	}
	else {
		return recieved_size;
	}
}








// ========================================================================== //
// CLIENT NETWORK FUNCTIONS
// ========================================================================== //

int d_client_connect(HR_ADDRESS server, int reconnect) {
	int recvraw_tryes = 0;
	int last_out_code = 0;

	UPACK_HEAD* pack_T = make_UPACK(1);

	
	if ((reconnect != 1) && (reconnect != 2)) {
		reconnect = 0;
	}

	if ((reconnect <= 1) &&
			(transform_from_hr_(&server, &(D_CLIENT_NET_DATA.s_addr)) < 0)) {
		fprintf(stderr, "Connect failure: Incorrect IP address\n");
		free(pack_T);
		return -1;
	}


	if (reconnect == 0) {
		pack_T ->type = DP_CONNECT;
	}
	else {
		pack_T ->type = DP_RECONNECT;
	}
	pack_T ->stamp = DP_S_ASK;

	while (1) {
		for (recvraw_tryes = 0; recvraw_tryes < NET_CLIENT_RECVTRYES;
				recvraw_tryes++) {
			if (d_client_send(pack_T, UPACK_SIZE(1), NET_REPEAT_ONE) < 0) {
				fprintf(stderr, "Connect failure: Something has gone wrong\n");
				free(pack_T);
				d_all_disconnect();
				return -1;
			}
			
			d_all_delay(NET_PING);
			
			last_out_code = d_client_get(pack_T, UPACK_SIZE(1), 0);

			if (last_out_code == -1) {
				fprintf(stderr, "Connect failure: Something has gone wrong\n");
				free(pack_T);
				d_all_disconnect();
				return -1;
			}
			else if (last_out_code == 0) {
				break;
			}
		}
		if (recvraw_tryes == NET_CLIENT_RECVTRYES) {
			fprintf(stderr, "Connect failure: Server not responding\n");
			free(pack_T);
			d_all_disconnect();
			return -1;
		}

		// Check server response
		if (((pack_T ->type == DP_CONNECT) && (reconnect == 0)) ||
				((pack_T ->type == DP_RECONNECT) && (reconnect > 0))) {
			if (pack_T ->stamp == DP_S_SUCCESS) {
				free(pack_T);
				return 0;
			}
			else if (pack_T ->stamp == DP_S_ERROR) {
				fprintf(stderr, "Connect failure: Rejected by server\n");
				free(pack_T);
				d_all_disconnect();
				return -1;
			}
			else {
				fprintf(stderr, "Connect failure: Unusual server response\n");
				free(pack_T);
				d_all_disconnect();
				return -1;
			}
		}
	}
}




int d_client_send(const void* data, size_t data_length, size_t repeats) {
	return d_all_sendraw(data, data_length,
			&(D_CLIENT_NET_DATA.s_addr), sizeof(struct sockaddr_in),
			repeats);
}




int d_client_get(void* data, size_t data_length, TICK_TYPE tick) {
	int last_out_code = 0;
	int recvraw_tryes = 0;

	struct sockaddr_in addr_T;
	socklen_t addl_T = sizeof(struct sockaddr_in);

	UPACK_HEAD* recieved = data;
	
	HR_ADDRESS recieved_hr;
	HR_ADDRESS server_hr;
	

	if (data == NULL) {
		fprintf(stderr, "Get failure: NULL pointer\n");
		return -1;
	}
	
	transform_to_hr_(&D_CLIENT_NET_DATA.s_addr, &server_hr);

	while (1) {
		for (recvraw_tryes = 0; recvraw_tryes < NET_CLIENT_RECVTRYES;
				recvraw_tryes++) {
			last_out_code = d_all_recvraw(data, data_length, &addr_T, &addl_T);

			if (last_out_code >= 0) {
				break;
			}
			else if (last_out_code == -1) {
				return -1;
			}
		}
		if (recvraw_tryes == NET_CLIENT_RECVTRYES) {
			return -2;
		}

		if (tick > 0) {
			if (recieved ->stamp < tick) {
				continue;
			}
		}
		
		transform_to_hr_(&addr_T, &recieved_hr);
		if (d_compare_hr(&server_hr, &recieved_hr) != 0) {
			continue;
		}

		return 0;
	}
}








// ========================================================================== //
// SERVER NETWORK FUNCTIONS
// ========================================================================== //

int d_server_send(const void* data, size_t data_length,
		HR_ADDRESS destination, size_t repeats) {
	struct sockaddr_in temp_sock;
	if (transform_from_hr_(&destination, &temp_sock) < 0) {
		fprintf(stderr, "Send failure: Incorrect IP\n");
		return -1;
	}
	return d_all_sendraw(data, data_length,
			&temp_sock,
			sizeof(temp_sock),
			repeats);
}




int d_server_get(int mode, void* data, size_t data_length,
		HR_ADDRESS* departure) {
	int last_out_code = 0;

	struct sockaddr_in cli_addr;
	socklen_t cli_addl = sizeof(struct sockaddr_in);

	UPACK_HEAD* received = data;

	while (1) {
		last_out_code = d_all_recvraw(data, data_length,
				&cli_addr, &cli_addl);
		if (last_out_code < 0) {
			return last_out_code;
		}

		if (received ->type == DP_GAME) {
			// A server cannot get GAME datagrams, it only sends them
			continue;
		}

		if (mode == 0) {
			break;
		}
		else if (mode == 1) {
			if (received ->type == DP_CLIENT_ACTION) {
				continue;
			}
			else {
				break;
			}
		}
	}

	if (transform_to_hr_(&cli_addr, departure) < 0) {
		return -1;
	}
	else {
		return 0;
	}
}







