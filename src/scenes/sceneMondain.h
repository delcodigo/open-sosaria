#ifndef OS_SCENE_MONDAIN_H
#define OS_SCENE_MONDAIN_H

#include "engine/scene.h"

extern Scene sceneMondain;
extern int mondainMap[19][11];

bool sceneMondain_isValidPosition(int x, int y);
bool sceneMondain_isGemActive();
void sceneMondain_destroyGem();
float sceneMondain_getDistanceToGem();
void sceneMondain_checkForGemTransform();

#endif