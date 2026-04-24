#ifndef OS_DUNGEON_ENEMY_H
#define OS_DUNGEON_ENEMY_H

typedef struct {
  float points[40];
  int pointCount;
} HPlotList;

typedef struct {
  HPlotList hplotLists[20];
  int hplotListCount;
} DungeonEnemyHplotData;

extern DungeonEnemyHplotData dungeonEnemyHplotPoints[25];
extern int dungeonEnemyHplotPointsCount;

#endif