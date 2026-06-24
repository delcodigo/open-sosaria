#ifndef OS_MONDAIN_H
#define OS_MONDAIN_H

#include "engine/geometry.h"
#include "maths/vector2.h"

typedef enum {
  MONDAIN_STATE_IDLE,
  MONDAIN_STATE_ACTIVE,
  MONDAIN_STATE_DEFEATED,
  MONDAIN_STATE_TRANSFORMED
} MONDAIN_STATE;

typedef struct {
  Vector2 position;
  Geometry geometry;
  int hp;
  MONDAIN_STATE state;
  float transformMatrix[16];
} Mondain;

extern Mondain mondain;

void mondain_init();
void mondain_update();
void mondain_render(float *viewMatrix);
void mondain_free();

#endif