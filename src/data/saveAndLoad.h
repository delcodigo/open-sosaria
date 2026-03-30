#ifndef OS_SAVEANDLOAD_H
#define OS_SAVEANDLOAD_H

#include <stdbool.h>
#include "player.h"
#include "config.h"

typedef struct {
  char header[17];
  char version[10];
  Player player;
  int vehiclesMap[OS_BTERRA_MAP_WIDTH * 2][OS_BTERRA_MAP_HEIGHT * 2];
} SaveData;

extern SaveData savedGame;

bool loadGame();
void saveGame();

#endif