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
  int transport;
} Player;

extern Player player;

void player_consumeFood();
void player_waitPenalty();

#endif