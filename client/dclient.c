#define _GNU_SOURCE  // Non-standart function pthread_yield used

#include <ncurses.h>
#include <pthread.h>
#include <stdlib.h>

#include "../all/dconnect.h"
#include "../all/gamekey.h"

#include "dclient_settings.h"




/// Curses client variable
int screen_width;
/// Curses client variable
int screen_height;
/// Curses client variable
int screen_width_middle;
/// Curses client variable
int screen_height_middle;


/// Current tick
TICK_TYPE tick;

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
		case CELL_EMPTY:
			return (chtype)' ';
		case CELL_WALL:
			return (chtype)'#';
		
		case CELL_PLAYER:
			return (chtype)'@';
		case CELL_ENEMY:
			return (chtype)'$';
		
		case CELL_HEART:
			return (chtype)'+';
		case CELL_POISON:
			return (chtype)'+';
		case CELL_BOMB:
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
		bkgd(COLOR_PAIR(DISPLAY_BACKGROUND));
		clear();
		refresh();
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
	
	/*
	// Draw game info
	attron(COLOR_PAIR(DISPLAY_ID_INFO));
	// TODO: Print game info
	attroff(COLOR_PAIR(DISPLAY_ID_INFO));
	*/
	
	refresh();
}


/**
 * @brief Thread function. Receives all available data from server
 * and calls display_game() each time new DP_GAME packet is received
 */
void* receive_display_game(void *dummy) {
	UPACK_HEAD* pack_to_receive = make_UPACK(UDP_MAX_PACKET_SIZE);
	short keep_receiving = 1;
	
	while (keep_receiving) {
		if (d_client_get(pack_to_receive, UPACK_SIZE(UDP_MAX_PACKET_SIZE),
						 tick) < 0) {
			pthread_yield();
			continue;
		}
		switch(pack_to_receive ->type) {
			case DP_GAME:
				tick = pack_to_receive ->stamp + 1;
				display_game(pack_to_receive);
				pthread_yield();
				break;
			case DP_CLIENT_STOP:
				keep_receiving = 0;
				pthread_yield();
				break;
			default:
				continue;
		}
	}
	
	free(pack_to_receive);
	pthread_exit(NULL);
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
		init_pair(DISPLAY_ID_INFO, COLOR_BLACK, COLOR_YELLOW);
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
			d_all_delay(5.000);
			endwin();
			return -1;
		}
		
		curses_printw_in_middle(screen_height_middle - 1,
								"Welcome to DOOM-592!");
		curses_printw_in_middle(screen_height_middle,
								"\b\b\bServer IP: ");
		refresh();
		scanw("%s", server.ip);
		clear();
		server.port = NET_PORT;
		
		curses_printw_in_middle(screen_height_middle,
								"New game? [y/n] ");
		refresh();
		scanw("%c", &response_newgame);
		clear();
		refresh();
		
		int d_client_connect_result;
		if ((response_newgame == 'n') || (response_newgame == 'N')) {
			d_client_connect_result = d_client_connect(server, 1);
		}
		else {
			d_client_connect_result = d_client_connect(server, 0);
		}
		if (d_client_connect_result < 0) {
			clear();
			curses_printw_in_middle(screen_height_middle,
									"Connection failure");
			refresh();
			d_all_delay(5.000);
			endwin();
			return -1;
		}
	}
	
	// Wait before the game and start
	{
		UPACK_HEAD* pack_to_receive = make_UPACK(UDP_MAX_PACKET_SIZE);
		
		curs_set(0);
		
		curses_printw_in_middle(screen_height_middle,
								"Please wait for other players.");
		curses_printw_in_middle(screen_height_middle + 1,
								"MESSAGES: ");
		refresh();
		
		while (1) {
			if (d_client_get(pack_to_receive,
									UPACK_SIZE(UDP_MAX_PACKET_SIZE), 0) < 0) {
				continue;
			}
			
			if (pack_to_receive ->type == DP_MESSAGE) {
				mvprintw(screen_height_middle + 2 +
								 (int)(pack_to_receive ->stamp), 0,
						 "%s%s", "   * ", pack_to_receive ->data);
				refresh();
			}
			else if ((pack_to_receive ->type == DP_GAME_PREPARE) ||
					(pack_to_receive ->type == DP_GAME_BEGIN)) {
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
		
		// Create thread to receive data from server
		if (pthread_create(&receiver_id, NULL, &receive_display_game, NULL) != 0) {
			curses_printw_in_middle(screen_height_middle,
									"Thread create error");
			refresh();
			d_all_delay(2.500);
			endwin();
			return -1;
		}
		
		free(pack_to_receive);
	}
	
	// Client listening
	UPACK_HEAD* pack_to_send = make_UPACK(1);
	int client_input;
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
				client_command = CMD_BOMB;
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
			|| (client_command == CMD_BOMB)) {
			pack_to_send ->type = DP_CLIENT_ACTION;
			pack_to_send ->data[0] = client_command;
		}
		else if (client_command == CMD_QUIT) {
			pack_to_send ->type = DP_CLIENT_STOP;
		}
		
		pack_to_send ->stamp = tick;
		d_client_send(pack_to_send, UPACK_SIZE(1), NET_REPEAT_CLIENT);
		
		if (pack_to_send ->type == DP_CLIENT_STOP) {
			break;
		}
		
		pthread_yield();
	}
	
	free(pack_to_send);
	
	// Join receiver
	pthread_join(receiver_id, NULL);
	
	// End game
	nocbreak();
	nodelay(stdscr, false);
	keypad(stdscr, false);
	echo();
	
	
	
	clear();
	refresh();
	curses_printw_in_middle(screen_height_middle,
							"Game ended. Thanks for playing!");
	refresh();
	d_all_delay(10.000);
	endwin();
	return 0;
}



