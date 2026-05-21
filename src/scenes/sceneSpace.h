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

extern Scene sceneSpace;

#endif