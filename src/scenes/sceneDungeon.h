#ifndef OS_SCENE_DUNGEON_H
#define OS_SCENE_DUNGEON_H

#include <stdbool.h>
#include "engine/scene.h"
#include "config.h"

extern Scene sceneDungeon;
extern int dungeonMap[OS_DUNGEON_MAP_WIDTH][OS_DUNGEON_MAP_HEIGHT];
extern int monstersIndex;

void sceneDungeon_generateFloor();
bool sceneDungeon_isSolid(int x, int y);

#endif