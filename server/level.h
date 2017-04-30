//
// Created by egorov on 30.04.17.
//

#ifndef DOOM_SERVER_LEVEL_H
#define DOOM_SERVER_LEVEL_H

//#include "consoleColor.h"

int rowsCount;
int columnsCount;
int range_of_damage;
int damage;
const int heartHeal;

const unsigned char CellSymbol_Empty;
const unsigned char CellSymbol_Wall;
const unsigned char CellSymbol_Hero;
const unsigned char CellSymbol_Exit;
const unsigned char CellSymbol_Heart;
const unsigned char CellSymbol_Bomb;


char **levelData0;

/*unsigned char GetRenderCellSymbol(unsigned char cellSymbol)
{
    if (cellSymbol == CellSymbol_Empty) return  ' ';
    if (cellSymbol == CellSymbol_Wall) return  177;
    if (cellSymbol == CellSymbol_Hero) return  2;
    if (cellSymbol == CellSymbol_Heart) return  3;

    return '?';
}

enum ConsoleColor GetRenderCellSymbolColor(unsigned char cellSymbol)
{
    if (cellSymbol == CellSymbol_Empty) return ConsoleColor_Black;
    if (cellSymbol == CellSymbol_Wall) return ConsoleColor_White;
    if (cellSymbol == CellSymbol_Hero) return ConsoleColor_Yellow;
    if (cellSymbol == CellSymbol_Heart) return ConsoleColor_Red;

    return ConsoleColor_Gray;
}*/

void read_from_file(char* pathname);

#endif //DOOM_SERVER_LEVEL_H
