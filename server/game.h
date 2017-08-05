//
// Created by egorov on 15.05.17.
//

#ifndef GAME_H
#define GAME_H

#include "unit.h"
#include <time.h>


extern const int maxUnitsCount;
extern const float cellEndValue;
extern const float cellBeginValue;

bool isGameActive;
clock_t clockLastFrame;

int framesCounter;
float framesTimeCounter;
int fps;

char **levelData;

UnitData *unitsData;
int unitsCount;
int liveUnitsCount;
int heroIndex;

bool MoveUnitTo(UnitData* pointerToUnitData, float newX, float newY);

void SetBomb(UnitData* pointerToUnitData);

void UpdateUnit(UnitData* pointerToUnitData, float deltaTime);

void Initialize(char* pathname);
void Update();
void Shutdown();


#endif //GAME_H
