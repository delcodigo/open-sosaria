#include "player.h"
#include "entities/ui/uiConsole.h"

Player player = {0};

void player_consumeFood() {
  player.food -= (7.0f - player.transport) / 14.0f;
  player.time += (7.0f - player.transport) / 7.0f;
  uiConsole_updateStats();
}

void player_waitPenalty() {
  player.food -= 0.05f;
  player.time += 0.5f;
  uiConsole_updateStats();
}