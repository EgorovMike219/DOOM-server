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




// Compare two socket addresses. Return -1, if not equal, otherwise return 0
int compare_sockaddr_in_(const struct sockaddr_in* a, const socklen_t al,
		const struct sockaddr_in* b, const socklen_t bl) {
	if (bl != al) {
		return -1;
	}
	if (memcmp(a, b, al) != 0) {
		return -1;
	}
	return 0;
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
	return malloc(sizeof(int16_t) + sizeof(TICK_TYPE) + data_size);
}








// ========================================================================== //
// COMMON NETWORK FUNCTIONS
// ========================================================================== //

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




int d_all_sendraw(const void* data, size_t data_length,
		const struct sockaddr_in* address, size_t address_length,
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
		const struct sockaddr_in* address, socklen_t* address_length) {
	ssize_t recieved_size;
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

int d_client_connect(HR_ADDRESS server) {
	const int MAX_RECVRAW_TRYES = 32768;
	int last_out_code = 0;
	int recvraw_tryes = 0;

	UPACK_HEAD pack_T;
	struct sockaddr_in pack_T_addr;
	socklen_t pack_T_addl;


	if (transform_from_hr_(&server, &(D_CLIENT_NET_DATA.s_addr)) < 0) {
		fprintf(stderr, "Connect failure: Incorrect IP address\n");
		return -1;
	}

	pack_T.type = DP_CONNECT;
	pack_T.stamp = DP_S_ASK;

	while (1) {
		for (recvraw_tryes = 0; recvraw_tryes < MAX_RECVRAW_TRYES;
				recvraw_tryes++) {
			if (d_all_sendraw(&pack_T, sizeof(pack_T),
					&(D_CLIENT_NET_DATA.s_addr),
					sizeof(D_CLIENT_NET_DATA.s_addr),
					NET_REPEAT_ONE) < 0) {
				fprintf(stderr, "Connect failure: Something has gone wrong\n");
				return -1;
			}
			last_out_code = d_all_recvraw(&pack_T, sizeof(pack_T),
							&pack_T_addr, &pack_T_addl);

			if (last_out_code == -1) {
				fprintf(stderr, "Connect failure: Something has gone wrong\n");
				return -1;
			}
			else if (last_out_code >= 0) {
				break;
			}
		}
		if (last_out_code == -2) {
			fprintf(stderr, "Connect failure: Server not responding\n");
			return -1;
		}

		if (compare_sockaddr_in_(&pack_T_addr, pack_T_addl,
				&(D_CLIENT_NET_DATA.s_addr),
				sizeof(D_CLIENT_NET_DATA.s_addr)) != 0) {
			continue;
		}

		// Server tested, correct packet recieved
		if (pack_T.type == DP_CONNECT) {
			if (pack_T.stamp == DP_S_SUCCESS) {
				return 0;
			}
			else if (pack_T.stamp == DP_S_ERROR) {
				fprintf(stderr, "Connect failure: Rejected by server\n");
				return -1;
			}
			else {
				fprintf(stderr, "Connect failure: Unknown server error\n");
				return -1;
			}
		}
	}
}




int d_client_send(const void* data, size_t data_length, size_t repeats) {
	return d_all_sendraw(data, data_length,
			&(D_CLIENT_NET_DATA.s_addr), sizeof(D_CLIENT_NET_DATA.s_addr),
			repeats);
}




int d_client_get(void* data, size_t data_length, TICK_TYPE tick) {
	const int MAX_RECVRAW_TRYES = 64;
	int last_out_code = 0;
	int recvraw_tryes = 0;

	struct sockaddr_in addr_T;
	socklen_t addl_T;

	UPACK_HEAD* recieved = data;


	if (data == NULL) {
		fprintf(stderr, "Get failure: NULL pointer\n");
		return -1;
	}

	while (1) {
		for (recvraw_tryes = 0; recvraw_tryes < MAX_RECVRAW_TRYES;
				recvraw_tryes++) {
			last_out_code = d_all_recvraw(data, data_length, &addr_T, &addl_T);

			if (last_out_code == -1) {
				return -1;
			}
			else if (last_out_code == 0) {
				break;
			}
		}
		if (last_out_code == -2) {
			return -2;
		}

		if (tick > 0) {
			if ((recieved ->type != DP_GAME) ||
					(recieved ->stamp < tick)) {
				continue;
			}
		}

		if (compare_sockaddr_in_(&addr_T, addl_T,
				&(D_CLIENT_NET_DATA.s_addr),
				sizeof(D_CLIENT_NET_DATA.s_addr)) != 0) {
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
	socklen_t cli_addl;

	UPACK_HEAD* recieved = data;

	while (1) {
		last_out_code = d_all_recvraw(data, data_length,
				&cli_addr, &cli_addl);
		if (last_out_code < 0) {
			return last_out_code;
		}

		if (recieved ->type == DP_GAME) {
			// A server cannot get GAME datagrams, it only sends them
			continue;
		}

		if (mode == 0) {
			break;
		}
		else if (mode == 1) {
			if (recieved ->type == DP_CLIENT_ACTION) {
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







