#ifndef OS_PLAYER_OVERWORLD_H
#define OS_PLAYER_OVERWORLD_H

#include <stdbool.h>
#include "data/player.h"

void playerOverworld_init();
bool playerOverworld_update(float deltaTime);
void playerOverworld_render();
void playerOverworld_free();
void playerOverworld_updateGeometry();
void playerOverworld_setCameraFollow();

#endif