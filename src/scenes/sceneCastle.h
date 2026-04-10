#ifndef OS_SCENE_CASTLE_H
#define OS_SCENE_CASTLE_H

#include "engine/engine.h"
#include "engine/scene.h"
#include "maths/vector2.h"

extern Scene sceneCastle;
extern Vector2 princessPosition;
extern int castleKey;
extern int allowToTakeItemsFromCastle;
extern int openedCellDoorIndex;

bool sceneCastle_isSolid(int x, int y);
void sceneCastle_attackAt(int x, int y);

#endif