#include "player.h"
#include "entities/ui/uiConsole.h"

Player player = {0};

bool playerActed = false;
bool respawnPlayer = false;
bool isPlayerInCastle = false;
int respawnX = 0;
int respawnY = 0;
float keyRepeatDelay = 0;
float waitingTime = 0.0f;
float lagTime = 0.0f;

void player_consumeFood() {
  player.food -= (7.0f - player.vehicle) / 14.0f;
  player.time += (7.0f - player.vehicle) / 7.0f;
  uiConsole_updateStats();
}

void player_consumeTownFood() {
  player.food -= 0.01f;
  player.time += 0.1f;
  uiConsole_updateStats();
}

void player_waitPenalty() {
  player.food -= 0.05f;
  player.time += 0.5f;
  uiConsole_updateStats();
}

bool player_isAlive() {
  return player.health > 0 && player.food > 0;
}

int player_getEncumbrance() {
  int encumbrance = 0;
  
  for (int i=0;i<OS_WEAPONS_COUNT;i++) {
    encumbrance += player.weapons[i];
  }

  for (int i=0;i<OS_ARMORS_COUNT;i++) {
    encumbrance += i * player.armors[i];
  }

  encumbrance += player.gold / 100;
  
  encumbrance -= player.vehicles[2] * 20;
  encumbrance -= player.strength * 4;

  return encumbrance;
}