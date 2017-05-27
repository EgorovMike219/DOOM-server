#include <string.h>
#include "dconnect.h"

#define NET_MAX_RESOLVABLE 256




int D_IP_ID[NET_MAX_RESOLVABLE];
HR_ADDRESS D_ID_IP[NET_MAX_RESOLVABLE];




int IpToId(const char* ip) {
	int len = strlen(ip);
	int i = len - 1;
	int st = 1;
	int id = 0;
	while (ip[i] != '.') {
		id += st*((int)(ip[i]) - 48);
		st *= 10;
		i--;
	}
	return id;
}



void initialize_ipToId() {
	int i;
	for (i = 0; i < NET_MAX_RESOLVABLE; i++) {
		D_IP_ID[i] = -1;
		D_ID_IP[i].port = 0;
	}
}



int check_hr(HR_ADDRESS given) {
	int ip = IpToId(given.ip);
	if (ip >= NET_MAX_RESOLVABLE) {
		fprintf(stderr, "Unresolvable ip address\n");
		return -1;
	}
	if (D_IP_ID[ip] == -1) {
		return 0;
	}
	else {
		return 1;
	}
}



int add_hr(HR_ADDRESS given, int id) {
	int ip = IpToId(given.ip);
	if ((id >= NET_MAX_RESOLVABLE) || (ip >= NET_MAX_RESOLVABLE)) {
		fprintf(stderr, "Unresolvable ip address or id\n");
		return -1;
	}
	D_IP_ID[ip] = id;
	D_ID_IP[id] = given;
	return 0;
}



// Returns id
int resolve_hr(HR_ADDRESS given, int* id) {
	int ip = IpToId(given.ip);
	if (ip >= NET_MAX_RESOLVABLE) {
		fprintf(stderr, "Unresolvable ip address\n");
		return -1;
	}
	*id = D_IP_ID[ip];
	return 0;
}



int resolve_id(HR_ADDRESS* to_place, int id) {
	if (id >= NET_MAX_RESOLVABLE) {
		fprintf(stderr, "Unresolvable id\n");
		return -1;
	}
	*to_place = D_ID_IP[id];
	return 0;
}



int remove_id(int id) {
	if (id >= NET_MAX_RESOLVABLE) {
		fprintf(stderr, "Unresolvable ip address or id\n");
		return -1;
	}
	D_IP_ID[IpToId(D_ID_IP[id].ip)] = -1;
	D_ID_IP[id].port = 0;
	return 0;
}



