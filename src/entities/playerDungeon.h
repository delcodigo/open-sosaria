#ifndef OS_PLAYER_DUNGEON_H
#define OS_PLAYER_DUNGEON_H

#include <stdbool.h>

void playerDungeon_init();
bool playerDungeon_update(float deltaTime);
void playerDungeon_free();

#endif