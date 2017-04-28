#include "dconnect.h"




int d_all_connect(int type, int connections) {
	int sockfd;
	struct sockaddr_in binder;


	// Essential net connection operations
	if ((type > 2) || (type < 0)) {
		perror("Net failure: Incorrect connection type\n");
		return -1;
	}

	if ((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("Net failure: Socket creation failed\n");
		return -1;
	}

	bzero(&binder, sizeof(binder));
	binder.sin_family = AF_INET;
	if (type == 0) {
		binder.sin_port = htons(NET_SERVER_PORT);
	}
	else {
		binder.sin_port = htons(0);
	}
	binder.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(sockfd, (struct  sockaddr*) &binder,
				sizeof(binder)) < 0) {
		perror("Net failure: Binding\n");
		close(sockfd);
		return -1;
	}


	// Type-specific operations
	if (type == 0) {
		if (connections < 0) {
			perror("Net failure: Incorrect number of connections\n");
			close(sockfd);
			return -1;
		}
		if ((D_SERVER_NET_DATA = malloc(sizeof(struct Server_Net_data_t) *
				connections)) < 0) {
			perror("Net failure: Malloc\n");
			close(sockfd);
			return -1;
		}
	}


	D_NET_DATA.socket = sockfd;
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
	const size_t UDP_MAX_PACKET_SIZE = 65536;
	int FLAGS = 0;
	size_t i;
	size_t failed_sends = 0;

	if ((data == NULL) || (address == NULL)) {
		perror("SendRaw failure: NULL pointer\n");
		return -1;
	}
	if ((data_length == 0) || (address_length == 0)) {
		perror("SendRaw failure: Incorrect length\n");
		return -1;
	}
	if (data_length >= UDP_MAX_PACKET_SIZE) {
		perror("SendRaw failure: Data too long\n");
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
	if (failed_sends == repeats) {
//		perror("SendRaw warning: too many failed sends");
//		return -2;
		return -1;
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
	D_CLIENT_NET_DATA.s_addr.sin_port = NET_SERVER_PORT;
	inet_aton(ip, &(D_CLIENT_NET_DATA.s_addr.sin_addr));

	size_t pack_1_len = sizeof(int8_t) * 2 + sizeof(int8_t);
	int8_t *pack_1 = malloc(pack_1_len);
	pack_1[0] = 2;
	pack_1[1] = 1;
	pack_1[2] = 0;

	size_t pack_R_len = sizeof(int8_t) * 2 + sizeof(int8_t);
	int8_t *pack_R = malloc(pack_R_len);

	struct sockaddr_in pack_R_addr;
	socklen_t pack_R_addl;

	d_all_sendraw((void*)pack_1, pack_1_len,
			&(D_CLIENT_NET_DATA.s_addr), sizeof(D_CLIENT_NET_DATA.s_addr),
			NET_USUAL_REPEAT);

	while (1) {
		d_all_recvraw((void*)pack_R, pack_R_len,
				&pack_R_addr, &pack_R_addl);

		if (pack_R_addl != sizeof(D_CLIENT_NET_DATA.s_addr.sin_addr)) {
			continue;
		}
		if (!memcmp(&pack_R_addr, &(D_CLIENT_NET_DATA.s_addr.sin_addr),
				pack_R_addl)) {
			continue;
		}

		if ((pack_R[0] == 2) && (pack_R[1] == 1)) {
			if (pack_R[2] > 0) {
				return 0;
			}
			else {
				return -1;
			}
		}
	}
}









