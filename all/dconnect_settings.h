#ifndef DCONNECT_SETTINGS_H
#define DCONNECT_SETTINGS_H

// ========================================================================== //
// NETWORK TYPES AND CONSTANTS
// ========================================================================== //

// UPACK.type //
#define DP_NONE 0

#define DP_SERVICE 1

#define DP_CONNECT 2

#define DP_RECONNECT 3


#define DP_GAME 10
#define SZ_GAME (UPACK_SIZE(CLIENT_FIELD_AREA + LENGTH_NET_INFO))


#define DP_GAME_PREPARE 11

#define DP_GAME_BEGIN 12

#define DP_GAME_OVER 13

#define DP_CLIENT_ACTION 20
#define SZ_CLIENT_ACTION (UPACK_SIZE(LENGTH_NET_COMMAND))


#define DP_MESSAGE 31
#define SZ_PURE_MESSAGE 250 + 1




// UPACK.stamp //
#define DP_S_SUCCESS 0

#define DP_S_ASK 1

#define DP_S_ERROR 2








// ========================================================================== //
// NETWORK SETTINGS
// ========================================================================== //

#define UDP_MAX_PACKET_SIZE 65536


#define NET_PORT 59200

#define NET_PING 0.250


#define NET_REPEAT_ONE 1

#define NET_REPEAT_CLIENT 1

#define NET_REPEAT_SERVER 4


#define NET_CLIENT_RECVTRYES 128








#endif //DCONNECT_SETTINGS_H
