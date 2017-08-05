//
// Created by egorov on 30.04.17.
//

#ifndef LEVEL_H
#define LEVEL_H

#include "consoleColor.h"


int reductionHealth;
int reductionHealthWhileStand;
int rowsCount;
int columnsCount;
int range_of_damage;
int damage;
int heartHeal;
int poisoningEffect;


extern const unsigned char CellSymbol_Empty;
extern const unsigned char CellSymbol_Wall;
extern const unsigned char CellSymbol_Hero;
extern const unsigned char CellSymbol_Heart;
extern const unsigned char CellSymbol_Poison;
extern const unsigned char CellSymbol_Bomb;


char **levelData0;

unsigned char GetRenderCellSymbol(unsigned char cellSymbol);

ConsoleColor GetRenderCellSymbolColor(unsigned char cellSymbol);


ConsoleColor GetRenderCellSymbolBackgroundColor(unsigned char cellSymbol);

ConsoleColor GetRenderHeroColor( int heroHealth );

void read_from_file(char* pathname);



#endif //LEVEL_H
