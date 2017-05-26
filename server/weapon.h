//
// Created by egorov on 30.04.17.
//

#ifndef DOOM_SERVER_WEAPONDATA_H
#define DOOM_SERVER_WEAPONDATA_H

#include <stdbool.h>
#include "level.h"

typedef enum WeaponType
{
    WeaponType_None,
    WeaponType_Fist,
    WeaponType_Bomb,
} WeaponType;


const char* WeaponName_None;
const char* WeaponName_Fist;
const char* WeaponName_Bomb;

const char* GetWeaponName(WeaponType weaponType);

int GetWeaponDamage(WeaponType weaponType);

int GetWeaponRangeOfCharge(WeaponType weaponType);

WeaponType GetWeaponTypeFromCell(unsigned char cellSymbol);

#endif //DOOM_SERVER_WEAPONDATA_H
