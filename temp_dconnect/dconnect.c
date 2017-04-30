#include "dconnect.h"




/* Insides */

#define UDP_MAX_PACKET_SIZE 65536

#define PACK_C_ASK 0
#define PACK_C_SUCCESS 1
#define PACK_C_ERROR -1

#define PACK_CT_CLIENTCONNECT 1

/* ------- */








/// Session net data
struct All_Net_data_t {
	int socket;
	struct sockaddr_in binder;
} D_NET_DATA;


/// Client connection data
struct Client_Net_data_t {
	struct sockaddr_in s_addr;
} D_CLIENT_NET_DATA;


/// Server connection database
struct Server_Net_data_t {
	struct sockaddr_in s_addr;

} *D_SERVER_NET_DATA;

ID_TYPE D_SERVER_NET_DATA_LEN;
ID_TYPE D_SERVER_NET_DATA_NOW;








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



int d_all_connect(int type, size_t connections) {
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
		D_NET_DATA.binder.sin_port = htons(NET_PORT);
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
		D_SERVER_NET_DATA_LEN = (ID_TYPE)connections;
		D_SERVER_NET_DATA_NOW = 0;
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
	// TODO: Use PACK_C_HEAD instead of arrays

	bzero(&(D_CLIENT_NET_DATA.s_addr), sizeof(D_CLIENT_NET_DATA.s_addr));
	D_CLIENT_NET_DATA.s_addr.sin_family = AF_INET;
	D_CLIENT_NET_DATA.s_addr.sin_port = htons(NET_PORT);
	if (inet_aton(ip, &(D_CLIENT_NET_DATA.s_addr.sin_addr)) == 0) {
		fprintf(stderr, "Connection error: Incorrect server IP address\n");
		return -1;
	}

	size_t pack_T_len = sizeof(int8_t) * 2 + sizeof(int8_t);
	int8_t *pack_T = malloc(pack_T_len);
	pack_T[0] = PACK_C;
	pack_T[1] = PACK_CT_CLIENTCONNECT;
	pack_T[2] = PACK_C_ASK;

	d_all_sendraw(pack_T, pack_T_len,
			&(D_CLIENT_NET_DATA.s_addr), sizeof(D_CLIENT_NET_DATA.s_addr),
			NET_REPEAT);

	struct sockaddr_in pack_T_addr;
	socklen_t pack_T_addl;

	while (1) {
		d_all_recvraw(pack_T, pack_T_len,
				&pack_T_addr, &pack_T_addl);

		if (compare_sockaddr_in_(&pack_T_addr, pack_T_addl,
				&(D_CLIENT_NET_DATA.s_addr),
				sizeof(D_CLIENT_NET_DATA.s_addr)) != 0) {
			continue;
		}

		if ((pack_T[0] == PACK_C) && (pack_T[1] == PACK_CT_CLIENTCONNECT)) {
			if (pack_T[2] == PACK_C_SUCCESS) {
				free(pack_T);
				return 0;
			}
			else {
				free(pack_T);
				return -1;
			}
		}
	}
}




int d_client_send(const void* data, size_t data_length) {
	return d_all_sendraw(data, data_length,
			&(D_CLIENT_NET_DATA.s_addr), sizeof(D_CLIENT_NET_DATA.s_addr),
			NET_REPEAT);
}




int d_client_get(void* data, size_t data_length, TICK_TYPE tick) {
	void* buff = malloc(UDP_MAX_PACKET_SIZE);
	struct sockaddr_in addr_buff;
	socklen_t addr_length;
	size_t recieved_length;

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
			PACK_USUAL_HEAD* clip_recieved = buff;
			if ((clip_recieved ->type != PACK_USUAL) ||
					(clip_recieved ->tick < tick)) {
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




int d_server_send(ID_TYPE id, const void* data, size_t data_length) {
	if (id >= D_SERVER_NET_DATA_NOW) {
		fprintf(stderr, "Send error: incorrect ID");
		return -1;
	}
	return d_all_sendraw(data, data_length,
			&(D_SERVER_NET_DATA[id].s_addr),
			sizeof(D_SERVER_NET_DATA[id].s_addr),
			NET_REPEAT);
}




int d_server_get(void* data, size_t data_length, ID_TYPE* id_ptr, int mode) {
	size_t buff_len = UDP_MAX_PACKET_SIZE;
	void* buff = malloc(buff_len);
	int buff_recieved;
	struct sockaddr_in addr;
	socklen_t addr_len;
	int id_found;


	int return_value;
	ID_TYPE return_id;

	ID_TYPE i;

	while (1) {
		if ((buff_recieved =  d_all_recvraw(buff, buff_len, &addr, &addr_len))
				< 0) {
			return -1;
		}
		else if (buff_recieved > data_length) {
			continue;
		}

		if (mode == 0) {
			PACK_USUAL_HEAD* head = buff;
			if (head ->type == PACK_USUAL) {
				return_value = 0;
			}
			else if (head ->type == PACK_SPECIAL) {
				return_value = 1;
			}
			else if (head ->type == PACK_C) {
				// TODO: May be changed to check some additional parameters
				continue;
			}
			else {
				return -1;
			}

			id_found = 0;
			for (i = 0; i < D_SERVER_NET_DATA_NOW; i++) {
				if (memcmp(&(D_SERVER_NET_DATA[i]), &addr, addr_len) == 0) {
					return_id = i;
					id_found = 1;
					break;
				}
			}
			if (id_found == 1) {
				break;
			}
			else {
				continue;
			}
		}
		else if (mode == 1) {
			PACK_SPECIAL_HEAD* head = buff;
			if (head ->type == PACK_USUAL) {
				continue;
			}
			else if (head ->type == PACK_SPECIAL) {
				return_value = 1;
			}
			else if (head ->type == PACK_C) {
				continue;
			}
			else {
				return -1;
			}

			id_found = 0;
			for (i = 0; i < D_SERVER_NET_DATA_NOW; i++) {
				if (memcmp(&(D_SERVER_NET_DATA[i]), &addr, addr_len) == 0) {
					return_id = i;
					id_found = 1;
					break;
				}
			}
			if (id_found == 1) {
				break;
			}
			else {
				continue;
			}
		}
		else if (mode == 2) {
			PACK_C_HEAD* head = buff;
			if (head ->type == PACK_USUAL) {
				continue;
			}
			else if (head ->type == PACK_SPECIAL) {
				return_value = 1;
			}
			else if (head ->type == PACK_C) {
				return_value = 0;
			}
			else {
				return -1;
			}

			id_found = 0;
			for (i = 0; i < D_SERVER_NET_DATA_NOW; i++) {
				if (memcmp(&(D_SERVER_NET_DATA[i]), &addr, addr_len) == 0) {
					return_id = i;
					id_found = 1;
					break;
				}
			}
			if (return_value == 1) {
				if (id_found == 1) {
					break;
				}
				else {
					continue;
				}
			}
			else {
				if (id_found == 1) {
					continue;
				}
				// Add new client
				if (D_SERVER_NET_DATA_NOW == D_SERVER_NET_DATA_LEN) {
					// TODO: Send busy package
					continue;
				}
				D_SERVER_NET_DATA[D_SERVER_NET_DATA_NOW].s_addr = addr;
				D_SERVER_NET_DATA_NOW += 1;
			}
		}
	}

	memcpy(data, buff, buff_recieved);
	*id_ptr = return_id;
	return return_value;
}




int d_server_manageid(int op, ID_TYPE id) {
	if (id >= D_SERVER_NET_DATA_NOW) {
		return 1;
	}

	if (op == 0) {
		return 0;
	}
	else if (op == -1) {
		if (D_SERVER_NET_DATA_NOW == 0) {
			return -1;
		}

		if (D_SERVER_NET_DATA_NOW > 1) {
			D_SERVER_NET_DATA[id] = D_SERVER_NET_DATA[D_SERVER_NET_DATA_NOW];
			D_SERVER_NET_DATA_NOW -= 1;
			return 0;
		}
		else {
			D_SERVER_NET_DATA_NOW -= 1;
			return 2;
		}

	}

	return 0;
}









