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

int GetUnitSpeed(UnitType unitType) {
    switch (unitType)
    {
        case UnitType_None:
            return 0.0f;
        case UnitType_Hero:
            return 4.0f;
    }
}

int GetUnitDefaultHealth(UnitType unitType) {
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

bool MoveUnitTo(UnitData* pointerToUnitData, float newX, float newY)
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

                if ((int)(unitsData[u].y) == row && (int)(unitsData[u].x) == column)
                {
                    // Calculate weapon damage
                    int damage = GetWeaponDamage(WeaponName_Fist);

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

            break;
        }
    }

    // Only hero actions
    if (pointerToUnitData->type == UnitType_Hero)
    {
        switch (destinationCellSymbol)
        {

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
            unitsData[u] -= (int)(damage * range_of_damage / (range_of_damage + 1)); // пока без препятствий
        }
    }

}

void UpdateUnit( UnitData* pointerToUnitData, float deltaTime )
{
    // Unit row and column
    int row = (int)(pointerToUnitData->y);
    int column = (int_(pointerToUnitData->x);


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
            break;
        }
        if (newRow >= rowsCount) {
            pointerToUnitData->y = row + cellEndValue;
            break;
        }
        if(!MoveUnitTo(pointerToUnitData, pointerToUnitData->x, newY)) {
            // Can not move cell down
            if (newRow > row) {
                pointerToUnitData->y = row + cellEndValue;
            } else {
                pointerToUnitData->y = row + cellBeginValue;
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
            break;
        }
        if (newColumn >= columnsCount) {
            pointerToUnitData->x = column + cellEndValue;
            break;
        }
        if (!MoveUnitTo(pointerToUnitData, newX, pointerToUnitData->y)) {
            // Can not move cell right
            if (newColumn > column) {
                pointerToUnitData->x = column + cellEndValue;
            } else {
                pointerToUnitData->x = column + cellBeginValue;
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