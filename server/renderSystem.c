#include "renderSystem.h"
#include <stdio.h>
#include <ncurses.h>
#include <stdlib.h>


/////////////////////////////////////
// Logics variables
#define screenRows  35
#define screenColumns 80

ConsoleSymbolData backBuffer[screenRows][screenColumns];
ConsoleSymbolData screenBuffer[screenRows][screenColumns];


/////////////////////////////////////
// Functions
void RenderSystemInitialize() {
    /*if (!has_colors()) {
        endwin();
        printf("Цвета не поддерживаются");
        exit(1);
    }
    start_color();*/

    initscr(); // Переход в curses-режим
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);
    noecho();

    // Initialize buffers
    for (int r = 0; r < screenRows; r++ ) {
        for (int c = 0; c < screenColumns; c++ ) {
            backBuffer[r][c].symbol = 0;
            backBuffer[r][c].symbolColor = ConsoleColor_Gray;
            backBuffer[r][c].backgroundColor = ConsoleColor_Black;

            screenBuffer[r][c] = backBuffer[r][c];
        }
    }
}

void RenderSystemClear() {
    for (int r = 0; r < screenRows; r++ ) {
        for (int c = 0; c < screenColumns; c++ ) {
            backBuffer[r][c].symbol = 0;
            backBuffer[r][c].backgroundColor = ConsoleColor_Black;
        }
    }
}

void RenderSystemDrawChar(int r, int c, char symbol, ConsoleColor symbolColor, ConsoleColor backgroundColor ) {
    backBuffer[r][c].symbol = symbol;
    backBuffer[r][c].symbolColor = symbolColor;
    backBuffer[r][c].backgroundColor = backgroundColor;
}

void RenderSystemDrawText( int r, int c, const char* text, ConsoleColor symbolColor, ConsoleColor backgroundColor ) {
    int column = c;
    char symbol = *text;

    while( symbol != 0 )
    {
        RenderSystemDrawChar(r, column, symbol, symbolColor, backgroundColor);

        text++;
        column++;
        symbol = *text;
    }
}

void SetConsoleCursor( int r, int c ) {
    move(r,c);
}

int SetConsoleColor(ConsoleColor symbolColor, ConsoleColor backgroundColor) {
    return 1; // можно возвращать init_pair
}

void RenderSystemFlush() {
    bool screenBufferModified = false;

    for (int r = 0; r < screenRows; r++) {
        for (int c = 0; c < screenColumns; c++) {
            if (   ( backBuffer[r][c].symbol != screenBuffer[r][c].symbol )
                || ( backBuffer[r][c].symbolColor != screenBuffer[r][c].symbolColor )
                || ( backBuffer[r][c].backgroundColor != screenBuffer[r][c].backgroundColor ) ) {
                // Copy symbol data from back to screen buffer
                screenBuffer[r][c] = backBuffer[r][c];

                // Draw symbol in (r,c) position
                SetConsoleCursor( r, c );
                SetConsoleColor( screenBuffer[r][c].symbolColor, screenBuffer[r][c].backgroundColor );
                printw("%c", screenBuffer[r][c].symbol);

                screenBufferModified = true;
            }
        }
    }

    refresh(); // обновить экран, нужен для ncurses

    // Return console cursor to (0,0)
    if (screenBufferModified)
        SetConsoleCursor(0,0);
}



