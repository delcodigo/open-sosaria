#ifndef OS_PLAYER_TOWN_H
#define OS_PLAYER_TOWN_H

#include <stdbool.h>

void playerTown_init();
bool playerTown_update(float deltaTime);
void playerTown_render(float *viewMatrix);
void playerTown_free();

#endif