#ifndef OS_PLAYER_TOWN_H
#define OS_PLAYER_TOWN_H

void playerTown_init();
void playerTown_update(float deltaTime);
void playerTown_render(float *viewMatrix);
void playerTown_free();

#endif