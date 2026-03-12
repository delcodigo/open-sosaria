#ifndef OS_PLAYER_H
#define OS_PLAYER_H

typedef struct {
  int strength;
  int agility;
  int stamina;
  int charisma;
  int wisdom;
  int intelligence;
  int race;
  int type;
  char name[16];
  int gold;
  int food;
  int health;
  int tx;
  int ty;
} Player;

extern Player player;

#endif