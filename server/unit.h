//
// Created by egorov on 30.04.17.
//

#ifndef DOOM_SERVER_UNITTYPE_H
#define DOOM_SERVER_UNITTYPE_H

#include "level.h"
#include "weapon.h"

typedef enum UnitType
{
    UnitType_None,
    UnitType_Hero,
} UnitType;

const char* UnitName_None;
const char* UnitName_Hero;

typedef struct UnitData
{
    int id;
    UnitType type;
    float x;
    float y;
    int row;
    int column;
    int health;
    WeaponType weapon;
    int count_of_charge;
    int speed; // количество проходимых клеток за секунду
} UnitData;


const char* GetUnitName(UnitType unitType);

WeaponType GetUnitDefaultWeapon(UnitType unitType);

int GetUnitDefaultHealth(UnitType unitType);

UnitType GetUnitTypeFromCell(unsigned char cellSymbol);

#endif //DOOM_SERVER_UNITTYPE_H
