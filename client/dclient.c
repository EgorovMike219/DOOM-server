#define _GNU_SOURCE  // Non-standart function pthread_yield() used

#include <ncurses.h>
#include <pthread.h>
#include <stdlib.h>

#include "../all/gamekey.h"
#include "../all/dconnect.h"

#include "dclient_settings.h"




/// Curses client variable
int screen_width;

/// Curses client variable
int screen_height;

/// Curses client variable
int screen_width_middle;

/// Curses client variable
int screen_height_middle;


/// This client ID on server
int id;


/// In-game time
TICK_TYPE tick;

pthread_mutex_t tick_mutex;


/// Packet received during the game. @note Allocated by receiver
UPACK_HEAD* pack_shared;


/// receive_game_info thread
pthread_t receiver_id;




/**
 * @brief Print string in the middle of a line
 */
void curses_printw_in_middle(int line, char *string) {
	mvprintw(line, (screen_width / 2) - ((int)strlen(string) / 2),
			 "%s", string);
}


/**
 * @brief Define screen size and update 'curses' client variables
 */
void curses_define_screen_size(void) {
	screen_width = getmaxx(stdscr);
	screen_height = getmaxy(stdscr);
	screen_width_middle = screen_width / 2;
	screen_height_middle = screen_height / 2;
}




/**
 * @brief Change symbol to show it on player screen
 */
chtype convert_symbol(char sym) {
	switch (sym) {
		case ENTITY_EMPTY:
			return (chtype)' ';
		case ENTITY_WALL:
			return (chtype)'#';
		
		case ENTITY_PLAYER:
			return (chtype)'@';
		case ENTITY_ENEMY:
			return (chtype)'$';
		
		case ENTITY_HEART:
			return (chtype)'+';
		case ENTITY_POISON:
			return (chtype)'+';
		case ENTITY_BOMB:
			return (chtype)'*';
		
		default:
			return (chtype)sym;
	}
}


/**
 * @brief Draw game field, player data and additional graphics
 */
void display_game(UPACK_HEAD* pack_to_receive) {
	if (tick % CLIENT_REDRAW_TICK_MODULE == 0) {
		curses_define_screen_size();
	}
	
	int height_start = screen_height_middle + 1
							  - CLIENT_FIELD_HEIGHT / 2
							  - CLIENT_FIELD_BORDER
							  - CLIENT_INFO_SIZE;
	int width_start = screen_width_middle + 1
							 - CLIENT_FIELD_WIDTH / 2
							 - CLIENT_FIELD_BORDER;
	int height_field_start = screen_height_middle + 1
							  - CLIENT_FIELD_HEIGHT / 2;
	int width_field_start = screen_width_middle + 1
							 - CLIENT_FIELD_WIDTH / 2;
	
	int x;
	int y;
	int i;
	
	int health;
	int health_percent;
	char weapon_name[LENGTH_NAME];
	int weapon_charge;
	char command[LENGTH_NET_COMMAND];

	
	// Draw borders
	attron(COLOR_PAIR(DISPLAY_ID_BORDER));
	move(height_field_start - CLIENT_FIELD_BORDER,
		 width_field_start - CLIENT_FIELD_BORDER);
	for (x = 0; x < CLIENT_FIELD_WIDTH + 2; x++) {
		addch((chtype)' ');
	}
	for (y = 1; y < CLIENT_FIELD_HEIGHT + 1; y++) {
		move(height_field_start + y - CLIENT_FIELD_BORDER,
			 width_field_start - CLIENT_FIELD_BORDER);
		addch((chtype)' ');
		move(height_field_start + y - CLIENT_FIELD_BORDER,
			 width_field_start + CLIENT_FIELD_WIDTH + CLIENT_FIELD_BORDER - 1);
		addch((chtype)' ');
	}
	move(height_field_start + CLIENT_FIELD_HEIGHT + CLIENT_FIELD_BORDER - 1,
		 width_field_start - CLIENT_FIELD_BORDER);
	for (x = 0; x < CLIENT_FIELD_WIDTH + 2; x++) {
		addch((chtype)' ');
	}
	attroff(COLOR_PAIR(DISPLAY_ID_BORDER));
	
	// Draw field
	attron(COLOR_PAIR(DISPLAY_ID_FIELD));
	for (y = 0; y < CLIENT_FIELD_HEIGHT; y++) {
		move(height_field_start + y, width_field_start);
		for (x = 0; x < CLIENT_FIELD_WIDTH; x++) {
			addch(convert_symbol(
					pack_to_receive ->data[x + y * CLIENT_FIELD_WIDTH]));
		}
	}
	attroff(COLOR_PAIR(DISPLAY_ID_FIELD));
	
	// Decode game info
	sscanf(pack_to_receive ->data + (CLIENT_FIELD_WIDTH * CLIENT_FIELD_HEIGHT),
		   "%d %d %s %d %s",
		   &health, &health_percent, weapon_name, &weapon_charge, command);
	
	// Draw info
	attron(COLOR_PAIR(DISPLAY_ID_INFO));
	
	move(height_start, width_field_start);
	printw("%s", weapon_name);
	move(height_start + 1, width_field_start);
	printw("%04d", weapon_charge);
	
	move(height_start, width_field_start + 7);
	printw("TICK");
	move(height_start + 1, width_field_start + 7);
	printw("%04d", (int)(tick % 10000));
	
	move(height_start + 2, width_field_start);
	printw("HP       HP");
	attroff(COLOR_PAIR(DISPLAY_ID_INFO));
	
	attron(COLOR_PAIR(DISPLAY_ID_HEALTH));
	move(height_start + 2, width_field_start + 3);
	printw("%05d", health);
	move(height_start + 3, width_field_start);
	for (i = 0; i < health_percent && i < 11; i++) {
		printw("+");
	}
	for (i = health_percent; i < 11; i++) {
		printw(" ");
	}
	attroff(COLOR_PAIR(DISPLAY_ID_HEALTH));
	
	attron(COLOR_PAIR(DISPLAY_BACKGROUND));
	move(height_field_start + CLIENT_FIELD_HEIGHT + CLIENT_FIELD_BORDER,
		 width_field_start + 1);
	printw("DOOM-0592");
	attroff(COLOR_PAIR(DISPLAY_ID_INFO));
	
	refresh();
	redrawwin(stdscr);
}


/**
 * @brief Thread function. Receives all available data from server
 * and calls display_game() each time new DP_GAME packet is received
 */
void* receive_display_game(void* dummy) {
	// Dummy-check
	if (dummy != NULL) {
		return NULL;
	}
	
	pack_shared = make_UPACK(UDP_MAX_PACKET_SIZE);
	int keep_receiving = 1;
	
	while (keep_receiving == 1) {
		if (d_client_get(pack_shared, UPACK_SIZE(UDP_MAX_PACKET_SIZE),
						 tick) < 0) {
			pthread_yield();
			continue;
		}
		switch(pack_shared ->type) {
			case DP_GAME:
				pthread_mutex_lock(&tick_mutex);
				tick = pack_shared ->stamp + 1;
				display_game(pack_shared);
				pthread_mutex_unlock(&tick_mutex);
				pthread_yield();
				break;
			case DP_GAME_OVER:
				keep_receiving = 0;
				pthread_mutex_lock(&tick_mutex);
				tick = 0;
				pthread_mutex_unlock(&tick_mutex);
				break;
			default:
				continue;
		}
	}
	
	return NULL;
}




int main() {
	// Initialize ncurses
	if (initscr() == NULL) {
		fprintf(stderr, "Unable to initialize graphics (ncurses)\n");
		return -1;
	}
	
	// Prepare screen
	{
		nocbreak();
		start_color();
		curses_define_screen_size();
		
		init_pair(DISPLAY_BACKGROUND, COLOR_WHITE, COLOR_BLACK);
		init_pair(DISPLAY_ID_BORDER, COLOR_YELLOW, COLOR_YELLOW);
		init_pair(DISPLAY_ID_FIELD, COLOR_WHITE, COLOR_BLACK);
		init_pair(DISPLAY_ID_INFO, COLOR_WHITE, COLOR_BLUE);
		init_pair(DISPLAY_ID_HEALTH, COLOR_RED, COLOR_WHITE);
		
		bkgd(COLOR_PAIR(DISPLAY_BACKGROUND));
		clear();
		refresh();
	}
	
	// Connect to server
	{
		HR_ADDRESS server;
		char response_newgame;
		
		if (d_all_connect(1) < 0) {
			clear();
			curses_printw_in_middle(screen_height_middle,
									"Connection fatal error");
			refresh();
			d_all_delay(CLIENT_DELAY);
			endwin();
			return -1;
		}
		
		curses_printw_in_middle(screen_height_middle - 1,
								"Welcome to DOOM-592!");
		curses_printw_in_middle(screen_height_middle,
								"\b\b\bServer IP: ");
		refresh();
		scanw("%s", server.ip);
		curs_set(0);
		clear();
		
		server.port = NET_PORT;
		
		if (d_client_connect(server, 0) < 0) {
			curses_printw_in_middle(screen_height_middle,
									"Connection failure");
			refresh();
			d_all_delay(CLIENT_DELAY);
			endwin();
			return -1;
		}
	}
	
	// Wait before the game and start
	{
		UPACK_HEAD* pack_to_receive = make_UPACK(UDP_MAX_PACKET_SIZE);
		
		curses_printw_in_middle(screen_height_middle,
								"Please wait for other players.");
		mvprintw(screen_height_middle + 1, 0,
				 "     MESSAGES FROM SERVER:");
		refresh();
		
		while (1) {
			if (d_client_get(pack_to_receive,
									UPACK_SIZE(UDP_MAX_PACKET_SIZE), 0) < 0) {
				continue;
			}
			
			if (pack_to_receive ->type == DP_MESSAGE) {
				move(screen_height_middle + 2 + (int)(pack_to_receive ->meta),
					 0);
				clrtoeol();
				printw("%s%s", "   * ", pack_to_receive ->data);
				refresh();
			}
			else if ((pack_to_receive ->type == DP_GAME_PREPARE) ||
					(pack_to_receive ->type == DP_GAME_BEGIN)) {
				id = pack_to_receive ->meta;
				clear();
				refresh();
				break;
			}
		}
		
		// Few seconds left
		while (pack_to_receive ->type == DP_GAME_PREPARE) {
			curses_printw_in_middle(screen_height_middle,
									"BE READY! GAME STARTS IN A FEW SECONDS!");
			refresh();
			d_client_get(pack_to_receive, UPACK_SIZE(UDP_MAX_PACKET_SIZE), 0);
		}
		clear();
		curses_printw_in_middle(screen_height_middle,
								"START");
		refresh();
		
		// Game initialization
		cbreak();
		nodelay(stdscr, true);
		keypad(stdscr, true);
		noecho();
		
		tick = 1;
		pthread_mutex_init(&tick_mutex, NULL);
		
		// Create thread to receive data from server
		if (pthread_create(&receiver_id, NULL, &receive_display_game, NULL) != 0) {
			curses_printw_in_middle(screen_height_middle,
									"Thread create error");
			refresh();
			d_all_delay(CLIENT_DELAY);
			endwin();
			return -1;
		}
		
		free(pack_to_receive);
	}
	
	// Client listening
	UPACK_HEAD* pack_to_send = make_UPACK(SZ_CLIENT_ACTION);
	pack_to_send ->meta = id;
	
	int client_input = ' ';
	char client_command;
	int failed_key;
	
	while (1) {
		for (failed_key = 0; failed_key < CLIENT_KEYTRYES; failed_key++) {
			client_input = wgetch(stdscr);
			if (client_input != ERR) {
				break;
			}
		}
		if (failed_key == CLIENT_KEYTRYES) {
			pthread_yield();
			continue;
		}
		
		switch(client_input) {
			case 'w':
			case 'W':
			case KEY_UP:
				client_command = CMD_W;
				break;
			case 'a':
			case 'A':
			case KEY_LEFT:
				client_command = CMD_A;
				break;
			case 's':
			case 'S':
			case KEY_DOWN:
				client_command = CMD_S;
				break;
			case 'd':
			case 'D':
			case KEY_RIGHT:
				client_command = CMD_D;
				break;
			case 'b':
			case 'B':
				client_command = CMD_WEAPON;
				break;
			case 'q':
			case 'Q':
				client_command = CMD_QUIT;
				break;
			default:
				client_command = CMD_NONE;
				break;
		}
		
		if (client_command == CMD_NONE) {
			continue;
		}
		if ((client_command == CMD_W) || (client_command == CMD_A)
			|| (client_command == CMD_S) || (client_command == CMD_D)
			|| (client_command == CMD_WEAPON) || (client_command == CMD_QUIT)) {
			pack_to_send ->type = DP_CLIENT_ACTION;
			pack_to_send ->data[0] = client_command;
		}
		
		pthread_mutex_lock(&tick_mutex);
		pack_to_send ->stamp = tick;
		pthread_mutex_unlock(&tick_mutex);
		
		if (pack_to_send ->stamp == 0) {
			break;
		}
		
		d_client_send(pack_to_send, UPACK_SIZE(LENGTH_NET_COMMAND),
					  NET_REPEAT_CLIENT);
		
		if (pack_to_send ->data[0] == CMD_QUIT) {
			break;
		}
		
		pthread_yield();
	}
	
	free(pack_to_send);
	
	// Join receiver
	pthread_join(receiver_id, NULL);
	
	// Return to normal terminal mode
	nocbreak();
	nodelay(stdscr, false);
	keypad(stdscr, false);
	echo();
	clear();
	refresh();
	
	//
	char command[LENGTH_NET_COMMAND];
	strcpy(command, (char*)pack_shared + (SZ_GAME - LENGTH_NET_COMMAND));
	if (strcmp(command, "win") == 0) {
		curses_printw_in_middle(screen_height_middle,
								"*** Congratulations! You are the champion! ***");
	}
	else if (strcmp(command, "lose") == 0) {
		curses_printw_in_middle(screen_height_middle,
								"Unfortunately, you have lost :-(");
	}
	curses_printw_in_middle(screen_height_middle + 1,
							"Thank you for playing!");
	refresh();
	
	d_all_delay(CLIENT_DELAY * 2.0f);
	endwin();
	
	return 0;
}



