//
// Created by egorov on 30.04.17.
//

#ifndef UNIT_H
#define UNIT_H

#include "level.h"
#include "weapon.h"

typedef enum UnitType
{
    UnitType_None,
    UnitType_Hero,
} UnitType;

typedef enum UnitOrder
{
    UnitOrder_Forward		= 1,
    UnitOrder_None			= 0,
    UnitOrder_Backward		= -1
} UnitOrder;

const char* UnitName_None;
const char* UnitName_Hero;

typedef struct UnitData
{
    int id;
    UnitType type;
    float x;
    float y;
    int health;
    WeaponType weapon;
    int count_of_charge;
    // количество проходимых клеток за секунду
    // если скорость положительная то двигаемя направо,
    // а если отрицательная то налево
    float speed_x;
    float speed_y;
    UnitOrder order_y;
    UnitOrder order_x;
    int last_command;
} UnitData;


const char* GetUnitName(UnitType unitType);

WeaponType GetUnitDefaultWeapon(UnitType unitType);

int GetUnitDefaultHealth(UnitType unitType);

float GetUnitSpeed(UnitType unitType);

UnitType GetUnitTypeFromCell(unsigned char cellSymbol);

int GetUnitDefaultCountOfCharge();

#endif //UNIT_H
