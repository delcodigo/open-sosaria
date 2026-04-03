#ifndef OS_SCENE_TOWN_H
#define OS_SCENE_TOWN_H

#include "maths/vector2.h"
#include "engine/engine.h"
#include "engine/scene.h"

typedef enum {
  TOWN_ENTITY_TYPE_NONE = 0,
  TOWN_ENTITY_TYPE_MERCHANT = 1,
  TOWN_ENTITY_TYPE_WENCH = 2,
  TOWN_ENTITY_TYPE_BARD = 3,
  TOWN_ENTITY_TYPE_GUARD = 4
} TOWN_ENTITY_TYPE;

extern Scene sceneTown;
extern Vector2 wenchPosition;

bool sceneTown_isSolid(int x, int y);
void sceneTown_attackAt(int x, int y);

#endif