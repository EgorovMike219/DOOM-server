#ifndef DOOM_SERVER_IPTOID_H
#define DOOM_SERVER_IPTOID_H

#include <string.h>


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


#endif //DOOM_SERVER_IPTOID_H