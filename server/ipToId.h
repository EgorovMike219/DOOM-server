#ifndef IPTOID_INCLUDED
#define IPTOID_INCLUDED

#include <string.h>

#include "../all/dconnect.h"




#define NET_MAX_RESOLVABLE 256




int D_IP_ID[NET_MAX_RESOLVABLE];
HR_ADDRESS D_ID_IP[NET_MAX_RESOLVABLE];




int IpToId(const char* ip);


void initialize_ipToId();


int check_hr(HR_ADDRESS given);


int add_hr(HR_ADDRESS given, int id);



// Returns id
int resolve_hr(HR_ADDRESS given, int* id);


int resolve_id(HR_ADDRESS* to_place, int id);


int remove_id(int id);




#endif /* IPTOID_INCLUDED */
