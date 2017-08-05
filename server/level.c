//
// Created by egorov on 30.04.17.
//
#include "level.h"
#include <stdio.h>
#include <stdlib.h>



unsigned char GetRenderCellSymbol(unsigned char cellSymbol)
{
    if (cellSymbol == CELL_EMPTY) return  ' ';
    if (cellSymbol == CELL_WALL) return  '#';
    if (cellSymbol == CELL_PLAYER) return  'h';
    if (cellSymbol == CELL_HEART) return  '+';
    if (cellSymbol == CELL_POISON) return  '+';
    if (cellSymbol == CELL_BOMB) return  'b';

    return '?';
}

ConsoleColor GetRenderCellSymbolColor(unsigned char cellSymbol) {
    if (cellSymbol == CELL_EMPTY) return ConsoleColor_Black;
    if (cellSymbol == CELL_WALL) return ConsoleColor_White;
    if (cellSymbol == CELL_PLAYER) return ConsoleColor_Yellow;
    if (cellSymbol == CELL_HEART) return ConsoleColor_Red;

    return ConsoleColor_Gray;
}


ConsoleColor GetRenderCellSymbolBackgroundColor(unsigned char cellSymbol) {
    if (cellSymbol == CELL_WALL) return ConsoleColor_White;
    if (cellSymbol == CELL_PLAYER) return ConsoleColor_Gray;
    if (cellSymbol == CELL_HEART) return ConsoleColor_Gray;

    return ConsoleColor_Black;
}

ConsoleColor GetRenderHeroColor( int heroHealth ) {
    return ConsoleColor_Yellow;
}

void read_from_file(char* pathname) {
    FILE *file;
    int i,j;
    file = fopen(pathname, "r");
    if(file == NULL)
    {
        printf("can't open file\n");
    }
    char string[40];
    if (fscanf (file, "%s%s%d", string, string, &reductionHealth) == EOF) {
        printf("can't read value of reductionHealth\n");
        exit(1);
    }
    if (fscanf (file, "%s%s%d", string, string, &reductionHealthWhileStand) == EOF) {
        printf("can't read value of reductionHealthWhileStand\n");
        exit(1);
    }
    if (fscanf (file, "%s%s%d", string, string, &heartHeal) == EOF) {
        printf("can't read value of heartHeal\n");
        exit(1);
    }
    if (fscanf (file, "%s%s%d", string, string, &poisoningEffect) == EOF) {
        printf("can't read value of poisoningEffect\n");
        exit(1);
    }
    if (fscanf (file, "%s%s%d", string, string, &range_of_damage) == EOF) {
        printf("can't read value of range_of_damage\n");
        exit(1);
    }
    if (fscanf (file, "%s%s%d", string, string, &damage) == EOF) {
        printf("can't read value of damage\n");
        exit(1);
    }
    if (fscanf (file, "%s%s%d", string, string, &rowsCount) == EOF) {
        printf("can't read value of rowsCount\n");
        exit(1);
    }
    if (fscanf (file, "%s%s%d", string, string, &columnsCount) == EOF) {
        printf("can't read value of columnsCount\n");
        exit(1);
    }
    fgets(string,columnsCount,file); // cчитываю пустую строку
    levelData0 = (char**)malloc(rowsCount * sizeof(char*));
    for (i = 0; i < rowsCount; ++i) {
        levelData0[i] = (char*)malloc(columnsCount * sizeof(char));
    }
    for (i = 0; i < rowsCount; ++i) {
        char str[columnsCount];
        if (fgets(str, columnsCount + 2, file) == NULL) {
            printf("error");
            exit(1);
        }
        for (j = 0; j < columnsCount; ++j) {
            levelData0[i][j] = str[j];
        }
    }
    fclose(file);
}
