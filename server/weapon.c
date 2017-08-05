//
// Created by egorov on 30.04.17.
//

#include "weapon.h"

const char* WeaponName_None = "None";
const char* WeaponName_Fist = "Fist";
const char* WeaponName_Bomb = "Bomb";

const char* GetWeaponName(WeaponType weaponType) {
    switch (weaponType)
    {
        case WeaponType_Fist:           return WeaponName_Fist;
        case WeaponType_Bomb:          return WeaponName_Bomb;
        case WeaponType_None:          return WeaponName_None;
    }
}

int GetWeaponDamage(WeaponType weaponType) {  // урон наносимый соседней клетке
    switch (weaponType)
    {
        case WeaponType_Fist:           return 2;
        case WeaponType_Bomb:          return damage;  // считываем из файла
        case WeaponType_None:          return 0;
    }
}

int GetWeaponRangeOfCharge(WeaponType weaponType) {
    switch (weaponType)
    {
        case WeaponType_Fist:           return 1;
        case WeaponType_Bomb:          return range_of_damage;  // считываем из файла
        case WeaponType_None:          return 0;
    }
}

WeaponType GetWeaponTypeFromCell(unsigned char cellSymbol)
{
    if (cellSymbol == CELL_BOMB) return WeaponType_Bomb;  // по идее это оружие есть всегда и оно активируется на какую-нибудь кнопку

    return WeaponType_None;
}

