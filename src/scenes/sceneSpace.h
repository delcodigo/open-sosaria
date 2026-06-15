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
extern bool isFirstPersonView;
extern int enemyCrafts;

int sceneSpace_getBase4Digit(int position, int value);
void sceneSpace_initializeShapes();
bool sceneSpace_tryBoardVessel(int shapeId, int rotation);
void sceneSpace_transformShape(float *transformMatrix, float x, float y, float rotation);
void sceneSpace_crunchCollision();
void sceneSpace_checkCollisions();

#endif