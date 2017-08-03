#include <ncurses.h>
#include <pthread.h>
#include <stdlib.h>

#include "../all/dconnect.h"

#include "dclient_settings.h"





// ========================================================================== //
// GLOBAL CLIENT VARIABLES
// ========================================================================== //

/// Curses variables
int screen_width;
int screen_height;
int screen_width_middle;
int screen_height_middle;

/// Current tick
TICK_TYPE tick;

/// Last actual packet received from server
UPACK_HEAD* pack_to_receive;

/// PID of receive_game_info thread
pthread_t receiver_id;



void curses_printw_in_middle(int line, char *string) {
	mvprintw(line, (screen_width / 2) - ((int)strlen(string) / 2),
			 "%s", string);
}


void curses_define_screen_size(void) {
	screen_width = getmaxx(stdscr);
	screen_height = getmaxy(stdscr);
	screen_width_middle = screen_width / 2;
	screen_height_middle = screen_height / 2;
}




void display_game(void) {
	static uint64_t last_update = 1;
	if (tick - last_update >= CLIENT_REDRAW_TICK_INTERVAL) {
		clear();
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
	init_pair(1, COLOR_WHITE, COLOR_WHITE);
	attron(COLOR_PAIR(1));
	move(height_field_start - CLIENT_FIELD_BORDER,
		 width_field_start - CLIENT_FIELD_BORDER);
	for (x = 0; x < CLIENT_FIELD_WIDTH + 2; x++) {
		addch((chtype)' ');
	}
	for (y = 1; y < CLIENT_FIELD_HEIGHT; y++) {
		move(height_field_start + y - CLIENT_FIELD_BORDER,
			 width_field_start - CLIENT_FIELD_BORDER);
		addch((chtype)' ');
		move(height_field_start + y - CLIENT_FIELD_BORDER,
			 width_field_start + CLIENT_FIELD_WIDTH + CLIENT_FIELD_BORDER - 1);
	}
	move(height_field_start + CLIENT_FIELD_HEIGHT + CLIENT_FIELD_BORDER - 1,
		 width_field_start - CLIENT_FIELD_BORDER);
	for (x = 0; x < CLIENT_FIELD_WIDTH + 2; x++) {
		addch((chtype)' ');
	}
	attroff(COLOR_PAIR(1));
	
	// Draw field
	init_pair(1, COLOR_RED, COLOR_BLACK);
	attron(COLOR_PAIR(1));
	for (x = 0, y = 0; x < CLIENT_FIELD_WIDTH; x++) {
		move(height_field_start + y, width_field_start);
		for (; y < CLIENT_FIELD_HEIGHT; y++) {
			addch((chtype)pack_to_receive ->data[x + y * CLIENT_FIELD_WIDTH]);
		}
	}
	attroff(COLOR_PAIR(1));
	
	// Draw game info
	init_pair(1, COLOR_WHITE, COLOR_BLACK);
	attron(COLOR_PAIR(1));
	// TODO: Print game info
	attroff(COLOR_PAIR(1));
	
	refresh();
}


void* receive_display_game(void *dummy) {
	short keep_receiving = 1;
	while (keep_receiving) {
		if (d_client_get(pack_to_receive, UPACK_SIZE(UDP_MAX_PACKET_SIZE),
						 tick) < 0) {
			continue;
		}
		curses_printw_in_middle(screen_height_middle, "SOMETHING HERE");
		d_all_delay(1.000);
		refresh();
		switch(pack_to_receive ->type) {
			case DP_GAME:
				curses_printw_in_middle(screen_height_middle, "=====GAME=====");
				refresh();
				d_all_delay(1.000);
				tick = pack_to_receive ->stamp + 1;
				display_game();
				break;
			case DP_CLIENT_STOP:
				keep_receiving = 0;
				break;
			default:
				d_all_delay(0.001);
		}
	}
	
	pthread_exit(NULL);
}




int main() {
	// Initialize ncurses
	if (initscr() == NULL) {
		fprintf(stderr, "Unable to initialize graphics (ncurses)\n");
		return -1;
	}
	nocbreak();
	curses_define_screen_size();
	
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
								"Server IP: ");
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
	
	// Connection successful. Allocate memory to get server response normally
	pack_to_receive = make_UPACK(UDP_MAX_PACKET_SIZE);
	
	// Wait before the game and start
	{
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
		start_color();
		nodelay(stdscr, true);
		keypad(stdscr, true);
		curs_set(0);
		noecho();
		
		tick = 1;
		
		// Create thread to receive data from server
		if (!pthread_create(&receiver_id, NULL, &receive_display_game, NULL)) {
			curses_printw_in_middle(screen_height_middle,
									"Thread create error");
			d_all_delay(2.500);
			endwin();
			return -1;
		}
	}
	
	// Client listening
	UPACK_HEAD* pack_to_send = make_UPACK(1);
	int client_input;
	char client_command;
	
	while (1) {
		client_input = wgetch(stdscr);
		
		if (client_input == ERR) {
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
	}
	free(pack_to_send);
	
	// Join receiver
	pthread_join(receiver_id, NULL);
	free(pack_to_receive);
	
	// End game
	curses_printw_in_middle(screen_height_middle,
							"Game ended. Thanks for playing!");
	d_all_delay(10.000);
	endwin();
	return 0;
}



