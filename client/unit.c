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

float GetUnitSpeed(UnitType unitType) {
    switch (unitType)
    {
        case UnitType_None:
            return 0.0f;
        case UnitType_Hero:
            return 4000.0f;
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
    if (cellSymbol == CellSymbol_Hero) {
        return UnitType_Hero;
    }

    return UnitType_None;
}

int GetUnitDefaultCountOfCharge() {
    return 200;
}