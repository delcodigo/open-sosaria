#ifndef OS_SCENE_MONDAIN_H
#define OS_SCENE_MONDAIN_H

#include "engine/scene.h"

extern Scene sceneMondain;
extern int mondainMap[19][11];

bool sceneMondain_isValidPosition(int x, int y);
bool sceneMondain_isGemActive();
void sceneMondain_destroyGem();
void sceneMondain_setWall(int x, int y, int wallIndex);
float sceneMondain_getDistanceToGem();
void sceneMondain_checkForGemTransform();

#endif