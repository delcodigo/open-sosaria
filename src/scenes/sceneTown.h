#ifndef OS_SCENE_TOWN_H
#define OS_SCENE_TOWN_H

#include "maths/vector2.h"
#include "engine/engine.h"
#include "engine/scene.h"

extern Scene sceneTown;
extern Vector2 wenchPosition;

bool sceneTown_isSolid(int x, int y);

#endif