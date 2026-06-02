#ifndef OS_PLAYER_SPACE_H
#define OS_PLAYER_SPACE_H

#include <stdbool.h>
#include "maths/vector2.h"

extern Vector2f targetCentre;

void playerSpace_init();
void playerSpace_render(float *viewMatrix);
void playerSpace_performHyperJump();
bool playerSpace_update(float deltaTime);
void playerSpace_free();

#endif