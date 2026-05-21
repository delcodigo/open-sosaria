#ifndef OS_SCENE_SPACE_H
#define OS_SCENE_SPACE_H

#include "engine/scene.h"
#include "engine/geometry.h"

typedef struct {
  int x;
  int y;
  int shapeId;
  int rotation;
  Geometry geometry;
} SpaceShape;

extern Geometry playerShipGeometries[3];
extern int spaceMap[11][11];
extern Scene sceneSpace;

void sceneSpace_transformShape(float *transformMatrix, float x, float y, float rotation);

#endif