//
// Created by egorov on 30.04.17.
//

#ifndef DOOM_SERVER_HANDLER_H
#define DOOM_SERVER_HANDLER_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <stdbool.h>

#include "consoleColor.h"
#include "level.h"
#include "weaponType.h"
#include "unitType.h"
#include "unitData.h"


/////////////////////////////////////
// Constants
const int maxUnitsCount = 35;


/////////////////////////////////////
// Logics variables

bool isGameActive = true;

char levelData[rowsCount][columnsCount];
bool fogOfWar[rowsCount][columnsCount];

UnitData unitsData[maxUnitsCount];
int unitsCount = 0;
int heroIndex = 0;

char tempBuffer[128];
char statusMessage[200];


/////////////////////////////////////
// Functions
void SetupSystem()
{
    statusMessage[0] = 0;

    srand(time(0));

    consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);

    // Hide console cursor
    CONSOLE_CURSOR_INFO cursorInfo;
    cursorInfo.dwSize = 1;
    cursorInfo.bVisible = 0;
    SetConsoleCursorInfo(consoleHandle, &cursorInfo);
}

void RevealFogOfWar(int row, int column)
{
    for (int r = row - 1; r <= row + 1; r++)
    {
        for (int c = column - 1; c <= column + 1; c++)
        {
            fogOfWar[r][c] = false;  //���� false
        }
    }
}

void Initialize()
{
    unitsCount = 0;

    // Load level
    for (int r = 0; r < rowsCount; r++)
    {
        for (int c = 0; c < columnsCount; c++)
        {
            fogOfWar[r][c] = false;  //���� true


            unsigned char cellSymbol = levelData0[r][c];

            levelData[r][c] = cellSymbol;

            switch (cellSymbol)
            {
                case CellSymbol_Hero:
                    heroIndex = unitsCount;

                case CellSymbol_Orc:
                case CellSymbol_Skeleton:
                {
                    UnitType unitType = GetUnitTypeFromCell(cellSymbol);
                    unitsData[unitsCount].type = unitType;
                    unitsData[unitsCount].row = r;
                    unitsData[unitsCount].column = c;
                    unitsData[unitsCount].weapon = GetUnitDefaultWeapon(unitType);
                    unitsData[unitsCount].health = GetUnitDefaultHealth(unitType);
                    unitsCount++;

                    break;
                }
            }
        }
    }

    // Reveal fog of war
    RevealFogOfWar(unitsData[heroIndex].row, unitsData[heroIndex].column);
}

void Render()
{
    // Move console cursor to (0,0)
    COORD cursorCoord;
    cursorCoord.X = 0;
    cursorCoord.Y = 0;
    SetConsoleCursorPosition(consoleHandle, cursorCoord);


    // Draw game title
    SetConsoleTextAttribute(consoleHandle, ConsoleColor_Green);
    printf("\n\tDUNGEONS AND ORCS");


    // Draw hero HP
    SetConsoleTextAttribute(consoleHandle, ConsoleColor_Red);
    printf("\n\n\tHP: ");
    SetConsoleTextAttribute(consoleHandle, ConsoleColor_White);
    printf("%i", unitsData[heroIndex].health);


    // Draw hero weapon
    SetConsoleTextAttribute(consoleHandle, ConsoleColor_Cyan);
    printf("     Weapon: ");
    SetConsoleTextAttribute(consoleHandle, ConsoleColor_White);
    printf("%s", GetWeaponName(unitsData[heroIndex].weapon));
    SetConsoleTextAttribute(consoleHandle, ConsoleColor_Gray);
    printf(" (Dmg: %i)               ", GetWeaponDamage(unitsData[heroIndex].weapon));


    // Draw level
    printf("\n\n\t");
    for (int r = 0; r < rowsCount; r++)
    {
        for (int c = 0; c < columnsCount; c++)
        {
            if (fogOfWar[r][c] == false)
            {
                unsigned char cellSymbol = levelData[r][c];

                unsigned char renderCellSymbol = GetRenderCellSymbol(cellSymbol);
                ConsoleColor cellColor = GetRenderCellSymbolColor(cellSymbol);

                SetConsoleTextAttribute(consoleHandle, cellColor);
                printf("%c", renderCellSymbol);
            }
            else
            {
                SetConsoleTextAttribute(consoleHandle, fogOfWarRenderColor);
                printf("%c", fogOfWarRenderSymbol);
            }
        }
        printf("\n\t");
    }


    // Fill status message with spaces
    while (strlen(statusMessage) <  (sizeof(statusMessage) - 1))
    {
        strcat_s(statusMessage, " ");
    }
    SetConsoleTextAttribute(consoleHandle, ConsoleColor_Yellow);
    printf("\n\n\t%s", statusMessage);

    // Clear status message
    statusMessage[0] = 0;
}

void MoveUnitTo(UnitData* pointerToUnitData, int row, int column)
{
    // Ignore dead units
    if (pointerToUnitData->health <= 0)
    {
        return;
    }

    unsigned char unitSymbol = levelData[pointerToUnitData->row][pointerToUnitData->column];
    unsigned char destinationCellSymbol = levelData[row][column];
    bool canMoveToCell = false;

    // All units actions
    switch (destinationCellSymbol)
    {
        // Empty cell
        case CellSymbol_Empty:
        {
            canMoveToCell = true;
            break;
        }

            // Units cells
        case CellSymbol_Hero:
        case CellSymbol_Orc:
        case CellSymbol_Skeleton:
        {
            UnitType destinationUnitType = GetUnitTypeFromCell(destinationCellSymbol);

            // If destination unit have other type
            if (pointerToUnitData->type != destinationUnitType)
            {
                // Find enemy unit struct
                for (int u = 0; u < unitsCount; u++)
                {
                    // Ignore dead units
                    if (unitsData[u].health <= 0)
                        continue;

                    if (unitsData[u].row == row && unitsData[u].column == column)
                    {
                        // Calculate weapon damage
                        int damage = GetWeaponDamage(pointerToUnitData->weapon);

                        // Deal damage
                        unitsData[u].health = unitsData[u].health - damage;

                        // Add to status message
                        sprintf_s(tempBuffer, " %s dealt %i damage to %s.", GetUnitName(pointerToUnitData->type), damage, GetUnitName(destinationUnitType));
                        strcat_s(statusMessage, tempBuffer);

                        // If enemy unit die
                        if (unitsData[u].health <= 0.0f)
                        {
                            levelData[row][column] = CellSymbol_Empty;

                            // Add to status message
                            sprintf_s(tempBuffer, " %s died.", GetUnitName(destinationUnitType), damage, GetUnitName(destinationUnitType));
                            strcat_s(statusMessage, tempBuffer);
                        }

                        break;
                    }
                }
            }

            break;
        }
    }

    // Only hero actions
    if (pointerToUnitData->type == UnitType_Hero)
    {
        switch (destinationCellSymbol)
        {
            // Weapon Cell
            case CellSymbol_Stick:
            case CellSymbol_Club:
            case CellSymbol_Spear:
            case CellSymbol_Saber:
            {
                canMoveToCell = true;

                WeaponType weaponType = GetWeaponTypeFromCell(destinationCellSymbol);
                if (unitsData[heroIndex].weapon < weaponType)
                {
                    unitsData[heroIndex].weapon = weaponType;
                }

                // Add to status message
                sprintf_s(tempBuffer, " %s found %s.", GetUnitName(pointerToUnitData->type), GetWeaponName(weaponType));
                strcat_s(statusMessage, tempBuffer);

                break;
            }

                // Heart
            case CellSymbol_Heart:
            {
                canMoveToCell = true;
                unitsData[heroIndex].health += heartHeal;
                break;
            }

                // Exit cell
            case CellSymbol_Exit:
            {
                isGameActive = false;
                break;
            }
        }
    }

    if (canMoveToCell)
    {
        // Remove unit symbol from previous position
        levelData[pointerToUnitData->row][pointerToUnitData->column] = CellSymbol_Empty;

        // Set new hero position
        pointerToUnitData->row = row;
        pointerToUnitData->column = column;

        // Set hero symbol to new position
        levelData[pointerToUnitData->row][pointerToUnitData->column] = unitSymbol;


        // Reveal fog of war
        if (pointerToUnitData->type == UnitType_Hero)
        {
            RevealFogOfWar(row, column);
        }
    }
}

void UpdateAI()
{
    // Pass all units
    for (int u = 0; u < unitsCount; u++)
    {
        // Ignore hero
        if (u == heroIndex)
            continue;

        // Ignore dead units
        if (unitsData[u].health <= 0)
            continue;

        // Distance to hero
        int distanceToHeroR = abs(unitsData[heroIndex].row - unitsData[u].row);
        int distanceToHeroC = abs(unitsData[heroIndex].column - unitsData[u].column);

        // If hero nearby
        if ((distanceToHeroR + distanceToHeroC) == 1)
        {
            // Attack hero
            MoveUnitTo(&unitsData[u], unitsData[heroIndex].row, unitsData[heroIndex].column);
        }
        else
        {
            // Random move
            switch (rand() % 4)
            {
                case 0:
                    MoveUnitTo(&unitsData[u], unitsData[u].row - 1, unitsData[u].column);
                    break;

                case 1:
                    MoveUnitTo(&unitsData[u], unitsData[u].row + 1, unitsData[u].column);
                    break;

                case 2:
                    MoveUnitTo(&unitsData[u], unitsData[u].row, unitsData[u].column - 1);
                    break;

                case 3:
                    MoveUnitTo(&unitsData[u], unitsData[u].row, unitsData[u].column + 1);
                    break;
            }

        }
    }
}

void Update()
{
    unsigned char inputChar = _getch();
    inputChar = tolower(inputChar);

    switch (inputChar)
    {
        // Up
        case 'w':
            MoveUnitTo(&unitsData[heroIndex], unitsData[heroIndex].row - 1, unitsData[heroIndex].column);
            break;

            // Down
        case 's':
            MoveUnitTo(&unitsData[heroIndex], unitsData[heroIndex].row + 1, unitsData[heroIndex].column);
            break;

            // Left
        case 'a':
            MoveUnitTo(&unitsData[heroIndex], unitsData[heroIndex].row, unitsData[heroIndex].column - 1);
            break;

            // Right
        case 'd':
            MoveUnitTo(&unitsData[heroIndex], unitsData[heroIndex].row, unitsData[heroIndex].column + 1);
            break;

            // Restart level
        case 'r':
            Initialize();
            break;
    }


    // AI turn
    UpdateAI();


    // Hero death
    if (unitsData[heroIndex].health <= 0)
    {
        isGameActive = false;
    }
    else
    {
        // Health regeneration
        if (unitsData[heroIndex].health < GetUnitDefaultHealth(UnitType_Hero))
        {
            //unitsData[heroIndex].health++;
        }
    }

}

void Shutdown()
{
    system("cls");
    printf("\n\tGame over...");
    _getch();
}
#endif //DOOM_SERVER_HANDLER_H
