#ifndef DCONNECT_INCLUDED
#define DCONNECT_INCLUDED

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "dconnect_settings.h"

// TODO: Remove from this file (not only network-related)
#define TICK_TYPE uint64_t








// ========================================================================== //
// CUSTOM NETWORK TYPES
// ========================================================================== //

/*
 * Human-readable client address
 *
 * @note 'port' is stored in local byte order
 */
typedef struct All_Net_clientaddr_t {
	char ip[16];
	in_port_t port;
} HR_ADDRESS;


/*
 * Universal packet header
 *
 * @var type: packet type and operation it performs. Use predefined constants!
 * @var stamp: meta-information about this packet
 * @var data
 */
typedef struct All_Net_packheader_t {
	int16_t type;
	TICK_TYPE stamp;
	char data[1];
} UPACK_HEAD;

// Return UPACK size to place 'i' chars into data
#define UPACK_SIZE(i) (sizeof(int16_t) + sizeof(TICK_TYPE) + i)

/*
 * Create UPACK_HEAD using malloc, where data will be data[data_size]
 *
 * @param data_size
 *
 * @return pointer to UPACK_HEAD structure
 */
void* make_UPACK(size_t data_size);








// ========================================================================== //
// COMMON NETWORK FUNCTIONS
// ========================================================================== //

/*
 * Delay execution
 *
 * @param time: Time (in seconds) to delay execution for
 */
void d_all_delay(float time);


/*
 * Create an UDP socket and bind it.
 *
 * @param type: Type of connection required
 * 				= 0: server
 * 				= 1: client
 * 				= 2: statistics
 *
 * @return 0 if successful
 * @return -1 for errors
 *
 * @note Execute this before using any other dconnect.h functions
 */
int d_all_connect(int type);


/*
 * Safely close a connection. Can be reopened using d_all_connect
 */
void d_all_disconnect();


/*
 * Send raw data using UDP protocol
 * Wrapper for <socket.h>::sendto()
 *
 * @param data
 * @param data_length: 'sizeof(data)'
 * @param address
 * @param address_length: 'sizeof(address)'
 * @param repeats: Send UDP packet this number of times
 *
 * @return 0 if successful
 * @return -1 for errors
 */
int d_all_sendraw(const void* data, size_t data_length,
		const struct sockaddr_in* address, size_t address_length,
		size_t repeats);


/*
 * Recieve raw data using UDP protocol
 * Wrapper for <socket.h>::recvfrom()
 *
 * @param data
 * @param data_length: 'sizeof(data)'
 * @param address
 * @param address_length: 'sizeof(address)'
 *
 * @return sizeof(data) actually recieved
 * @return -1 for errors
 * @return -2 if no data can be recieved at the moment
 *
 * @note function call never freezes; returns -2 if no data available
 */
int d_all_recvraw(void* data, size_t data_length,
		const struct sockaddr_in* address, socklen_t* address_length);








// ========================================================================== //
// CLIENT NETWORK FUNCTIONS
// ========================================================================== //

/*
 * CLIENT: Send "hello" datagram to server
 *
 * @param server: Info of server to be connected to
 * @param reconnect: Is reconnect procedure required
 * 				= 1: Reconnect, use given IP adress
 * 				= 2: Reconnect using valid D_CLIENT_NET_DATA
 * 				Oth: Do not reconnect
 *
 * @return 0 if successful
 * @return -1 for errors
 *
 * @note Execute this before using any other _client functions
 */
int d_client_connect(HR_ADDRESS server, int reconnect);


/*
 * CLIENT: Send datagram to server
 *
 * @param data: Data to send
 * @param data_length
 * @param repeats: Send UDP packet this number of times
 *
 * @return 0 if successful
 * @return -1 for errors
 *
 * @warning data must fit UPACK_HEAD
 */
int d_client_send(const void* data, size_t data_length, size_t repeats);


/*
 * CLIENT: Recieve [time-signed] datagram
 *
 * @param data: Place for data to be put to
 * @param data_length
 * @param tick: Minimum required tick to get data
 * 				= 0, all datagrams are allowed
 * 				> 0, 'old' datagrams are ignored (which have tick < old)
 *
 * @return 0 if successful
 * @return -1 for errors
 */
int d_client_get(void* data, size_t data_length, TICK_TYPE tick);








// ========================================================================== //
// SERVER NETWORK FUNCTIONS
// ========================================================================== //

/*
 * SERVER: Send a datagram
 *
 * @param data: Data to send
 * @param data_length
 * @param destination: Adressee info
 * @param repeats: Send UDP packet this number of times
 *
 * @return 0 if successful
 * @return -1 for errors
 */
int d_server_send(const void* data, size_t data_length,
		HR_ADDRESS destination, size_t repeats);


/*
 * SERVER: Recieve a datagram
 *
 * @param mode:
 * 				= 0, accept and save all datagrams
 * 				= 1, ignore CLIENT_ACTION datagrams
 * 				.. to be continued ..
 *
 * @param data: Place for data to be put to
 * @param data_length
 * @param departure: Place for adresser info to be put to. May be NULL
 *
 * @return 0 if datagram recieved correctly
 * @return -1 for errors
 * @return -2 if no packets available at the moment
 *
 * @note it is recommended to have data as big as possible: UDP_MAX_PACKET_SIZE
 * and cut it afterwards; otherwise some packets may be partially lost
 *
 * @note all GAME datagrams will be lost, as a server must not get them normally
 */
int d_server_get(int mode, void* data, size_t data_length,
		HR_ADDRESS* departure);








#endif /* DCONNECT_INCLUDED */
