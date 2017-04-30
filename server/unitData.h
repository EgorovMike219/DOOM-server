//
// Created by egorov on 30.04.17.
//

#ifndef DOOM_SERVER_UNITDATA_H
#define DOOM_SERVER_UNITDATA_H

#include "unitType.h"
#include "weaponType.h"

typedef struct UnitData
{
    UnitType type;
    int row;
    int column;
    int health;
    WeaponType weapon;
} UnitData;

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
        case CellSymbol_Enemy:
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

#endif //DOOM_SERVER_UNITDATA_H
