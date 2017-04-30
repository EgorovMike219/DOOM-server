#ifndef DCONNECT_INCLUDED
#define DCONNECT_INCLUDED

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define TICK_TYPE uint64_t
#define ID_TYPE int




/* Network customization */

#define NET_PORT 57575
#define NET_REPEAT 3

#define PACK_USUAL 0
#define PACK_SPECIAL 1
#define PACK_C 2

/* --------------------- */




/// Packet types to make pointers easily
typedef struct All_Net_packet_usual_t {
	int8_t type;
	TICK_TYPE tick;
	char data[1];
} PACK_USUAL_HEAD;

typedef struct All_Net_packet_special_t {
	int8_t type;
	char data[1];
} PACK_SPECIAL_HEAD;

typedef struct All_Net_packet_c_t {
	int8_t type;
	int8_t query_type;
	char data[1];
} PACK_C_HEAD;




/**
 * Creates an UDP socket and binds it.
 *
 * @param type: Type of connection required
 * = 0: server
 * = 1: client
 * = 2: statistics
 * @param connections: [SERVER-only] Maximum number of connections allowed
 *
 * @return 0 if successful
 * @return -1 for errors
 *
 * @note Makes D_NET_DATA valid if successful and prepares D_SERVER_NET_DATA
 */
int d_all_connect(int type, size_t connections);


/**
 * @param type: Type of connection to be closed
 * = 0: server
 * = 1: client
 * = 2: statistics
 */
void d_all_disconnect(int type);


/**
 * Send raw data using UDP protocol
 * Wrapper for <socket.h>::sendto()
 *
 * @param data
 * @param data_length: 'sizeof(data)'
 * @param address
 * @param address_length: 'sizeof(address)'
 * @param repeats: Send UDP packet this number of times
 * 	@def: NET_USUAL_REPEAT
 *
 * @return 0 if successful
 * @return -1 for errors
 */
int d_all_sendraw(const void* data, size_t data_length,
		const struct sockaddr_in* address, size_t address_length,
		size_t repeats);


/**
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
 */
int d_all_recvraw(void* data, size_t data_length,
		const struct sockaddr_in* address, socklen_t* address_length);






/**
 * CLIENT: Send "hello" datagram to server
 *
 * @param ip: Server ip (must be a valid C string with \0 ending)
 *
 * @return 0 if successful
 * @return -1 for errors
 *
 * @note Makes D_CLIENT_NET_DATA valid if call successful
 */
int d_client_connect(const char* ip);


/**
 * CLIENT: Send (any) datagram to server
 *
 * @param data: Data to send
 * @param data_length
 *
 * @return 0 if successful
 * @return -1 for errors
 *
 * @warning valid D_CLIENT_NET_DATA required
 * @warning data must begin with a 'size8_t' protocol code
 */
int d_client_send(const void* data, size_t data_length);


/**
 * CLIENT: Recieve [time-signed] datagram
 *
 * @param data: Place for data to be put to
 * @param data_length
 * @param tick: Minimum required tick to get data
 * 				If = 0, all datagrams are allowed (including usual)
 * 				If > 0, all unusual and 'old' datagrams are ignored
 *
 * @return 0 if successful
 * @return -1 for errors
 *
 * @warning valid D_CLIENT_NET_DATA required
 */
int d_client_get(void* data, size_t data_length, TICK_TYPE tick);






/**
 * SERVER: Send datagram
 *
 * @param id: Valid D_SERVER_NET_DATA id
 * @param data: Data to send
 * @param data_length
 *
 * @return 0 if successful
 * @return -1 for errors
 *
 * @warning valid D_SERVER_NET_DATA required
 */
int d_server_send(ID_TYPE id, const void* data, size_t data_length);


/**
 * SERVER: Recieve datagram from client
 *
 * @param data: Place for data to be put to
 * @param data_length
 * @param id_ptr: ID of client sending datagram
 * @param mode:
 * 				If = 0, do NOT connect new clients
 * 				If = 1, ONLY special datagrams are allowed
 * 				If = 2, do NOT accept usual datagrams (but connect new clients)
 *
 * @return 0 if a usual datagram is recieved or a new client is added
 * @return 1 if a special datagram is recieved
 * @return -1 for errors
 */
int d_server_get(void* data, size_t data_length, ID_TYPE* id_ptr, int mode);


/**
 * SERVER: Manage preferences for selected client id
 *
 * @param id: Client id
 * @param op: Operation to be performed
 * 		0: Test existence & exit
 * 		-1: Remove client
 *
 * @return 0 if successful
 * @return 1 if no such client exists
 * @return 2 if successful and now is empty
 * @return -1 for errors
 *
 * @note a record with last existing id will be put in given 'id' position
 */
int d_server_manageid(ID_TYPE id, int op);





#endif /* DCONNECT_INCLUDED */
