#ifndef OS_SPACE_3D_H
#define OS_SPACE_3D_H

#include <stdbool.h>

typedef enum {
  HYPER_JUMP_STATE_OFF,
  HYPER_JUMP_STATE_ACCELERATE,
  HYPER_JUMP_STATE_KEEP,
  HYPER_JUMP_STATE_STOP
} HYPER_JUMP_STATE;

extern float starsSpeedModifier;
extern HYPER_JUMP_STATE hyperJumpingState;

void space3D_init();
void space3D_drawPlayerAttack();
bool space3D_isEnemyInSights();
void space3D_killEnemy();
void space3D_render(float *viewMatrix);
void space3D_update(float delta);
void space3D_free();

#endif