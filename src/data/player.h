#ifndef OS_PLAYER_H
#define OS_PLAYER_H

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
  int armors[5];
  int vehicles[6];
  int weapons[15];
  int spells[10];
  int gems[4];
} Player;

extern Player player;

void player_consumeFood();
void player_waitPenalty();

#endif