//
// Created by egorov on 30.04.17.
//

#ifndef DOOM_SERVER_WEAPONTYPE_H
#define DOOM_SERVER_WEAPONTYPE_H

#include "level.h"

enum WeaponType
{
    WeaponType_None,
    WeaponType_Fist,
    WeaponType_Bomb,
};


const char* WeaponName_None = "None";
const char* WeaponName_Fist = "Fist";
const char* WeaponName_Bomb = "Bomb";


const char* GetWeaponName(WeaponType weaponType)
{
    switch (weaponType)
    {
        case WeaponType_Fist:           return WeaponName_Fist;
        case WeaponType_Bomb:          return WeaponName_Bomb;
    }

    return WeaponName_None;
}

int GetWeaponDamage(WeaponType weaponType)  // Изменить логику так как урон зависит от растояния
{
    switch (weaponType)
    {
        case WeaponType_Fist:           return 2;
        case WeaponType_Bomb:          return 16;  // Изменить логику так как урон зависит от растояния
    }

    return 0;
}

WeaponType GetWeaponTypeFromCell(unsigned char cellSymbol)
{
    switch (cellSymbol)
    {
        case CellSymbol_Bomb:          return WeaponType_Bomb;  // по идее это оружие есть всегда и оно активируется на какую-нибудь кнопку
    }

    return WeaponType_None;
}

#endif //DOOM_SERVER_WEAPONTYPE_H
