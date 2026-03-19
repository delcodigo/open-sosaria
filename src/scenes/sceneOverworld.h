#ifndef OS_SCENEOVERWORLD_H
#define OS_SCENEOVERWORLD_H

#include "engine/scene.h"

typedef struct {
  int monsterId;
  int hp;
  int number;
} EnemyEncounter;

extern Scene sceneOverworld;

#endif