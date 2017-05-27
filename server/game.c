#include "game.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
//#include "statistics/statistics.h"


const int maxUnitsCount = 1;

const float cellBeginValue = 0.001f;
const float cellEndValue = 0.999f;
extern int d;

/////////////////////////////////////
// Logics variables
bool isGameActive = false;
clock_t clockLastFrame = 0;

int framesCounter = 0;
float framesTimeCounter = 0;
int fps = 0;

int unitsCount = 0;
int heroIndex = 0;

bool InsertUnitOnMap(int *r, int *c) {
    int i, j, r1, c1;
    for (i = -1; i <= 1; ++i){
        for (j = -1; j <= 1; ++j) {
            r1 = *r + i;
            c1 = *c + j;
            if ( ( (r1 >= 0) && (r1 < rowsCount))  && ( (c1 >= 0) && (c1 < columnsCount)) ) {
                if (levelData[r1][c1] == CellSymbol_Empty) {
                    levelData[r1][c1] = CellSymbol_Hero;
                    *r = r1;
                    *c = c1;
                    return true;
                }
            }
        }
    }
    return false;
}

void Initialize(char* pathname) {
    //read levelData0 and others
    read_from_file(pathname);

    // Set clockLastFrame start value
    clockLastFrame = clock();

    unitsData = (UnitData*)malloc(unitsCount * sizeof(UnitData));
    levelData = (char**)malloc(rowsCount * sizeof(char*));
    for (int i = 0; i < rowsCount; ++i) {
        levelData[i] = (char *) malloc(columnsCount * sizeof(char));
    }
    //unitsCount = 0; определяем в main-е

    int r, c;
    // Load level
    for (r = 0; r < rowsCount; r++ )
    {
        for (c = 0; c < columnsCount; c++ )
        {
            unsigned char cellSymbol = levelData0[r][c];

            levelData[r][c] = cellSymbol;
        }
    }
    int i, row, column;

    liveUnitsCount = unitsCount;
    UnitType unitType;
    for (i = 0; i < unitsCount; ++i) {
        unitType = UnitType_Hero;
        do {
            row = rand() % rowsCount;
            column = rand() % columnsCount;
        } while (InsertUnitOnMap(&row, &column) == false);
        unitsData[i].type = unitType;
        unitsData[i].id = i;
        unitsData[i].y = (float)(row);
        unitsData[i].x = (float)(column);
        unitsData[i].health = GetUnitDefaultHealth(unitType);
        unitsData[i].speed_x = 0.0;
        unitsData[i].speed_y = 0.0;
        unitsData[i].order_x = UnitOrder_None;
        unitsData[i].order_y = UnitOrder_None;
        unitsData[i].weapon = GetUnitDefaultWeapon(unitType);
        unitsData[i].count_of_charge = GetUnitDefaultCountOfCharge();
    }
    heroIndex = 0;
    isGameActive = true;  // Костя просил убрать
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

    int i, c;
    // Hero control
    for (i = 0; i < unitsCount; ++i) {
        c = unitsData[i].last_command;
        unitsData[i].last_command = -1; // хорошо было бы иметь атомики
        if (c == 'w') {
            unitsData[i].order_y = UnitOrder_Backward;
        }
        if (c == 's') {
            unitsData[i].order_y = UnitOrder_Forward;
        }

        if (c == 'a') {
            unitsData[i].order_x = UnitOrder_Backward;
        }
        if (c == 'd') {
            unitsData[i].order_x = UnitOrder_Forward;
        }
        if (c == 'b') {
            SetBomb(&unitsData[i]);
        }
        if (c == 'q') {
            isGameActive = false;
        }
    }


    // Update all units
    for( int u = 0; u < unitsCount; u++ ) {
        UpdateUnit(&unitsData[u], deltaTime);
        //record_in_statistics(unitsData[u].id, unitsData[u].health);  надо вернуть в master
    }


    // Hero dead
    if( liveUnitsCount <= 1 )
        isGameActive = false;
}

void Shutdown(){
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
                if (unitsData[u].health <= 0) {
                    levelData[row][column] = CellSymbol_Empty;
                    liveUnitsCount--;
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
            pointerToUnitData->health += heartHeal;
        }
        // Poison
        if (destinationCellSymbol == CellSymbol_Poison) {
            canMoveToCell = true;
            pointerToUnitData->health -= poisoningEffect;
            if (pointerToUnitData->health <= 0) {
                liveUnitsCount--;
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
        int max, unit_damage;
        if ((int)(unitsData[u].y) <= row + range_of_damage &&
            (int)(unitsData[u].y) >= row - range_of_damage &&
            (int)(unitsData[u].x) <= column + range_of_damage &&
            (int)(unitsData[u].x) >= column - range_of_damage) {
            max = abs((int)(unitsData[u].y) - row);
            if (abs((int)(unitsData[u].x) - column) > max) {
                max= abs((int)(unitsData[u].x) - column);
            }
            unit_damage = (int)(damage / range_of_damage *(range_of_damage + 1 - max));
            if (unit_damage > 0) {
                unitsData[u].health -= unit_damage; // пока без препятствий
            }
            if (unitsData[u].health <= 0) {
                liveUnitsCount--;
            }
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
