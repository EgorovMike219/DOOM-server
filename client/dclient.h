#ifndef CLIENT_FUNCTIONS_H
#define CLIENT_FUNCTIONS_H

#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "dconnect.h"
#include "dconnect_settings.h"

#define screenRows  10
#define screenColumns 10
#define receive_pack_size (screenColumns * screenRows)


typedef enum {forward = 119, left = 97, backward = 115, right = 100,
              bomb = 98, quit = 113} keyboard_key;


UPACK_HEAD* pack_to_send;
UPACK_HEAD* pack_to_receive;
TICK_TYPE tick;
bool game_is_on;


void ClientStartGame();

void* SendData();

void* GetData_and_RenderScreen(); //gets data from the server

#endif // CLIENT_FUNCTIONS_H
