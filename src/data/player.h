#ifndef OS_PLAYER_H
#define OS_PLAYER_H

#include "config.h"
#include <stdbool.h>

typedef struct {
  int health;
  int strength;
  int agility;
  int stamina;
  int charisma;
  int wisdom;
  int intelligence;
  int gold;
  int race;
  int type;
  char name[16];
  float food;
  int tx;
  int ty;
  int experience;
  float time;
  int vehicle;
  int armor;
  int weapon;
  int spell;
  int armors[OS_ARMORS_COUNT];
  int vehicles[OS_VEHICLES_COUNT];
  int weapons[OS_WEAPONS_COUNT];
  int spells[OS_SPELLS_COUNT];
  int gems[OS_GEMS_COUNT];
  int quests[OS_QUESTS_COUNT];
  int px;
  int py;
  int eptns;
} Player;

extern Player player;
extern bool playerActed;
extern bool respawnPlayer;
extern int respawnX;
extern int respawnY;
extern float keyRepeatDelay;
extern float waitingTime;
extern float lagTime;

void player_consumeFood();
void player_consumeTownFood();
void player_waitPenalty();
bool player_isAlive();

#endif