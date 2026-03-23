#ifndef OS_WORLDMAP_H
#define OS_WORLDMAP_H

#include "engine/engine.h"
#include "engine/geometry.h"

void worldMap_init();
int worldMap_getTileAt(int tx, int ty);
int worldMap_getPlayerTile();
void worldMap_update(float *viewMatrix);
void worldMap_free();

#endif