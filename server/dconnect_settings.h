#ifndef DCONNECT_SETTINGS_H_
#define DCONNECT_SETTINGS_H_

// ========================================================================== //
// NETWORK TYPES AND CONSTANTS
// ========================================================================== //

// UPACK.type //
#define DP_INFO 0
#define DP_CONNECT 1
#define DP_RECONNECT 2

#define DP_GAME 10
#define DP_GAME_INFO 11
#define DP_GAME_PREPARE 12
#define DP_GAME_BEGIN 13

#define DP_CLIENT_ACTION 20
#define DP_CLIENT_STOP 21




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
#define NET_REPEAT_CLIENT 2
#define NET_REPEAT_SERVER 4
#define NET_REPEAT_TRYES 128








#endif /* DCONNECT_SETTINGS_H_ */
