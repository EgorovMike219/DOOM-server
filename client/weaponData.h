//
// Created by egorov on 30.04.17.
//

#ifndef DOOM_SERVER_WEAPONDATA_H
#define DOOM_SERVER_WEAPONDATA_H

#include "weaponType.h"
#include <stdbool.h>


typedef struct WeaponData {
    WeaponType type;  // тип оружия
    float cool_down; // перезарядка
    int damage;
    bool damage_depends_on_distance;  // зависит ли урон от расстояния
} WeaponData;

#endif //DOOM_SERVER_WEAPONDATA_H
