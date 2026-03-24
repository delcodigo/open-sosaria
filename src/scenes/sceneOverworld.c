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
#include "entities/vehicleOverworld.h"
#include "engine/input.h"
#include "utils.h"

static bool playerActed = false;
static bool respawnPlayer = false;
static int respawnX = 0;
static int respawnY = 0;

EnemyEncounter enemyEncounter = { -1, 0, 0};
float lagTime = 0.0f;

static void sceneOverworld_init() {
  worldMap_init();
  playerOverworld_init();
  uiConsole_init();
  uiConsole_addMessage(ultimaStrings[98]);
  vehicleOverworld_init();
  playerActed = false;

  // TODO: Test vehicles, delete this after testing
  vehiclesMap[38][38] = 1;
  vehiclesMap[38][40] = 2;
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

  enemyEncounter.number = (int)(pow(rand01(), 2) * enemyDefinitions[enemyEncounter.monsterId].group) + 1;
}

static void sceneOverworld_resolveEncounter() {
  if (enemyEncounter.monsterId < 6 || enemyEncounter.monsterId > 20) {
    sceneOverworld_spawnMonsters();
    return;
  }

  char consoleMessage[31] = {0};
  snprintf(consoleMessage, sizeof(consoleMessage), "^1%.26s^0", ultimaStrings[252]);
  uiConsole_queueMessage(consoleMessage);

  char enemiesMsg[31] = {0};
  snprintf(enemiesMsg, sizeof(enemiesMsg), "^1%.8s %d %s%s^0", ultimaStrings[253], enemyEncounter.number, enemyDefinitions[enemyEncounter.monsterId].name, enemyEncounter.number > 1 ? "s" : ""  );
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

  snprintf(enemiesMsg, sizeof(enemiesMsg), "^1%.8s%d%.10s%d^0", ultimaStrings[256], hits, ultimaStrings[257], enemyEncounter.number - hits);
  uiConsole_queueMessage(enemiesMsg);

  uiConsole_updateStats();

  if (hits > 0) {
    // TODO: Play attack sound
  }
}

static bool sceneOverworld_attemptResurrection() {
  if (!player_isAlive()) {
    char consoleMessage[31] = {0};
    snprintf(consoleMessage, sizeof(consoleMessage), "%.15s%.14s", player.name, ultimaStrings[259]);
    uiConsole_addMessage(consoleMessage);
    uiConsole_addMessage(ultimaStrings[260]);

    for (int i=0;i<OS_WEAPONS_COUNT;i++) {
      player.weapons[i] = 0;
    }

    player.health = 99;
    player.weapon = 0;
    player.vehicle = 0;
    player.gold = 0;
    player.food = 20;
    
    int tx = (int)(rand01() * 64);
    int ty = (int)(rand01() * 64);

    // The original game doesn't check if the player is placed in a valid tile after resurrection, but we will to avoid softlocks.
    int tile = (worldMap_getTileAt(tx, ty) >> 4) & 0xFF;
    while (tile == 0 || tile >= 3) {
      tx = (int)(rand01() * 64);
      ty = (int)(rand01() * 64);
      tile = (worldMap_getTileAt(tx, ty) >> 4) & 0xFF;
    }

    respawnX = tx;
    respawnY = ty;

    enemyEncounter.monsterId = -1;
    playerActed = false;
    respawnPlayer = true;

    lagTime = 5.0f;

    return true;
  }

  return false;
}

static void sceneOverworld_update(float deltaTime) {
  if (lagTime > 0) {
    lagTime -= deltaTime;
    if (lagTime < 0) { lagTime = 0; }
  }
  
  if (!queuedMessagesCount && lagTime <= 0 && !sceneOverworld_attemptResurrection()) {
    if (respawnPlayer) {
      player.tx = respawnX;
      player.ty = respawnY;
      respawnPlayer = false;
      uiConsole_updateStats();
    }

    if (ztatsActive){
      uiZtats_update(deltaTime);
      return;
    }

    if (playerActed) {
      playerActed = false;
      sceneOverworld_resolveEncounter();
      if (player_isAlive()) {
        uiConsole_addMessage(ultimaStrings[98]);
      }
    }

    if (player_isAlive() && playerOverworld_update(deltaTime)) { 
      playerActed = true; 
    }
  }
  
  float *viewMatrix = camera_getViewProjectionMatrix(&camera);
  worldMap_update(viewMatrix);
  vehicleOverworld_render(viewMatrix);
  playerOverworld_render();
  uiConsole_update(deltaTime);
}

static void sceneOverworld_free() {
  worldMap_free();
  playerOverworld_free();
  uiConsole_free();
  uiZtats_free();
  vehicleOverworld_free();
}

Scene sceneOverworld = {
  .scene_init = sceneOverworld_init,
  .scene_update = sceneOverworld_update,
  .scene_free = sceneOverworld_free
};