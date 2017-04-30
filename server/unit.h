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
    UnitType_Enemy,
} UnitType;

const char* UnitName_None = "None";
const char* UnitName_Hero = "Hero";
const char* UnitName_Enemy = "Enemy";

typedef struct UnitData
{
    int id;
    UnitType type;
    int row;
    int column;
    int health;
    WeaponType weapon;
    int count_of_charge;
} UnitData;


const char* GetUnitName(UnitType unitType)
{
    switch (unitType)
    {
        case UnitType_Hero:     return UnitName_Hero;
        case UnitType_Enemy:      return UnitName_Enemy;
    }

    return UnitName_None;
}

WeaponType GetUnitDefaultWeapon(UnitType unitType)
{
    switch (unitType)
    {
        case UnitType_Hero:             return WeaponType_Bomb;
        case UnitType_Enemy:             return WeaponType_Bomb;
    }

    return WeaponType_None;
}

int GetUnitDefaultHealth(UnitType unitType)
{
    switch (unitType)
    {
        case UnitType_Hero:         return 4000;
        case UnitType_Enemy:          return 4000;
    }

    return 0;
}

UnitType GetUnitTypeFromCell(unsigned char cellSymbol)
{
    switch (cellSymbol)
    {
        case CellSymbol_Hero:               return UnitType_Hero;
        case CellSymbol_Enemy:                return UnitType_Enemy;
    }

    return UnitType_None;
}

#endif //DOOM_SERVER_UNITTYPE_H
