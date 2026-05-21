#ifndef OS_PLAYER_SPACE_H
#define OS_PLAYER_SPACE_H

#include <stdbool.h>

void playerSpace_render(float *viewMatrix);
bool playerSpace_update(float deltaTime);

#endif