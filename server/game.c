#include "game.h"
#include <time.h>
#include <stdio.h>
#include <ncurses.h>
#include <stdlib.h>


const int maxUnitsCount = 35;

const float cellBeginValue = 0.001f;
const float cellEndValue = 0.999f;


/////////////////////////////////////
// Logics variables
bool isGameActive = true;
clock_t clockLastFrame = 0;

int framesCounter = 0;
float framesTimeCounter = 0;
int fps = 0;

int unitsCount = 0;
int heroIndex = 0;

void SetupSystem() {
    //srand(time(0));
    // Initialize render system
    RenderSystemInitialize();
}

void Initialize() {
    //read levelData0 and others
    read_from_file("/home/egorov/CLionProjects/DOOM-server/server/map.txt");

    // Set clockLastFrame start value
    clockLastFrame = clock();

    unitsData = (UnitData*)malloc(maxUnitsCount * sizeof(UnitData));
    levelData = (char**)malloc(rowsCount * sizeof(char*));
    for (int i = 0; i < rowsCount; ++i) {
        levelData[i] = (char *) malloc(columnsCount * sizeof(char));
    }
    unitsCount = 0;

    int r, c;
    // Load level
    for (r = 0; r < rowsCount; r++ )
    {
        for (c = 0; c < columnsCount; c++ )
        {
            unsigned char cellSymbol = levelData0[r][c];

            levelData[r][c] = cellSymbol;

            if (cellSymbol == CellSymbol_Hero) {
                heroIndex = unitsCount;
                UnitType unitType = GetUnitTypeFromCell(cellSymbol);
                unitsData[unitsCount].type = unitType;
                unitsData[unitsCount].y = (float)(r);
                unitsData[unitsCount].x = (float)(c);
                unitsData[unitsCount].health = GetUnitDefaultHealth(unitType);
                unitsData[unitsCount].speed_x = 0.0;
                unitsData[unitsCount].speed_y = 0.0;
                unitsData[unitsCount].order_x = UnitOrder_None;
                unitsData[unitsCount].order_y = UnitOrder_None;
                unitsCount++;
            }
        }
    }
}

void Render() {
    // Start frame
    //RenderSystemClear();

    // Draw level
    int r,c;
    for (r = 0; r < rowsCount; r++) {
        for (c = 0; c < columnsCount; c++) {
            unsigned char cellSymbol = levelData[r][c];

            unsigned char renderSymbol  = GetRenderCellSymbol(cellSymbol);
            ConsoleColor symbolColor = GetRenderCellSymbolColor( cellSymbol );
            ConsoleColor backgroundColor = GetRenderCellSymbolBackgroundColor( cellSymbol );

            if (cellSymbol == CellSymbol_Hero )
                symbolColor = GetRenderHeroColor( unitsData[heroIndex].health );

            RenderSystemDrawChar(r, c, renderSymbol, symbolColor, backgroundColor);
        }
    }

    // Draw FPS
    char textBuffer[32];
    sprintf(textBuffer, "FPS: %d", fps);
    RenderSystemDrawText(rowsCount + 3, 0, textBuffer, ConsoleColor_Gray, ConsoleColor_Black);

    move(rowsCount + 5, 0);
    printw("unit.x = %f\n", unitsData[heroIndex].x);
    printw("unit.y = %f\n", unitsData[heroIndex].y);
    printw("unit.health = %d\n", unitsData[heroIndex].health);
    refresh();

    // End frame
    RenderSystemFlush();
}

void Update() {
    // Calculate delta time
    clock_t clockNow = clock();
    clock_t deltaClock = clockNow - clockLastFrame;
    float deltaTime = (float)(deltaClock) / CLOCKS_PER_SEC;
    clockLastFrame = clockNow;


    // Calculate FPS
    framesCounter++;
    framesTimeCounter += deltaTime;
    if( framesTimeCounter >= 1.0 )
    {
        framesTimeCounter -= 1.0;
        fps = framesCounter;
        framesCounter = 0;
    }

    int c;
    c = getch();
    // Hero control
    if ( c == 'w' ) {
        unitsData[heroIndex].order_y = UnitOrder_Backward;
    }
    if ( c == 's' ) {
        unitsData[heroIndex].order_y = UnitOrder_Forward;
    }

    if ( c == 'a' ) {
        unitsData[heroIndex].order_x = UnitOrder_Backward;
    }
    if ( c == 'd' ) {
        unitsData[heroIndex].order_x = UnitOrder_Forward;
    }
    if ( c == 'b' ) {
        SetBomb(&unitsData[heroIndex]);
    }
    if ( c == 'q' ) {
        isGameActive = false;
    }

    if (c != -1) {
        RenderSystemDrawChar(rowsCount + 4, 0, (char)(c), ConsoleColor_Gray, ConsoleColor_Black);
    }

    // Update all units
    for( int u = 0; u < unitsCount; u++ )
        UpdateUnit( &unitsData[u], deltaTime );


    // Hero dead
    if( unitsData[heroIndex].health <= 0 ) // для одиночной игры
        Initialize();
}

void Shutdown(){
    endwin();  // Выход из curses-режима. Обязательная команда.
    int i;
    for (i = 0; i < rowsCount; ++i) {
        free(levelData0[i]);
        free(levelData[i]);
    }
    free(levelData0);
    free(levelData);
    free(unitsData);
}

bool MoveUnitTo(UnitData* pointerToUnitData, float newX, float newY)
{
    // Ignore dead units
    if (pointerToUnitData->health <= 0)
    {
        return false;
    }

    int row = (int)(newY);
    int column = (int)(newX);
    int oldRow = (int)(pointerToUnitData->y);
    int oldColumn = (int)(pointerToUnitData->x);

    unsigned char unitSymbol = levelData[oldRow][oldColumn];
    unsigned char destinationCellSymbol = levelData[row][column];
    bool canMoveToCell = false;

    // All units actions

    // Empty cell
    if (destinationCellSymbol == CellSymbol_Empty) {
        canMoveToCell = true;
    }
    // Units cells
    if (destinationCellSymbol == CellSymbol_Hero) {
        UnitType destinationUnitType = GetUnitTypeFromCell(destinationCellSymbol);

        // Find enemy unit struct
        for (int u = 0; u < unitsCount; u++)
        {
            // Ignore dead units
            if (unitsData[u].health <= 0)
                continue;

            // Ignore yourself
            if (&unitsData[u] == pointerToUnitData)
                continue;

            if ((int)(unitsData[u].y) == row && (int)(unitsData[u].x) == column)
            {
                // Calculate weapon damage
                int damage = GetWeaponDamage(WeaponType_Fist);

                // Deal damage
                unitsData[u].health -= damage;

                // If enemy unit die
                if (unitsData[u].health <= 0)
                {
                    levelData[row][column] = CellSymbol_Empty;
                }

                break;
            }
        }
    }

    // Only hero actions
    if (pointerToUnitData->type == UnitType_Hero)
    {
        // Heart
        if (destinationCellSymbol == CellSymbol_Heart) {
            canMoveToCell = true;
            unitsData[heroIndex].health += heartHeal;
        }
        // Poison
        if (destinationCellSymbol == CellSymbol_Poison) {
            canMoveToCell = true;
            unitsData[heroIndex].health -= poisoningEffect;
        }
    }

    if (canMoveToCell)
    {
        // Remove unit symbol from previous position
        levelData[oldRow][oldColumn] = CellSymbol_Empty;

        // Set new hero position
        pointerToUnitData->x = newX;
        pointerToUnitData->y = newY;

        // Set hero symbol to new position
        levelData[row][column] = unitSymbol;
    }

    return canMoveToCell;
}

void SetBomb(UnitData* pointerToUnitData) {
    pointerToUnitData->count_of_charge -= 1;
    for (int u = 0; u < unitsCount; u++) {
        // Ignore dead units
        if (unitsData[u].health <= 0)
            continue;

        // Ignore yourself
        if (&unitsData[u] == pointerToUnitData)
            continue;

        int row = (int)(pointerToUnitData->y);
        int column = (int)(pointerToUnitData->x);
        if ((int)(unitsData[u].y) <= row + range_of_damage &&
            (int)(unitsData[u].y) >= row - range_of_damage &&
            (int)(unitsData[u].x) <= column + range_of_damage &&
            (int)(unitsData[u].x) >= column - range_of_damage) {
            unitsData[u].health -= (int)(damage * range_of_damage / (range_of_damage + 1)); // пока без препятствий
        }
    }

}

void UpdateUnit( UnitData* pointerToUnitData, float deltaTime )
{
    // Unit row and column
    int row = (int)(pointerToUnitData->y);
    int column = (int)(pointerToUnitData->x);


    // X Order
    if (pointerToUnitData->order_x == UnitOrder_Backward) {
        pointerToUnitData->speed_x = -GetUnitSpeed(pointerToUnitData->type);
    }
    else {
        if (pointerToUnitData->order_x == UnitOrder_Forward) {
            pointerToUnitData->speed_x = GetUnitSpeed(pointerToUnitData->type);
        }
        else {
            pointerToUnitData->speed_x = 0;
        }
    }

    // Y Order
    if (pointerToUnitData->order_y == UnitOrder_Backward) {
        pointerToUnitData->speed_y = -GetUnitSpeed(pointerToUnitData->type);
    }
    else {
        if (pointerToUnitData->order_y == UnitOrder_Forward) {
            pointerToUnitData->speed_y = GetUnitSpeed(pointerToUnitData->type);
        }
        else {
            pointerToUnitData->speed_y = 0;
        }
    }


    // New position
    float deltaY = pointerToUnitData->speed_y * deltaTime;
    float deltaX = pointerToUnitData->speed_x * deltaTime;
    float newY = pointerToUnitData->y + deltaY;
    float newX =  pointerToUnitData->x + deltaX;
    int newRow = (int)(newY);
    int newColumn = (int)(newX);


    // Y( row ) step
    if (newRow != row) {
        // If unit can go to cell
        if (newRow < 0) {
            pointerToUnitData->y = row + cellBeginValue;
        } else {
            if (newRow >= rowsCount) {
                pointerToUnitData->y = row + cellEndValue;
            } else {
                if (MoveUnitTo(pointerToUnitData, pointerToUnitData->x, newY) == false) {
                    // Can not move cell down
                    if (newRow > row) {
                        pointerToUnitData->y = row + cellEndValue;
                    } else {
                        pointerToUnitData->y = row + cellBeginValue;
                    }
                }
            }
        }
    } else {
        pointerToUnitData->y = newY;
    }

    // X( column ) step
    if (newColumn != column) {
        // If unit can go to cell
        if (newColumn < 0) {
            pointerToUnitData->x = column + cellBeginValue;
        } else {
            if (newColumn >= columnsCount) {
                pointerToUnitData->x = column + cellEndValue;
            } else {
                if (MoveUnitTo(pointerToUnitData, newX, pointerToUnitData->y) == false) {
                    // Can not move cell right
                    if (newColumn > column) {
                        pointerToUnitData->x = column + cellEndValue;
                    } else {
                        pointerToUnitData->x = column + cellBeginValue;
                    }
                }
            }
        }
    } else {
        pointerToUnitData->x = newX;
    }
    pointerToUnitData->order_x = 0;
    pointerToUnitData->order_y = 0;
    pointerToUnitData->speed_x = 0;
    pointerToUnitData->speed_y = 0;
}
