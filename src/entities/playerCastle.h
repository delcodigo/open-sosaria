#ifndef OS_PLAYER_CASTLE_H
#define OS_PLAYER_CASTLE_H

#include <stdbool.h>

void playerCastle_init();
bool playerCastle_update(float deltaTime);
void playerCastle_render(float *viewMatrix);
void playerCastle_free();

#endif