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

typedef struct WeaponData {
    WeaponType type;  // тип оружия
    int damage;
    int range_of_charge; // радиус действия заряда
} WeaponData;

#endif //DOOM_SERVER_WEAPONDATA_H
