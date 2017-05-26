//
// Created by egorov on 15.05.17.
//

#ifndef DOOM_SERVER_RENDERSYSTEM_H_H
#define DOOM_SERVER_RENDERSYSTEM_H_H

#include "consoleColor.h"


/////////////////////////////////////
// Constants
extern const int screenRows;
extern const int screenColumns;


/////////////////////////////////////
// Struct
typedef struct ConsoleSymbolData {
    char symbol;
    ConsoleColor symbolColor;
    ConsoleColor backgroundColor;
} ConsoleSymbolData;


/////////////////////////////////////
// Functions
void RenderSystemInitialize();
void RenderSystemClear();
void RenderSystemDrawChar( int r, int c, char symbol, ConsoleColor symbolColor, ConsoleColor backgroundColor );
void RenderSystemDrawText( int r, int c, const char* text, ConsoleColor symbolColor, ConsoleColor backgroundColor );
void RenderSystemFlush();


#endif //DOOM_SERVER_RENDERSYSTEM_H_H
