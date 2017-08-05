//
// Created by egorov on 30.04.17.
//

#ifndef LEVEL_H
#define LEVEL_H

#include "../all/gamekey.h"

#include "consoleColor.h"



int reductionHealth;
int reductionHealthWhileStand;
int rowsCount;
int columnsCount;
int range_of_damage;
int damage;
int heartHeal;
int poisoningEffect;


char **levelData0;

unsigned char GetRenderCellSymbol(unsigned char cellSymbol);

ConsoleColor GetRenderCellSymbolColor(unsigned char cellSymbol);


ConsoleColor GetRenderCellSymbolBackgroundColor(unsigned char cellSymbol);

ConsoleColor GetRenderHeroColor( int heroHealth );

void read_from_file(char* pathname);



#endif //LEVEL_H
