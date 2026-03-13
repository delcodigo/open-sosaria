#include "player.h"
#include "entities/ui/uiConsole.h"

Player player = {0};

void player_consumeFood() {
  player.food -= (7.0f - player.transport) / 14.0f;
  uiConsole_updateStats();
}