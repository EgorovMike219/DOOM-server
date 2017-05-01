//
// Created by egorov on 30.04.17.
//

#include <stdlib.h>
#include "unit.h"
#include "weapon.h"

const char* UnitName_None = "None";
const char* UnitName_Hero = "Hero";

const char* GetUnitName(UnitType unitType)
{
    switch (unitType)
    {
        case UnitType_Hero:
            return UnitName_Hero;
    }

    return UnitName_None;
}

WeaponType GetUnitDefaultWeapon(UnitType unitType)
{
    switch (unitType)
    {
        case UnitType_Hero:
            return WeaponType_Bomb;
    }

    return WeaponType_None;
}

int GetUnitDefaultHealth(UnitType unitType)
{
    switch (unitType)
    {
        case UnitType_None:
            return 0;
        case UnitType_Hero:
            return 400;
    }
}

UnitType GetUnitTypeFromCell(unsigned char cellSymbol)
{
    switch (cellSymbol)
    {
        case CellSymbol_Hero:
            return UnitType_Hero;
    }

    return UnitType_None;
}

void MoveUnitTo(UnitData* pointerToUnitData, float newX, float newY)
{
    // Ignore dead units
    if (pointerToUnitData->health <= 0)
    {
        return;
    }

    int row = (int)(newY);
    int column = (int)(newX);
    int oldRow = (int)(pointerToUnitData->y);
    int oldColumn = (int)(pointerToUnitData->x);

    unsigned char unitSymbol = levelData[oldRow][oldColumn];
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
        case CellSymbol_Hero: {
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

                if (unitsData[u].row == row && unitsData[u].column == column)
                {
                    // Calculate weapon damage
                    int damage = GetWeaponDamage(WeaponName_Fist);

                    // Deal damage
                    unitsData[u].health -= damage;

                    /*
                    // Add to status message
                    sprintf_s(tempBuffer, " %s dealt %i damage to %s.", GetUnitName(pointerToUnitData->type), damage, GetUnitName(destinationUnitType));
                    strcat_s(statusMessage, tempBuffer);
                    */

                    // If enemy unit die
                    if (unitsData[u].health <= 0)
                    {
                        levelData[row][column] = CellSymbol_Empty;

                        /*
                        // Add to status message
                        sprintf_s(tempBuffer, " %s died.", GetUnitName(destinationUnitType), damage, GetUnitName(destinationUnitType));
                        strcat_s(statusMessage, tempBuffer);
                        */
                    }

                    break;
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
            /*
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
            */

            // Heart
            case CellSymbol_Heart: {
                canMoveToCell = true;
                unitsData[heroIndex].health += heartHeal;
                break;
            }
            // Poison
            case CellSymbol_Poison:
            {
                canMoveToCell = true;
                unitsData[heroIndex].health -= poisoningEffect;
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
    }
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

        if (unitsData[u].row <= pointerToUnitData->row + range_of_damage &&
            unitsData[u].row >= pointerToUnitData->row - range_of_damage &&
            unitsData[u].column <= pointerToUnitData->column + range_of_damage &&
            unitsData[u].row >= pointerToUnitData->row - range_of_damage) {
            unitsData[u] -= (int)(damage * range_of_damage / (range_of_damage + 1)); // пока без препятствий
        }
    }

}