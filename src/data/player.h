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
} Player;

extern Player player;

#endif