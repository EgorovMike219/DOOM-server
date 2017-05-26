#ifndef CLIENT_FUNCTIONS_H
#define CLIENT_FUNCTIONS_H

#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "dconnect.h"
#include "dconnect_settings.h"
#include "consoleColor.h"

#define screenRows  35
#define screenColumns 80


typedef enum {forward = 119, left = 97, backward = 115, right = 100,
              bomb = 98, quit = 113} keyboard_key;


UPACK_HEAD* pack_to_send;
TICK_TYPE tick;

typedef struct ConsoleSymbolData {
    char symbol;
    ConsoleColor symbolColor;
    ConsoleColor backgroundColor;
} ConsoleSymbolData;

ConsoleSymbolData screenBuffer[screenRows][screenColumns];


void ClientStartGame();

void* SendData();

void* RenderScreen(); //renders client screen

void* GetData_and_RenderScreen(); //gets data from the server







#endif // CLIENT_FUNCTIONS_H
