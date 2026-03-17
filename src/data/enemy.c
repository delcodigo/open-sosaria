#include <stdlib.h>
#include <string.h>
#include "enemy.h"
#include "memory.h"

EnemyDefinition *enemyDefinitions = NULL;

void enemy_define(int index, const char *name, float group, float rank, float hp) {
  if (enemyDefinitions == NULL) {
    return;
  }

  EnemyDefinition *def = &enemyDefinitions[index];
  strncpy(def->name, name, sizeof(def->name) - 1);
  def->name[sizeof(def->name) - 1] = '\0';
  def->group = group;
  def->rank = rank;
  def->hp = hp;
}

void enemy_freeDefinitions() {
  free(enemyDefinitions);
  enemyDefinitions = NULL;
}