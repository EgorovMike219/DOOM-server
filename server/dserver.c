#define _GNU_SOURCE  // Required for pthread_yield()

#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

#include "../all/dconnect.h"

#include "dgame.h"




/// Maximum length of user interaction message
#define INTERACTION_MESSAGE_LENGTH 1024

/// Normal message type
#define MES_INT_NORMAL 1

/// "Stop" message type. Send once to stop 'interactor' thread
#define MES_INT_STOP 592

/// IPC message buffer
struct msgbuf_server {
	long type;
	char text[INTERACTION_MESSAGE_LENGTH];
};

/// Message queue ID (private ownership)
int msquid_server_in;

/// Message queue ID (private ownership)
int msquid_server_out;

/// Interactor pthread identifier
pthread_t interactor_id;




/// Ingoing packages storage
void* packs_in;

pthread_mutex_t packs_in_mutex;

pthread_t packs_in_id;




/// Outgoing packages storage
void* packs_out;

/// Outgoing packages status flag
int packs_out_flag;

pthread_mutex_t packs_out_mutex;

pthread_cond_t packs_out_cond;

pthread_t packs_out_id;




/// Database of HR_ADDRESSes of all connected clients
HR_ADDRESS* hr_database;

/// Total number of connected clients (aka players)
int players_total;




/**
 * @brief Thread function. Interacts with server user (via stdin, stdout)
 *
 * @note Controlled by 'msquid_server_out'
 */
void* interactor(void* dummy) {
	struct msgbuf_server message_get;
	struct msgbuf_server message_send;
	char command[INTERACTION_MESSAGE_LENGTH];
	
	message_send.type = MES_INT_NORMAL;
	
	while (1) {
		// Receive and print all messages from "core"
		printf("\n");
		while (msgrcv(msquid_server_out,
					  &message_get, INTERACTION_MESSAGE_LENGTH, 0,
					  MSG_NOERROR | IPC_NOWAIT) > 0) {
			if (message_get.type == MES_INT_STOP) {
				pthread_exit(NULL);
			}
			printf("%s\n", message_get.text);
		}
		errno = 0;
		
		// Read command
		printf("\n > ");
		if (scanf("%s", command) == EOF) {
			continue;
		}
		
		// Process commands
		if (strcmp(command, "launch") == 0) {
			strcpy(message_send.text, command);
			msgsnd(msquid_server_in,
				   &message_send, INTERACTION_MESSAGE_LENGTH, 0);
		}
		else if (strcmp(command, "exit") == 0) {
			strcpy(message_send.text, command);
			msgsnd(msquid_server_in,
				   &message_send, INTERACTION_MESSAGE_LENGTH, 0);
			// Todo: Exit processing
		}
		else {
			continue;
		}
	}
}




/**
 * @brief Thread function. Receives 'packs_in'
 *
 * @note Controlled by DP_SERVICE packets from server itself
 */
void* packs_in_processor(void* dummy) {
	UPACK_HEAD* received = malloc(UPACK_SIZE(INFO_LENGTH));
	HR_ADDRESS received_hr;
	int exec_result;
	
	HR_ADDRESS server_hr;
	strcpy(server_hr.ip, "127.0.0.1");
	server_hr.port = NET_PORT;
	
	UPACK_HEAD* holder;
	
	char log_message[LOG_MESSAGE_LENGTH];  // Temporary log message
	struct msgbuf_server message_out;  // Message for interactor
	message_out.type = MES_INT_NORMAL;
	
	while (1) {
		// Receive a packet and process errors
		exec_result = d_server_get(0,
								   received, UPACK_SIZE(INFO_LENGTH),
								   &received_hr);
		if (exec_result == -2) {
			continue;
		}
		if (exec_result == -1) {
			d_log("ERR: Network receive error");
			continue;
		}
		
		// Process service commands (from server itself)
		if ((received ->type == DP_SERVICE) &&
				(d_compare_hr(&received_hr, &server_hr) == 0)) {
			if (strcmp(received ->data, "exit") == 0) {
				break;
			}
		}
		
		// Check client ID
		if (((int)received ->meta > players_total) ||
				((int) received ->meta < 0)) {
			d_log("ERR: Network request from unknown client");
			continue;
		}
		if (d_compare_hr(&received_hr, &(hr_database[received ->meta])) != 0) {
			d_log("ERR: Network request with invalid client ID");
			continue;
		}
		holder = (UPACK_HEAD*)packs_in +
				 (received ->meta) * UPACK_SIZE(INFO_LENGTH);
		
		// Process packet of each type
		switch (received ->type) {
			case DP_CLIENT_ACTION:
			case DP_CLIENT_STOP:
				pthread_mutex_lock(&packs_in_mutex);
				*holder = *received;
				pthread_mutex_unlock(&packs_in_mutex);
				break;
			case DP_RECONNECT:
				if (received ->stamp == DP_S_ASK) {
					received ->stamp = DP_S_SUCCESS;
					d_server_send(received, UPACK_SIZE(1),
								  received_hr, NET_REPEAT_SERVER);
					sprintf(log_message,
							"ACT: Reconnected client %s:%d; id %d",
							received_hr.ip, received_hr.port, received ->meta);
					d_log(log_message);
					strcpy(message_out.text, log_message);
					msgsnd(msquid_server_out,
						   &message_out, INTERACTION_MESSAGE_LENGTH, IPC_NOWAIT);
					errno = 0;
				}
				else {
					continue;
				}
				break;
			default:
				continue;
		}
	}
	
	free(received);
	
	return NULL;
}




/**
 * @brief Thread function. Sends 'packs_out'
 *
 * @note Controlled by 'packs_out_flag'
 */
void* packs_out_processor(void* dummy) {
	int i;
	
	UPACK_HEAD* current;
	
	const size_t SEND_SIZE = (size_t)UPACK_SIZE(
			level_width * level_height + INFO_LENGTH);
	
	while (1) {
		// Wait for packs_out to be prepared
		pthread_mutex_lock(&packs_out_mutex);
		while (packs_out_flag == 0) {
			pthread_cond_wait(&packs_out_cond, &packs_out_mutex);
		}
		
		// Process exit
		if (packs_out_flag < 0) {
			pthread_mutex_unlock(&packs_out_mutex);
			pthread_cond_broadcast(&packs_out_cond);
			break;
		}
		
		// Send all packets
		current = (UPACK_HEAD*)packs_out;
		for (i = 0; i < players_total; i++) {
			d_server_send(current, SEND_SIZE,
						  hr_database[i], NET_REPEAT_SERVER);
			current = current + SEND_SIZE;
		}
		
		packs_out_flag = 0;
		pthread_mutex_unlock(&packs_out_mutex);
		pthread_cond_broadcast(&packs_out_cond);
		
		pthread_yield();
	}
	
	return NULL;
}




/**
 * @brief Print help
 */
void print_help(void) {
	printf("Usage: server [KEY VALUE]\n");
	printf("Launch DOOM-592 game server\n");
	
	printf("\n\n");
	
	printf("  -m, --map <map file> \t\t Use given map file.\n");
	printf("  If not given, \"./map\" is used instead.\n");
	printf("\n");
	printf("  -n, --max-players <number> \t\t Limit number of players by a given number.\n");
	printf("  If not given, 16 is used instead.\n");
	printf("\n");
	printf("  -l, --log <log file> \t\t Use given log file and REWRITE it.\n");
	printf("  If not given, \"./game.log\" is used instead.\n");
	printf("\n");
	printf("  -lc, --log-continue <log file> \t\t Use given log file and CONTINUE it.\n");
	printf("  If not given, \"./game.log\" is used instead.\n");
	printf("\n");
	printf("  -?, --help, --usage \t\t Display this message.\n");
	
	printf("\n\n");
	
	printf("Notes:\n");
	printf("* \"stderr\" may be used as log file (not recommended)\n");
	printf("* If player number limit is much more than ~100, the game may significantly slow down\n");
	
	printf("\n");
}




/**
 * @brief Safely close IO thread and net threads
 */
void server_shutdown(void) {
	d_log("ACT: Server shutdown called");
	
	HR_ADDRESS server_self;
	strcpy(server_self.ip, "127.0.0.1");
	server_self.port = NET_PORT;
	
	UPACK_HEAD* current_pack = malloc(UPACK_SIZE(INFO_LENGTH));
	
	struct msgbuf_server message;
	
	
	pthread_mutex_lock(&packs_out_mutex);
	packs_out_flag = -1;
	pthread_mutex_unlock(&packs_out_mutex);
	pthread_cond_broadcast(&packs_out_cond);
	
	current_pack ->type = DP_SERVICE;
	strcpy(current_pack ->data, "exit");
	d_server_send(current_pack, UPACK_SIZE(INFO_LENGTH),
				  server_self, NET_REPEAT_ONE);
	
	message.type = MES_INT_STOP;
	msgsnd(msquid_server_out, &message, INTERACTION_MESSAGE_LENGTH, 0);
	
	pthread_join(packs_out_id, NULL);
	pthread_join(packs_in_id, NULL);
	pthread_join(interactor_id, NULL);
}




int main(int argc, char** argv) {
	char log_message[LOG_MESSAGE_LENGTH];  // Temporary log message
	
	struct msgbuf_server message_in;  // Message from interactor
	struct msgbuf_server message_out;  // Message for interactor
	message_out.type = MES_INT_NORMAL;
	
	HR_ADDRESS current_hr;  // HR holder
	UPACK_HEAD* current_pack = make_UPACK(UDP_MAX_PACKET_SIZE); // Packet holder
	
	
	// Process command line arguments and set parameters //
	
	{
		char* map_file = malloc(strlen("./map") + 1);
		strcpy(map_file, "./map");
		
		char* log_file = NULL;
		char log_mode = 'n';
		
		int maximum_players = 16;
		
		int i;
		for (i = 1; i < argc; i++) {
			// Map file
			if ((strcmp(argv[i], "-m") == 0) ||
					(strcmp(argv[i], "--map") == 0)) {
				if (i + 1 == argc) {
					continue;
				}
				i += 1;
				realloc(map_file, strlen(argv[i]) + 1);
				strcpy(map_file, argv[i]);
			}
			// Maximum number of players
			else if ((strcmp(argv[i], "-n") == 0) ||
					(strcmp(argv[i], "--max-players") == 0)) {
				if (i + 1 == argc) {
					continue;
				}
				i += 1;
				maximum_players = (int)strtol(argv[i], NULL, 10);
			}
			// Log file (rewrite)
			else if ((strcmp(argv[i], "-l") == 0) ||
					(strcmp(argv[i], "--log") == 0)) {
				if (i + 1 == argc) {
					continue;
				}
				i += 1;
				if (strcmp(argv[i], "stderr") == 0) {
					log_mode = 't';
					continue;
				}
				log_mode = 'n';
				log_file = malloc(strlen(argv[i]) + 1);
				strcpy(log_file, argv[i]);
			}
			// Log file (continue)
			else if ((strcmp(argv[i], "-lc") == 0) ||
					(strcmp(argv[i], "--log-continue") == 0)) {
				if (i + 1 == argc) {
					continue;
				}
				i += 1;
				if (strcmp(argv[i], "stderr") == 0) {
					log_mode = 't';
					continue;
				}
				log_mode = 'a';
				log_file = malloc(strlen(argv[i]) + 1);
				strcpy(log_file, argv[i]);
			}
			// Help
			else if ((strcmp(argv[i], "-?") == 0) ||
					(strcmp(argv[i], "--help") == 0) ||
					(strcmp(argv[i], "--usage") == 0)) {
				print_help();
				return 0;
			}
			else {
				fprintf(stderr, "Unknown command line options.\n");
				return -1;
			}
		}
		
		// Init log
		if (d_log_initialize(log_file, log_mode) < 0) {
			fprintf(stderr, "CRI: Unable to open log file\n");
		}
		
		// Load level
		if (d_level_load(map_file) < 0) {
			d_log_close();
			return -1;
		}
		
		// Allocate hr_database
		players_total = maximum_players;
		if ((hr_database = malloc(sizeof(HR_ADDRESS) * players_total))
			== NULL) {
			d_log("CRI: Client database allocation unsuccessful");
			d_log_close();
			return -1;
		}
		
		// Connect to network
		if (d_all_connect(0) < 0) {
			d_log("CRI: Cannot create connection");
			d_log_close();
			return -1;
		}
		
		// Launch interactor
		if ((msquid_server_in = msgget(IPC_PRIVATE, 0)) < 0) {
			d_log("CRI: Cannot create IO interaction message queue (in)");
			d_log_close();
			return -1;
		}
		if ((msquid_server_out = msgget(IPC_PRIVATE, 0)) < 0) {
			d_log("CRI: Cannot create IO interaction message queue (out)");
			d_log_close();
			return -1;
		}
		if (pthread_create(&interactor_id, NULL, interactor, NULL) != 0) {
			d_log("CRI: Cannot launch IO handler");
			d_log_close();
			return -1;
		}
	}
	
	// Connect clients //
	
	{
		int get_res;  // Packet send result
		
		units_total = 0;
		
		while (1) {
			// Get command from interactor
			if (msgrcv(msquid_server_in,
					   &message_in, INTERACTION_MESSAGE_LENGTH, 0,
					   MSG_NOERROR | IPC_NOWAIT) < 0) {
				// No commands
				errno = 0;
			}
			else {
				if (strcmp(message_in.text, "launch") == 0) {
					break;
				}
				
				continue;
			}
			
			// Receive packet
			get_res = d_server_get(0, current_pack,
								   UPACK_SIZE(UDP_MAX_PACKET_SIZE),
								   &current_hr);
			
			if (get_res == -1) {  // Error
				d_log("ERR: Network receive error");
				continue;
			}
			if (get_res == -2) {  // No packets to receive
				continue;
			}
			// Some packet received
			
			// Connect request
			if ((current_pack ->type == DP_CONNECT) &&
					(current_pack ->stamp == DP_S_ASK)) {
				// Check number of clients already connected
				if (units_total == players_total) {
					current_pack ->stamp = DP_S_ERROR;
					d_server_send(current_pack, UPACK_SIZE(1),
								  current_hr, NET_REPEAT_SERVER);
					continue;
				}
				
				// Check whether this client already exists
				int i;
				int is_found = 0;
				for (i = 0; i < units_total; i++) {
					if (d_compare_hr(&(hr_database[i]), &current_hr) == 0) {
						is_found = 1;
						break;
					}
				}
				if (is_found == 1) {
					current_pack ->stamp = DP_S_ERROR;
					d_server_send(current_pack, UPACK_SIZE(1),
								  current_hr, NET_REPEAT_SERVER);
					continue;
				}
				
				// Add client and respond success
				hr_database[i] = current_hr;
				units_total += 1;
				current_pack ->stamp = DP_S_SUCCESS;
				d_server_send(current_pack, UPACK_SIZE(1),
							  current_hr, NET_REPEAT_SERVER);
				
				// Log
				sprintf(log_message,
						"ACT: Connected client %s:%d; id %d",
						current_hr.ip, current_hr.port, i);
				d_log(log_message);
				strcpy(message_out.text, log_message);
				msgsnd(msquid_server_out,
					   &message_out, INTERACTION_MESSAGE_LENGTH, IPC_NOWAIT);
				errno = 0;
				continue;
			}
			
			// Reconnect request
			if ((current_pack ->type == DP_RECONNECT) &&
				(current_pack ->stamp == DP_S_ASK)) {
				// Check whether this client already exists
				int i;
				int is_found = 0;
				for (i = 0; i < units_total; i++) {
					if (d_compare_hr(&(hr_database[i]), &current_hr) == 0) {
						is_found = 1;
						break;
					}
				}
				if (is_found == 0) {
					current_pack ->stamp = DP_S_ERROR;
					d_server_send(current_pack, UPACK_SIZE(1),
								  current_hr, NET_REPEAT_SERVER);
					continue;
				}
				
				// Client found, respond success
				current_pack ->stamp = DP_S_SUCCESS;
				d_server_send(current_pack, UPACK_SIZE(1),
							  current_hr, NET_REPEAT_SERVER);
				
				// Log
				sprintf(log_message,
						"ACT: Reconnected client %s:%d; id %d",
						current_hr.ip, current_hr.port, i);
				d_log(log_message);
				strcpy(message_out.text, log_message);
				msgsnd(msquid_server_out,
					   &message_out, INTERACTION_MESSAGE_LENGTH, IPC_NOWAIT);
				errno = 0;
			}
		}
	}
	
	// Prepare for the game //
	
	d_log("ACT: Game prepare procedure started");
	{
		// Optimize hr_database
		if (units_total < players_total) {
			realloc(hr_database, sizeof(HR_ADDRESS) * units_total);
			players_total = units_total;
		}
		
		// Extra actions should be placed here for NPC addition //
		
		// Allocate buffers
		if ((units = malloc(sizeof(UNIT) * units_total)) == NULL) {
			d_log("CRI: Units database allocation unsuccessful");
			d_log_close();
			return -1;
		}
		if ((units_cmd = malloc((size_t)units_total)) == NULL) {
			d_log("CRI: Units' commands database allocation unsuccessful");
			d_log_close();
			return -1;
		}
		if ((packs_in = malloc((size_t)UPACK_SIZE(INFO_LENGTH) * players_total))
			== NULL) {
			d_log("CRI: Network buffer (in) allocation unsuccessful");
			d_log_close();
			return -1;
		}
		if ((packs_out = malloc((size_t)UPACK_SIZE(
				level_width * level_height + INFO_LENGTH) * players_total))
			== NULL) {
			d_log("CRI: Network buffer (out) allocation unsuccessful");
			d_log_close();
			return -1;
		}
		
		// Send DP_GAME_PREPARE message
		current_pack ->type = DP_GAME_PREPARE;
		current_pack ->stamp = 0;
		int i;
		for (i = 0; i < 10; i++) {  // This packet contains client's ID
			int k;
			for (k = 0; k < players_total; k++) {
				current_pack ->meta = k;
				d_server_send(current_pack, UPACK_SIZE(1),
							  hr_database[k], NET_REPEAT_SERVER);
			}
		}
		
		// Prepare receiver structures before launch
		UPACK_HEAD* pack_ptr;
		for (i = 0; i < players_total; i += UPACK_SIZE(INFO_LENGTH)) {
			pack_ptr = (UPACK_HEAD*)(packs_in + i);
			pack_ptr ->type = DP_CLIENT_ACTION;
			pack_ptr ->stamp = 0;
			pack_ptr ->meta = i / UPACK_SIZE(INFO_LENGTH);
			pack_ptr ->data[0] = CMD_NONE;
		}
		
		// Launch receiver. From now on packets can't be received by main()
		pthread_mutex_init(&packs_in_mutex, NULL);
		if (pthread_create(&packs_in_id, NULL, packs_in_processor, NULL) != 0) {
			d_log("CRI: Receiver launch failed");
			server_shutdown();
			d_log_close();
			return -1;
		}
		
		// Launch sender
		pthread_mutex_init(&packs_out_mutex, NULL);
		pthread_cond_init(&packs_out_cond, NULL);
		packs_out_flag = 0;
		if (pthread_create(&packs_out_id, NULL, packs_out_processor, NULL)
			!= 0) {
			d_log("CRI: Sender launch failed");
			server_shutdown();
			d_log_close();
			return -1;
		}
	}
	d_log("ACT: Game prepare procedure (stage 1) completed");
	
	// Launch the game //
	
	// Todo: Modify this part to enable all unit-related capabilities
	{
		int i;
		
		// Prepare players
		for (i = 0; i < players_total; i++) {
			units[i].id = i;
			units[i].type = ENTITY_PLAYER;
			units[i].health = level_passive_turns *
					level_passive_health_reduction;
			
			units[i].move_delay = UNIT_DEFAULT_MOVE_DELAY;
			units[i].next_action_tick = 1;
			
			units[i].weapon = bomb_default;
		}
		
		// Extra actions should be placed here for NPC addition //
		
		// Place units (one at a cell)
		for (i = 0; i < units_total; i++) {
			if (d_unit_get_position(&units[i].x, &units[i].y) < 0) {
				d_log("CRI: Level map is too small");
				server_shutdown();
				d_log_close();
				return -1;
			}
			level[d_level_pos(units[i].x, units[i].y)].units[0] = &units[i];
		}
		
		// Check and clear server commands
		while (msgrcv(msquid_server_in,
				   &message_in, INTERACTION_MESSAGE_LENGTH, 0,
				   MSG_NOERROR | IPC_NOWAIT) >= 0) {
			if (strcmp(message_in.text, "exit") == 0) {
				d_log("ACT: \"exit\" command received");
				server_shutdown();
				d_log_close();
				return -1;
			}
		}
		errno = 0;
		
		d_log("ACT: Game prepare procedure (stage 2) completed");
		d_all_delay(1.000);
		
		// Send begin message
		current_pack ->type = DP_GAME_BEGIN;
		current_pack ->stamp = 0;
		for (i = 0; i < players_total; i++) {
			current_pack ->meta = i;
			d_server_send(current_pack, UPACK_SIZE(1),
						  hr_database[i], NET_REPEAT_SERVER);
		}
		d_all_delay(0.500);
	}
	
	// Game process //
	
	d_log("ACT: Game starts");
	while (1) {
		int is_game_over = 0;
		
		int i;
		UPACK_HEAD* current;
		const size_t SEND_SIZE = (size_t)UPACK_SIZE(
				level_width * level_height + INFO_LENGTH);
		
		int refresh_result;
		
		int x;
		int y;
		int j;
		
		char command[INFO_LENGTH];  // Todo: Change length constant
		
		// Process console commands
		while (msgrcv(msquid_server_in,
					  &message_in, INTERACTION_MESSAGE_LENGTH, 0,
					  MSG_NOERROR | IPC_NOWAIT) >= 0) {
			if (strcmp(message_in.text, "exit") == 0) {
				d_log("ACT: \"exit\" command received");
				is_game_over = 1;
				break;
			}
			// Todo: Other options
		}
		errno = 0;
		if (is_game_over != 0) {
			break;
		}
		
		// Copy client commands
		current = (UPACK_HEAD*)packs_in;
		pthread_mutex_lock(&packs_in_mutex);
		for (i = 0; i < players_total; i++) {
			if (current ->type == DP_NONE) {
				continue;
			}
			units_cmd[i] = current ->data[0];
			current ->type = DP_NONE;
			current = current + UPACK_SIZE(INFO_LENGTH);
		}
		pthread_mutex_unlock(&packs_in_mutex);
		
		// Extra actions should be placed here for NPC addition //
		
		
		// Update game situation
		if (d_game_update() < 0) {
			sprintf(log_message,
					"ERR: Game update failed at tick %llu",
					(unsigned long long)tick);
			d_log(log_message);
			strcpy(message_out.text, log_message);
			msgsnd(msquid_server_out,
				   &message_out, INTERACTION_MESSAGE_LENGTH, IPC_NOWAIT);
			errno = 0;
		}
		
		// Refresh game and change tick
		refresh_result = d_game_refresh();
		if (refresh_result < -1) {
			sprintf(log_message,
					"ERR: Game check failed at tick %llu",
					(unsigned long long)tick);
			d_log(log_message);
			strcpy(message_out.text, log_message);
			msgsnd(msquid_server_out,
				   &message_out, INTERACTION_MESSAGE_LENGTH, IPC_NOWAIT);
			errno = 0;
		}
		
		// Send response to players
		current = (UPACK_HEAD*)packs_out;
		pthread_mutex_lock(&packs_out_mutex);
		for (i = 0; i < players_total; i++) {
			current ->stamp = tick;
			
			j = 0;
			for (x = units[i].x - (CLIENT_FIELD_WIDTH / 2);
				 x < units[i].x + (CLIENT_FIELD_WIDTH / 2);
				 x++) {
				for (y = units[i].y - (CLIENT_FIELD_HEIGHT / 2);
					 y < units[i].y + (CLIENT_FIELD_HEIGHT / 2); y++) {
					if ((x > level_width) || (x < 0) ||
							(y > level_height) || (y < 0)) {
						current ->data[j] = ENTITY_WALL;
					}
					else {
						current ->data[j] =
								level[d_level_pos(x, y)].representation;
						if ((level[d_level_pos(x, y)].weapon.type
							 == ENTITY_HEART) ||
							(level[d_level_pos(x, y)].weapon.type
							 == ENTITY_POISON)) {
							current ->data[j] = ENTITY_HEART;
						}
					}
					j += 1;
				}
			}
			
			if ((units[i].health <= 0) || (refresh_result < 0)) {
				current ->type = DP_GAME_OVER;
				strcpy(command, "lose");
			}
			else if (refresh_result > 0) {
				current ->type = DP_GAME_OVER;
				strcpy(command, "win");
			}
			else {
				current ->type = DP_GAME;
				strcpy(command, "0");
			}
			
			sprintf(current ->data + CLIENT_FIELD_AREA,
					"%04d %02d %s %04d %s",
					units[i].health,
					(int)(((float)units[i].health / (float)(
							level_passive_health_reduction *
							level_passive_turns)) * 100.0 - 5.0),
					units[i].weapon.name,
					units[i].weapon.charge,
					command);
			
			current = current + SEND_SIZE;
		}
		
		packs_out_flag = 1;
		pthread_mutex_unlock(&packs_out_mutex);
		pthread_cond_broadcast(&packs_out_cond);
		
		sprintf(log_message, "ACT: Turn %llu completed",
				(long long unsigned)tick);
		d_log(log_message);
		
		if (refresh_result != 0) {
			break;
		}
	}
	
	server_shutdown();
	d_log("ACT: Server shutdown successful");
	d_log_close();
	
	return 0;
}



