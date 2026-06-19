#ifndef OS_PLAYER_MODAIN_H
#define OS_PLAYER_MODAIN_H

#include <stdbool.h>

void playerMondain_init();
void playerMondain_render(float *viewMatrix);
bool playerMondain_update(float deltaTime);
void playerMondain_free();

#endif