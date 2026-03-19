#include <math.h>
#include <stdio.h>
#include <string.h>
#include "engine/geometry.h"
#include "engine/camera.h"
#include "data/enemy.h"
#include "maths/matrix4.h"
#include "sceneOverworld.h"
#include "sceneDiskLoader.h"
#include "entities/worldMap.h"
#include "entities/playerOverworld.h"
#include "entities/ui/uiConsole.h"
#include "entities/ui/uiztats.h"
#include "engine/input.h"
#include "utils.h"

static bool playerActed = false;

EnemyEncounter enemyEncounter = { -1, 0, 0};

static void sceneOverworld_init() {
  worldMap_init();
  playerOverworld_init();
  uiConsole_init();
  uiConsole_addMessage(ultimaStrings[98]);
  playerActed = false;
}

static void sceneOverworld_spawnMonsters() {
  int tile = (worldMap_getPlayerTile() >> 4) & 0x0F;
  if (tile > 2) { tile = 1; }

  if (rand01() > 0.1f || (tile == 1 && rand01() > 0.5f)) {
    return;
  }

  if (tile == 0) {
    enemyEncounter.monsterId = (int)(rand01() * 4 + 6);
    enemyEncounter.hp = (enemyEncounter.monsterId - 4) * 20 * rand01() + 30 + pow(rand01(), 2) * (int)(player.time / 100);
  } else if (tile == 1) {
    enemyEncounter.monsterId = (int)(rand01() * 6 + 15);
    enemyEncounter.hp = (enemyEncounter.monsterId - 14) * 10 * rand01() + 10 + pow(rand01(), 2) * (int)(player.time / 100);
  } else if (tile == 2) {
    enemyEncounter.monsterId = (int)(rand01() * 5 + 10);
    enemyEncounter.hp = (enemyEncounter.monsterId - 9) * 20 * rand01() + 30 + pow(rand01(), 2) * (int)(player.time / 100);
  }

  enemyEncounter.number = (int)pow(rand01(), 2) * enemyDefinitions[enemyEncounter.monsterId].group + 1;
}

static void sceneOverworld_resolveEncounter() {
  if (enemyEncounter.monsterId < 6 || enemyEncounter.monsterId > 20) {
    sceneOverworld_spawnMonsters();
    return;
  }

  uiConsole_inverseText();
  uiConsole_queueMessage(ultimaStrings[252]);

  char enemiesMsg[31] = {0};
  snprintf(enemiesMsg, sizeof(enemiesMsg), "%.10s %d %s%s", ultimaStrings[253], enemyEncounter.number, enemyDefinitions[enemyEncounter.monsterId].name, enemyEncounter.number > 1 ? "s" : ""  );
  uiConsole_queueMessage(enemiesMsg);

  int hits = 0;
  for (int i=1;i<=enemyEncounter.number;i++) {
    int defense = player.armor * 2;
    if (defense > 6) { defense = 5; }

    if (player.vehicle > 3 && player.vehicle < 7) {
      defense += 5;
    }

    defense = (int)(player.armor * defense / 10 + player.armor);
    int attackRoll = (int)pow(rand01() * 15, 2);

    if (attackRoll > 80 || attackRoll > defense) {
      player.health -= (int)(enemyDefinitions[enemyEncounter.monsterId].rank * rand01() + 1);
      if (player.health < 0) player.health = 0;

      hits++;
    }
  }

  memset(enemiesMsg, 0, sizeof(enemiesMsg));
  snprintf(enemiesMsg, sizeof(enemiesMsg), "%.10s%d%.10s%d", ultimaStrings[256], hits, ultimaStrings[257], enemyEncounter.number - hits);
  uiConsole_queueMessage(enemiesMsg);

  uiConsole_normalText();
  uiConsole_updateStats();

  if (hits > 0) {
    // TODO: Play attack sound
  }
}

static void sceneOverworld_update(float deltaTime) {
  if (!queuedMessagesCount){
    if (ztatsActive){
      uiZtats_update(deltaTime);
      return;
    }

    if (playerActed) {
      playerActed = false;
      sceneOverworld_resolveEncounter();
      uiConsole_addMessage(ultimaStrings[98]);
    }

    if (playerOverworld_update(deltaTime)) { 
      playerActed = true; 
    }
  }
  
  float *viewMatrix = camera_getViewProjectionMatrix(&camera);
  worldMap_update(viewMatrix);
  playerOverworld_render();
  uiConsole_update(deltaTime);
}

static void sceneOverworld_free() {
  worldMap_free();
  playerOverworld_free();
  uiConsole_free();
  uiZtats_free();
}

Scene sceneOverworld = {
  .scene_init = sceneOverworld_init,
  .scene_update = sceneOverworld_update,
  .scene_free = sceneOverworld_free
};