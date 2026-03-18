#ifndef OS_PLAYER_OVERWORLD_H
#define OS_PLAYER_OVERWORLD_H

#include <stdbool.h>
#include "data/player.h"

typedef enum {
  PLAYER_STATE_IDLE,
  PLAYER_STATE_READY_TYPE
} PLAYER_STATE;

void playerOverworld_init();
bool playerOverworld_update(float deltaTime);
void playerOverworld_free();

#endif