#ifndef DOOM_SERVER_IPTOID_H
#define DOOM_SERVER_IPTOID_H

#include <string.h>
#include "game.h"

#define NET_MAX_RESOLVABLE 255




int D_ID_IP[NET_MAX_RESOLVABLE];
int D_IP_ID[NET_MAX_RESOLVABLE];




int IpToId(char* ip) {
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



int check_hr(const HR_ADDRESS* given) {
	int ip = IpToId(given ->ip);
	int
}



int add_hr(const HR_ADDRESS* given, int id) {

}



// Returns id
int resolve_hr(const HR_ADDRESS* given, int* id) {

}



int resolve_id(HR_ADDRESS* to_place, int id) {

}



int remove_id(int id) {

}



#endif //DOOM_SERVER_IPTOID_H
