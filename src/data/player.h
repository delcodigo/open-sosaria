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
} Player;

extern Player player;

void player_consumeFood();
void player_waitPenalty();
bool player_isAlive();

#endif