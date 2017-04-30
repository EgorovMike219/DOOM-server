#include "dconnect.h"

const size_t UDP_MAX_PACKET_SIZE = 65536;




/* Compare two socket addresses. Return -1, if not equal, otherwise return 0 */
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



int d_all_connect(int type, int connections) {
	int sockfd;


	// Essential net connection operations
	if ((type > 2) || (type < 0)) {
		fprintf(stderr, "Net failure: Incorrect connection type\n");
		return -1;
	}

	if ((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
		fprintf(stderr, "Net failure: Socket creation failed\n");
		return -1;
	}

	bzero(&D_NET_DATA.binder, sizeof(D_NET_DATA.binder));
	D_NET_DATA.binder.sin_family = AF_INET;
	if (type == 0) {
		D_NET_DATA.binder.sin_port = htons(NET_SERVER_PORT);
	}
	else {
		D_NET_DATA.binder.sin_port = htons(0);
	}
	D_NET_DATA.binder.sin_addr.s_addr = htonl(INADDR_ANY);

	D_NET_DATA.socket = sockfd;

	if (bind(sockfd, (struct  sockaddr*) &D_NET_DATA.binder,
				sizeof(D_NET_DATA.binder)) < 0) {
		fprintf(stderr, "Net failure: Binding\n");
		close(sockfd);
		return -1;
	}


	// Type-specific operations
	if (type == 0) {
		if (connections < 0) {
			fprintf(stderr, "Net failure: Incorrect number of connections\n");
			close(sockfd);
			return -1;
		}
		if ((D_SERVER_NET_DATA = malloc(sizeof(struct Server_Net_data_t) *
				connections)) < 0) {
			fprintf(stderr, "Net failure: Malloc\n");
			close(sockfd);
			return -1;
		}
	}
	return 0;
}




void d_all_disconnect(int type) {
	close(D_NET_DATA.socket);
	if (type == 0) {
		free(D_SERVER_NET_DATA);
	}
}




int d_all_sendraw(const void* data, size_t data_length,
		const struct sockaddr_in* address, size_t address_length,
		size_t repeats) {
	int FLAGS = 0;
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
				FLAGS,
				(struct sockaddr*) address, address_length) < 0) {
			failed_sends += 1;
		}
	}
	if (failed_sends) {
//		printf(stderr, "SendRaw warning: too many failed sends");
//		return -2;
		return failed_sends;
	}

	return 0;
}




int d_all_recvraw(void* data, size_t data_length,
		const struct sockaddr_in* address, socklen_t* address_length) {
	ssize_t recieved_size;
	int FLAGS = 0;
	if ((recieved_size = recvfrom(D_NET_DATA.socket,
			data, data_length,
			FLAGS,
			(struct sockaddr*) address, address_length)) < 0) {
		return -1;
	}
	else {
		return recieved_size;
	}
}




int d_client_connect(const char* ip) {
	bzero(&(D_CLIENT_NET_DATA.s_addr), sizeof(D_CLIENT_NET_DATA.s_addr));
	D_CLIENT_NET_DATA.s_addr.sin_family = AF_INET;
	D_CLIENT_NET_DATA.s_addr.sin_port = htons(NET_SERVER_PORT);
	if (inet_aton(ip, &(D_CLIENT_NET_DATA.s_addr.sin_addr)) == 0) {
		fprintf(stderr, "Net error: Incorrect server IP address\n");
		return -1;
	}

	size_t pack_1_len = sizeof(int8_t) * 2 + sizeof(int8_t);
	int8_t *pack_1 = malloc(pack_1_len);
	pack_1[0] = 2;
	pack_1[1] = 1;
	pack_1[2] = 0;

	size_t pack_R_len = sizeof(int8_t) * 2 + sizeof(int8_t);
	int8_t *pack_R = malloc(pack_R_len);

	struct sockaddr_in pack_R_addr;
	socklen_t pack_R_addl;

	d_all_sendraw(pack_1, pack_1_len,
			&(D_CLIENT_NET_DATA.s_addr), sizeof(D_CLIENT_NET_DATA.s_addr),
			NET_USUAL_REPEAT);

	while (1) {
		d_all_recvraw(pack_R, pack_R_len,
				&pack_R_addr, &pack_R_addl);

		if (compare_sockaddr_in_(&pack_R_addr, pack_R_addl,
				&(D_CLIENT_NET_DATA.s_addr),
				sizeof(D_CLIENT_NET_DATA.s_addr)) != 0) {
			continue;
		}

		if ((pack_R[0] == 2) && (pack_R[1] == 1)) {
			if (pack_R[2] > 0) {
				free(pack_1);
				free(pack_R);
				return 0;
			}
			else {
				free(pack_1);
				free(pack_R);
				return -1;
			}
		}
	}
}




int d_client_send(const void* data, size_t data_length) {
	return d_all_sendraw(data, data_length,
			&(D_CLIENT_NET_DATA.s_addr), sizeof(D_CLIENT_NET_DATA.s_addr),
			NET_USUAL_REPEAT);
}




int d_client_get(void* data, size_t data_length, TICK_TYPE tick) {
	void* buff = malloc(UDP_MAX_PACKET_SIZE);
	struct sockaddr_in addr_buff;
	socklen_t addr_length;
	size_t recieved_length;

	struct Client_Net_packet_t clip;
	if (tick > 0) {
		clip.type = 0;
		clip.tick = tick;
	}
	while (1) {
		if ((recieved_length = d_all_recvraw(buff, UDP_MAX_PACKET_SIZE,
				&addr_buff, &addr_length)) < 0) {
			return -1;
		}
		if (compare_sockaddr_in_(&addr_buff, addr_length,
				&(D_CLIENT_NET_DATA.s_addr),
				sizeof(D_CLIENT_NET_DATA.s_addr)) != 0) {
			continue;
		}
		if (tick > 0) {
			if (memcmp(buff, &clip, sizeof(clip)) != 0) {
				continue;
			}
		}
		if (recieved_length > data_length) {
			return -1;
		}
		memcpy(data, buff, recieved_length);
		return 0;
	}
}















