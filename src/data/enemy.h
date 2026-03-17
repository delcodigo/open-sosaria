#ifndef OS_ENEMY_H
#define OS_ENEMY_H

typedef struct {
  char name[16];
  float group;  // Overworld: size of the group, Dungeon: alive or free slot
  float rank;   // Combat strength scaling
  float hp;     // Overworld: unused, Dungeon: current HP
} EnemyDefinition;

extern EnemyDefinition *enemyDefinitions;

void enemy_define(int index, const char *name, float group, float rank, float hp);
void enemy_freeDefinitions();

#endif