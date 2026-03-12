#ifndef OS_SAVEANDLOAD_H
#define OS_SAVEANDLOAD_H

#include "player.h"

typedef struct {
  char header[17];
  char version[10];
  Player player;
} SaveData;

extern SaveData savedGame;

void saveGame();

#endif