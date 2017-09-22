#define _GNU_SOURCE  // Non-standart function pthread_yield() used

#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <math.h>
#include <semaphore.h>

#include "../all/dconnect.h"

#include "dgame.h"

#include "dserver_settings.h"




/// Normal message type
#define MES_INT_NORMAL 1

/// "Stop" message type. Send once to stop 'interactor' thread
#define MES_INT_STOP 592

/// IPC message buffer
struct msgbuf_server {
	long type;
	char text[LENGTH_LOG_MESSAGE];
};

/// Message queue ID (private ownership)
int msquid_server_in;

/// Message queue ID (private ownership)
int msquid_server_out;

/// Interactor pthread identifier
pthread_t interactor_id = 0;




/// Ingoing packages storage
void* packs_in = NULL;

pthread_mutex_t packs_in_mutex;

pthread_t packs_in_id = 0;




/// Outgoing packages storage
void* packs_out = NULL;

/// Outgoing packages status flag
int packs_out_flag;

pthread_mutex_t packs_out_mutex;

pthread_cond_t packs_out_cond;

pthread_t packs_out_id = 0;




/// Database of HR_ADDRESSes of all connected clients
HR_ADDRESS* hr_database = NULL;

/// Total number of connected clients (aka players)
int players_total;




/**
 * @brief Thread function. Interacts with server user (via stdin, stdout)
 *
 * @note Controlled by 'msquid_server_out'
 */
void* interactor(void* dummy) {
	// Dummy-check
	if (dummy != NULL) {
		return NULL;
	}
	
	struct msgbuf_server message_get;
	struct msgbuf_server message_send;
	char command[LENGTH_LOG_MESSAGE];
	int i;
	
	message_send.type = MES_INT_NORMAL;
	
	while (1) {
		// Delay for the server to respond
		d_all_delay(SERVER_DELAY_IO);
		
		// Receive and print all messages from "core"
		while (msgrcv(msquid_server_out,
					  &message_get, LENGTH_LOG_MESSAGE, 0,
					  MSG_NOERROR | IPC_NOWAIT) > 0) {
			if (message_get.type == MES_INT_STOP) {
				pthread_exit(NULL);
			}
			printf("%s\n", message_get.text);
		}
		errno = 0;
		
		// Read command
		printf(" > ");
		fgets(command, LENGTH_LOG_MESSAGE, stdin);
		if (isspace((int)command[0]) != 0) {
			continue;
		}
		for (i = (int)strlen(command) - 1; i >= 0; i--) {
			if (isspace((int)command[i]) == 0) {
				break;
			}
			command[i] = '\0';
		}
		
		// Process commands
		if ((strcmp(command, "help") == 0) ||
				(strcmp(command, "?") == 0)) {
			printf("Messages from server (if any) are printed after any input.\n");
			printf("\nCommands available at any time:\n");
			printf(" *\x1B[1m status\x1B[0m\t Get actual information about the server\n");
			printf(" *\x1B[1m exit  \x1B[0m\t Shut server down immediately\n");
			printf("\nCommands available when waiting for players:\n");
			printf(" *\x1B[1m msg <n> <message>\x1B[0m\t Send a message with a given ID to all connected players. ID (<n>) must be a 1-symbol 16-base number\n");
			printf(" *\x1B[1m launch\x1B[0m\t Launch the game\n");
			printf("\nCommands available during the game:\n");
			printf(" *\x1B[1m kill <id>\x1B[0m\t Kill the unit with a given ID\n");
			printf("\n");
		}
		else {
			strcpy(message_send.text, command);
			msgsnd(msquid_server_in,
				   &message_send, LENGTH_LOG_MESSAGE, 0);
		}
		
		if (strcmp(command, "exit") == 0) {
			break;
		}
		
		pthread_yield();
	}
	
	return NULL;
}




/**
 * @brief Thread function. Receives 'packs_in'
 *
 * @note Controlled by DP_SERVICE packets from server itself
 */
void* packs_in_processor(void* dummy) {
	// Dummy-check
	if (dummy != NULL) {
		return NULL;
	}
	
	UPACK_HEAD* received = (UPACK_HEAD*) malloc(SZ_CLIENT_ACTION);
	HR_ADDRESS received_hr;
	int exec_result;
	
	HR_ADDRESS server_hr;
	strcpy(server_hr.ip, "127.0.0.1");
	server_hr.port = NET_PORT;
	
	UPACK_HEAD* holder;
	
	char log_message[LENGTH_LOG_MESSAGE];  // Temporary log message
	struct msgbuf_server message_out;  // Message for interactor
	message_out.type = MES_INT_NORMAL;
	
	while (1) {
		// Receive a packet and process errors
		exec_result = d_server_get(0,
								   received, SZ_CLIENT_ACTION,
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
		holder = (UPACK_HEAD*)(packs_in + (received ->meta) * SZ_CLIENT_ACTION);
		
		// Process packet of each type
		switch (received ->type) {
			case DP_CLIENT_ACTION:
				pthread_mutex_lock(&packs_in_mutex);
				*holder = *received;
				pthread_mutex_unlock(&packs_in_mutex);
				break;
			case DP_RECONNECT:  // Todo: Reconnect (resolve port conflict)
				if (received ->stamp == DP_S_ASK) {
					received ->stamp = DP_S_SUCCESS;
					d_server_send(received, UPACK_SIZE(1),
								  received_hr, NET_REPEAT_SERVER);
					sprintf(log_message,
							"ACT: Reconnected client #%d (%s:%d)",
							received ->meta, received_hr.ip, received_hr.port);
					d_log(log_message);
					strcpy(message_out.text, log_message);
					msgsnd(msquid_server_out,
						   &message_out, LENGTH_LOG_MESSAGE, IPC_NOWAIT);
					errno = 0;
				}
				else {
					continue;
				}
				break;
			case DP_CONNECT:
				received ->stamp = DP_S_ERROR;
				d_server_send(received, UPACK_SIZE(1),
							  received_hr, NET_REPEAT_SERVER);
				sprintf(log_message,
						"ACT: Connect request from %s:%d (refused)",
						received_hr.ip, received_hr.port);
				d_log(log_message);
				strcpy(message_out.text, log_message);
				msgsnd(msquid_server_out,
					   &message_out, LENGTH_LOG_MESSAGE, IPC_NOWAIT);
				errno = 0;
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
	// Dummy-check
	if (dummy != NULL) {
		return NULL;
	}
	
	int i;
	
	UPACK_HEAD* current;
	
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
			d_server_send(current, SZ_GAME,
						  hr_database[i], NET_REPEAT_SERVER);
			current = (void*)current + SZ_GAME;
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
	printf("\x1B[4mUsage\x1B[0m: server [KEY] [VALUE]\n");
	printf("DOOM-592 game server\n");
	
	printf("\n\n");
	
	printf("\t\x1B[1m-m, --map <map file>\x1B[0m\n");
	printf("\tUse given map file.\n");
	printf("\tIf not given, \"./map\" is used instead.\n");
	printf("\n");
	
	printf("\t\x1B[1m-n, --max-players <number>\x1B[0m\n");
	printf("\tLimit number of players by a given number.\n");
	printf("\tIf not given, 16 is used instead.\n");
	printf("\n");
	
	printf("\t\x1B[1m-l, --log <log file>\x1B[0m\n");
	printf("\tUse given log file and REWRITE it.\n");
	printf("\tIf not given, \"./game.log\" is used instead.\n");
	printf("\n");
	
	printf("\t\x1B[1m-lc, --log-continue <log file>\x1B[0m\n");
	printf("\tUse given log file and CONTINUE it.\n");
	printf("\tIf not given, \"./game.log\" is used instead.\n");
	printf("\n");
	
	printf("\t\x1B[1m-?, --help, --usage\x1B[0m\n");
	printf("\tDisplay this message.\n");
	
	printf("\n\n");
	
	printf("\x1B[4mNotes\x1B[0m:\n");
	printf("\t\"stderr\" may be used as log file (not recommended)\n");
	
	printf("\n");
}




/**
 * @brief Safely close IO thread and net threads
 * @note Safe to call at any moment
 */
void server_shutdown(void) {
	d_log("ACT: Server shutdown");
	
	if (interactor_id != 0) {
		struct msgbuf_server message;
		message.type = MES_INT_STOP;
		msgsnd(msquid_server_out, &message, LENGTH_LOG_MESSAGE, 0);
		
		pthread_join(interactor_id, NULL);
	}
	
	if (packs_out_id != 0) {
		pthread_mutex_lock(&packs_out_mutex);
		packs_out_flag = -1;
		pthread_mutex_unlock(&packs_out_mutex);
		pthread_cond_broadcast(&packs_out_cond);
		
		pthread_join(packs_out_id, NULL);
		
		free(packs_out);
	}
	
	if (packs_in_id != 0) {
		HR_ADDRESS server_self;
		strcpy(server_self.ip, "127.0.0.1");
		server_self.port = NET_PORT;
		
		UPACK_HEAD* current_pack = make_UPACK(LENGTH_NET_COMMAND);
		current_pack ->type = DP_SERVICE;
		strcpy(current_pack ->data, "exit");
		
		d_server_send(current_pack, UPACK_SIZE(LENGTH_NET_COMMAND),
					  server_self, NET_REPEAT_ONE);
		
		pthread_join(packs_in_id, NULL);
		
		free(packs_in);
	}
	
	free(hr_database);
	d_game_shutdown();
	
	d_log("ACT: Shutdown completed");
	d_log_close();
}




int main(int argc, char** argv) {
	char log_message[LENGTH_LOG_MESSAGE];  // Temporary log message
	
	struct msgbuf_server message_in;  // Message from interactor
	struct msgbuf_server message_out;  // Message for interactor
	message_out.type = MES_INT_NORMAL;
	
	HR_ADDRESS current_hr;  // HR holder
	UPACK_HEAD* current_pack = make_UPACK(UDP_MAX_PACKET_SIZE); // Packet holder
	
	char client_message[LENGTH_NET_COMMAND];
	
	struct timeval time_now;  // Current time
	struct timeval time_last_update;  // Time at last time check
	double interval_time;  // Time (in seconds) since last time check
	
	
	// Process command line arguments and set parameters //
	
	{
		char* map_file = malloc(strlen("./map") + 1);
		strcpy(map_file, "./map");
		
		char* log_file = malloc(strlen("./game.log") + 1);
		strcpy(log_file, "./game.log");
		
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
				map_file = realloc(map_file, strlen(argv[i]) + 1);
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
				free(map_file);
				free(log_file);
				free(current_pack);
				return 0;
			}
			else {
				fprintf(stderr, "Unknown command line options.\n");
				free(map_file);
				free(log_file);
				free(current_pack);
				return -1;
			}
		}
		
		// Init log
		if (d_log_initialize(log_file, log_mode) < 0) {
			fprintf(stderr, "CRI: Unable to open log file\n");
			server_shutdown();
			free(map_file);
			free(log_file);
			free(current_pack);
			return -1;
		}
		
		// Load level
		if (d_level_load(map_file) < 0) {
			server_shutdown();
			free(map_file);
			free(log_file);
			free(current_pack);
			return -1;
		}
		
		// Allocate hr_database
		players_total = maximum_players;
		if ((hr_database = malloc(sizeof(HR_ADDRESS) * players_total))
			== NULL) {
			d_log("CRI: Client database allocation unsuccessful");
			server_shutdown();
			free(current_pack);
			return -1;
		}
		
		// Allocate units
		if ((units = malloc(sizeof(UNIT) * players_total)) == NULL) {
			d_log("CRI: Units database allocation unsuccessful");
			server_shutdown();
			free(current_pack);
			return -1;
		}
		if ((units_cmd = malloc((size_t)players_total)) == NULL) {
			d_log("CRI: Units' commands database allocation unsuccessful");
			server_shutdown();
			free(current_pack);
			return -1;
		}
		
		// Connect to network
		if (d_all_connect(0) < 0) {
			d_log("CRI: Cannot create connection");
			server_shutdown();
			free(current_pack);
			return -1;
		}
		
		// Launch interactor
		if ((msquid_server_in = msgget(IPC_PRIVATE, 0666)) < 0) {
			d_log("CRI: Cannot create IO interaction message queue (in)");
			server_shutdown();
			free(current_pack);
			return -1;
		}
		if ((msquid_server_out = msgget(IPC_PRIVATE, 0666)) < 0) {
			d_log("CRI: Cannot create IO interaction message queue (out)");
			server_shutdown();
			free(current_pack);
			return -1;
		}
		if (pthread_create(&interactor_id, NULL, interactor, NULL) != 0) {
			d_log("CRI: Cannot launch IO handler");
			server_shutdown();
			free(current_pack);
			return -1;
		}
		
		// Log launch
		sprintf(log_message,
				"ACT: Server launched. Log: \"%s\"; Map: \"%s\"; Max. players: %d",
				log_file, map_file, players_total);
		d_log(log_message);
		strcpy(message_out.text, log_message);
		msgsnd(msquid_server_out,
			   &message_out, LENGTH_LOG_MESSAGE, IPC_NOWAIT);
		
		free(map_file);
		free(log_file);
	}
	
	// Connect clients //
	
	// Todo: Enable all unit capabilities (e.g. name)
	// Todo: Add DP_RECONNECT processing
	{
		int get_res;  // Packet send result
		
		units_total = 0;
		
		while (1) {
			// Get command from interactor
			if (msgrcv(msquid_server_in,
					   &message_in, LENGTH_LOG_MESSAGE, 0,
					   MSG_NOERROR | IPC_NOWAIT) < 0) {
				// No commands
				errno = 0;
			}
			else {
				if (strcmp(message_in.text, "launch") == 0) {
					break;
				}
				if (strncmp(message_in.text, "msg", 3) == 0) {
					current_pack ->type = DP_MESSAGE;
					current_pack ->meta = (int)strtol(message_in.text + 4, NULL,
													  16);
					if ((current_pack ->meta < 0) ||
							(current_pack ->meta > 16)) {  // Only 1-symbol base-16 meta allowed
						continue;
					}
					strncpy(current_pack ->data, (message_in.text + 6),
							SZ_PURE_MESSAGE);
					
					int i;
					for (i = 0; i < units_total; i++) {
						d_server_send(current_pack, UPACK_SIZE(SZ_PURE_MESSAGE),
									  hr_database[i], NET_REPEAT_SERVER);
					}
					
					sprintf(log_message,
							"ACT: Message (%d) sent: %s",
							current_pack ->meta, current_pack ->data);
					d_log(log_message);
					strcpy(message_out.text, log_message);
					msgsnd(msquid_server_out,
						   &message_out, LENGTH_LOG_MESSAGE, IPC_NOWAIT);
					errno = 0;
				}
				if (strcmp(message_in.text, "exit") == 0) {
					server_shutdown();
					free(current_pack);
					return 0;
				}
				if (strcmp(message_in.text, "status") == 0) {
					sprintf(message_out.text,
							"Waiting for players (%d/%d)",
							units_total, players_total);
					msgsnd(msquid_server_out,
						   &message_out, LENGTH_LOG_MESSAGE, IPC_NOWAIT);
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
						"ACT: Connected client #%04d (%s:%d)",
						i, current_hr.ip, current_hr.port);
				d_log(log_message);
				strcpy(message_out.text, log_message);
				msgsnd(msquid_server_out,
					   &message_out, LENGTH_LOG_MESSAGE, IPC_NOWAIT);
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
					sprintf(log_message,
							"ACT: Reconnect request from %s:%d refused",
							current_hr.ip, current_hr.port);
					d_log(log_message);
					strcpy(message_out.text, log_message);
					msgsnd(msquid_server_out,
						   &message_out, LENGTH_LOG_MESSAGE, IPC_NOWAIT);
					errno = 0;
					continue;
				}
				
				// Client found, respond success
				current_pack ->stamp = DP_S_SUCCESS;
				d_server_send(current_pack, UPACK_SIZE(1),
							  current_hr, NET_REPEAT_SERVER);
				
				// Log
				sprintf(log_message,
						"ACT: Reconnected client #%04d (%s:%d)",
						i, current_hr.ip, current_hr.port);
				d_log(log_message);
				strcpy(message_out.text, log_message);
				msgsnd(msquid_server_out,
					   &message_out, LENGTH_LOG_MESSAGE, IPC_NOWAIT);
				errno = 0;
			}
		}
	}
	
	// Prepare for the game //
	
	d_log("ACT: Game prepare procedure started");
	{
		// Optimize hr_database
		if (units_total < players_total) {
			hr_database = realloc(hr_database, sizeof(HR_ADDRESS) * units_total);
			players_total = units_total;
		}
		
		// Extra actions should be placed here for NPC addition //
		
		// Allocate buffers
		if ((units = realloc(units, sizeof(UNIT) * units_total)) == NULL) {
			d_log("CRI: Units database reallocation unsuccessful");
			server_shutdown();
			free(current_pack);
			return -1;
		}
		if ((units_cmd = realloc(units_cmd, (size_t)units_total)) == NULL) {
			d_log("CRI: Units' commands database reallocation unsuccessful");
			server_shutdown();
			free(current_pack);
			return -1;
		}
		if ((packs_in = malloc((size_t)(SZ_CLIENT_ACTION * players_total)))
			== NULL) {
			d_log("CRI: Network buffer (in) allocation unsuccessful");
			server_shutdown();
			free(current_pack);
			return -1;
		}
		if ((packs_out = malloc((size_t)(SZ_GAME * players_total)))
			== NULL) {
			d_log("CRI: Network buffer (out) allocation unsuccessful");
			server_shutdown();
			free(current_pack);
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
		UPACK_HEAD* pack_ptr = (UPACK_HEAD*)packs_in;
		for (i = 0; i < players_total; i++) {
			pack_ptr ->type = DP_CLIENT_ACTION;
			pack_ptr ->stamp = 0;
			pack_ptr ->meta = i;
			pack_ptr ->data[0] = CMD_NONE;
			pack_ptr = (void*)pack_ptr + SZ_CLIENT_ACTION;
		}
		
		// Launch receiver. From now on packets can't be received by main()
		pthread_mutex_init(&packs_in_mutex, NULL);
		if (pthread_create(&packs_in_id, NULL, packs_in_processor, NULL) != 0) {
			d_log("CRI: Receiver launch failed");
			server_shutdown();
			free(current_pack);
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
			free(current_pack);
			return -1;
		}
	}
	d_log("ACT: Game prepare procedure (stage 1) completed");
	
	// Launch the game //
	
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
				free(current_pack);
				return -1;
			}
			level[d_level_pos(units[i].x, units[i].y)].units[0] = &units[i];
		}
		
		// Delay, allowing players to prepare for the game
		gettimeofday(&time_last_update, NULL);
		while (1) {
			// Process commands from interactor
			while (msgrcv(msquid_server_in,
						  &message_in, LENGTH_LOG_MESSAGE, 0,
						  MSG_NOERROR | IPC_NOWAIT) >= 0) {
				if (strcmp(message_in.text, "exit") == 0) {
					d_log("ACT: \"exit\" command received");
					server_shutdown();
					free(current_pack);
					return -1;
				}
				if (strcmp(message_in.text, "status") == 0) {
					sprintf(message_out.text,
							"Game is starting");
					msgsnd(msquid_server_out,
						   &message_out, LENGTH_LOG_MESSAGE, IPC_NOWAIT);
				}
			}
			
			// Calculate time interval and start next tick if necessary
			if (gettimeofday(&(time_now), NULL) < 0) {
				break;
			}
			interval_time = (double)(time_now.tv_sec - time_last_update.tv_sec);
			interval_time += ((double)(time_now.tv_usec -
									   time_last_update.tv_usec)) / 1000000.0;
			if (SERVER_DELAY_PREPARE - interval_time < 0.0001) {
				time_last_update = time_now;
				break;
			}
		}
		errno = 0;
		
		d_log("ACT: Game prepare procedure (stage 2) completed");
		
		// Send begin message
		current_pack ->type = DP_GAME_BEGIN;
		current_pack ->stamp = 0;
		for (i = 0; i < players_total; i++) {
			current_pack ->meta = i;
			d_server_send(current_pack, UPACK_SIZE(1),
						  hr_database[i], NET_REPEAT_SERVER);
		}
		d_all_delay(SERVER_DELAY_BEGIN);
	}
	
	// Game process //
	
	sprintf(log_message, "ACT: Game starts");
	d_log(log_message);
	strcpy(message_out.text, log_message);
	msgsnd(msquid_server_out,
		   &message_out, LENGTH_LOG_MESSAGE, IPC_NOWAIT);
	errno = 0;
	
	int is_game_over = 0;
	
	int i;
	UPACK_HEAD* current;  // Pointer to current in/out packet
	
	int refresh_result = -1;
	int players_dead = 0;
	
	int x;
	int y;
	int j;
	int health_percent;
	
	HR_ADDRESS winner_hr;
	int winner_id = -1;
	
	while (1) {
		sprintf(log_message, "ACT: Tick %lu",
				tick);
		d_log(log_message);
		
		while (1) {
			// Process console commands
			while (msgrcv(msquid_server_in,
						  &message_in, LENGTH_LOG_MESSAGE, 0,
						  MSG_NOERROR | IPC_NOWAIT) >= 0) {
				if (strcmp(message_in.text, "exit") == 0) {
					d_log("ACT: \"exit\" command received");
					is_game_over = 1;
					break;
				}
				if (strcmp(message_in.text, "status") == 0) {
					sprintf(message_out.text,
							"Game is on (tick %05lu); Players: %d/%d",
							tick, players_total - players_dead, players_total);
					msgsnd(msquid_server_out,
						   &message_out, LENGTH_LOG_MESSAGE, IPC_NOWAIT);
				}
				if (strncmp(message_in.text, "kill", 4) == 0) {
					int kick_id;
					char kick_str[5];
					sscanf(message_in.text + 5, "%s", kick_str);
					kick_id = (int)strtol(kick_str, NULL, 10);
					if ((errno == ERANGE) ||
							(kick_id < 0) || (kick_id > units_total)) {
						sprintf(message_out.text,
								"No such unit. Use valid unit ID");
						msgsnd(msquid_server_out,
							   &message_out, LENGTH_LOG_MESSAGE, IPC_NOWAIT);
						errno = 0;
						continue;
					}
					units[kick_id].health = 0;
					sprintf(log_message,
							"GAM: #%04d killed by command",
							kick_id);
					d_log(log_message);
					strcpy(message_out.text, log_message);
					msgsnd(msquid_server_out,
						   &message_out, LENGTH_LOG_MESSAGE, IPC_NOWAIT);
					errno = 0;
				}
			}
			
			// Get time and check game over
			if ((gettimeofday(&(time_now), NULL) < 0) || (is_game_over != 0)) {
				break;
			}
			
			// Calculate time interval and start next tick if necessary
			interval_time = (double)(time_now.tv_sec - time_last_update.tv_sec);
			interval_time += ((double)(time_now.tv_usec -
									   time_last_update.tv_usec)) / 1000000.0;
			if (TICK_LENGTH - interval_time < 0.0001) {
				time_last_update = time_now;
				break;
			}
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
				// pass
			}
			else {
				units_cmd[i] = current ->data[0];
				current ->type = DP_NONE;
			}
			current = (void*)current + SZ_CLIENT_ACTION;
		}
		pthread_mutex_unlock(&packs_in_mutex);
		
		// Extra actions should be placed here for NPC addition //
		
		
		// Update game situation
		if (d_game_update() < 0) {
			sprintf(log_message,
					"ERR: Game update failed at tick %lu",
					tick);
			d_log(log_message);
			strcpy(message_out.text, log_message);
			msgsnd(msquid_server_out,
				   &message_out, LENGTH_LOG_MESSAGE, IPC_NOWAIT);
			errno = 0;
		}
		
		// Refresh game and change tick
		refresh_result = d_game_refresh();
		if (refresh_result < -1) {
			sprintf(log_message,
					"ERR: Game check failed at tick %lu",
					tick);
			d_log(log_message);
			strcpy(message_out.text, log_message);
			msgsnd(msquid_server_out,
				   &message_out, LENGTH_LOG_MESSAGE, IPC_NOWAIT);
			errno = 0;
		}
		
		// Send response to players
		current = (UPACK_HEAD*)packs_out;
		pthread_mutex_lock(&packs_out_mutex);
		for (i = 0; i < players_total; i++) {
			current ->stamp = tick;
			
			// Form a map for a player
			j = 0;
			for (y = units[i].y - (CLIENT_FIELD_HEIGHT / 2);
				 y < units[i].y + (CLIENT_FIELD_HEIGHT / 2) + 1;
				 y++) {
				for (x = units[i].x - (CLIENT_FIELD_WIDTH / 2);
					 x < units[i].x + (CLIENT_FIELD_WIDTH / 2) + 1;
					 x++) {
					if ((x >= level_width) || (x < 0) ||
							(y >= level_height) || (y < 0)) {
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
						
						if (level[d_level_pos(x, y)].units[0] != NULL) {
							current ->data[j] =
									level[d_level_pos(x, y)].units[0] ->type;
						}
					}
					
					j += 1;
				}
			}
			
			if ((units[i].health <= 0) || (refresh_result < 0)) {
				current ->type = DP_GAME_OVER;
				strcpy(client_message, "lose");
			}
			else if (refresh_result > 0) {
				current ->type = DP_GAME_OVER;
				strcpy(client_message, "win");
				winner_hr = hr_database[i];
				winner_id = i;
			}
			else {
				current ->type = DP_GAME;
				strcpy(client_message, " ");  // Default initializer
			}
			
			health_percent = (int)ceil(
					(
						(float)units[i].health /
						(float)(level_passive_health_reduction *
								level_passive_turns)
					) * 11.0
			);
			
			sprintf(current ->data + CLIENT_FIELD_AREA,
					"%04d %02d %s %04d %s",
					units[i].health,
					health_percent,
					units[i].weapon.name,
					units[i].weapon.charge,
					client_message);
			
			current = (void*)current + SZ_GAME;
		}
		
		packs_out_flag = 1;
		pthread_mutex_unlock(&packs_out_mutex);
		pthread_cond_broadcast(&packs_out_cond);
		
		if (refresh_result != 0) {
			break;
		}
	}
	
	if (refresh_result > 0) {
		sprintf(log_message, "ACT: Game ended. Winner is #%04d (%s:%d).",
				winner_id, winner_hr.ip, winner_hr.port);
	}
	else {
		sprintf(log_message, "ACT: Game ended with no winners.");
	}
	d_log(log_message);
	strcpy(message_out.text, log_message);
	msgsnd(msquid_server_out, &message_out, LENGTH_LOG_MESSAGE, 0);
	
	server_shutdown();
	free(current_pack);
	
	return 0;
}



