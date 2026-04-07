#include "sceneCastle.h"
#include "sceneDiskLoader.h"
#include "sceneTown.h"
#include "data/player.h"
#include "engine/geometry.h"
#include "engine/camera.h"
#include "entities/guardTown.h"
#include "entities/ui/uiConsole.h"
#include "entities/vmExecuter.h"
#include "entities/playerCastle.h"
#include "maths/matrix4.h"
#include "maths/vector2.h"
#include "config.h"
#include "utils.h"

static Geometry backgroundGeometry;
static Geometry jesterGeometry;
static Geometry princessGeometry;
static Geometry kingGeometry;
static Vector2 jesterPosition = { 0 };
static Vector2 princessPosition = { 0 };
static Vector2 kingPosition = { 0 };
static float backgroundTransformationMatrix[16];
static float personTransform[16];

static void sceneCastle_initialiseEntities() {
  jesterPosition.x = 37; jesterPosition.y = 6;
  princessPosition.x = 37; princessPosition.y = 14;
  kingPosition.x = 35; kingPosition.y = 4;
}

static void sceneCastle_init() {
  geometry_setSprite(&backgroundGeometry, OS_SCREEN_WIDTH, OS_SCREEN_HEIGHT, 0, 0, 1, 1);
  matrix4_setIdentity(backgroundTransformationMatrix);

  camera_setPosition3f(&camera, 0.0f, 0.0f, 10.0f);

  sceneTown_initializeGeometry(2, &jesterGeometry);
  sceneTown_initializeGeometry(5, &princessGeometry);
  sceneTown_initializeGeometry(0, &kingGeometry);

  matrix4_setIdentity(personTransform);

  isPlayerInCastle = true;

  player.px = 1;
  player.py = 11;
  playerCastle_init();
  guardCastle_init();
  sceneCastle_initialiseEntities();
}

bool sceneCastle_isSolid(int x, int y) {
  if (x < 0 || x >= OS_TOWN_CASTLE_SIZE_WIDTH || y < 0 || y >= OS_TOWN_CASTLE_SIZE_HEIGHT) {
    return false;
  }

  for (int i=0; i<OS_GUARD_TOWN_COUNT; i++) {
    if (guardTowns[i].x == x && guardTowns[i].y == y) {
      return true;
    }
  }

  if (jesterPosition.x == x && jesterPosition.y == y) {
    return true;
  }

  if (princessPosition.x == x && princessPosition.y == y) {
    return true;
  }

  if (kingPosition.x == x && kingPosition.y == y) {
    return true;
  }

  return ultimaAssets.castleCollisionMap[y][x] != 0;
}

static void sceneCastle_updateJester() {
  if (jesterPosition.x < 0) { return; }

  int dx = (int)(rand01() * 3) - 1;
  int dy = (int)(rand01() * 3) - 1;

  if (dx == 0 && dy == 0) { return; }
  if (jesterPosition.x + dx < 0) { return; }

  if (sceneCastle_isSolid(jesterPosition.x + dx, jesterPosition.y + dy)) { 
    if (rand01() > 0.8f){
      uiConsole_addMessage(ultimaStrings[821]);
      uiConsole_addMessage(ultimaStrings[822]);
    }

    return;
  }

  if (jesterPosition.x + dx != player.px || jesterPosition.y + dy != player.py) {
    jesterPosition.x += dx;
    jesterPosition.y += dy;
    return;
  }

  for (int i=0;i<OS_WEAPONS_COUNT;i++) {
    if (player.weapons[i] > 0 && player.weapon-1 != i) {
      player.weapons[i] -= 1;
    }
  }

  if (rand01() * 50 < player.wisdom) {
    uiConsole_addMessage(ultimaStrings[823]);
  }
}

static void sceneCastle_update(float deltaTime) {
  if (lagTime > 0) {
    lagTime -= deltaTime;
    if (lagTime < 0) { lagTime = 0; }
  }

  if (!queuedMessagesCount && lagTime <= 0 && !vmExecuter_update(deltaTime)) {
    if (playerActed) {
      playerActed = false;

      sceneCastle_updateJester();

      for (int i=0; i<OS_GUARD_TOWN_COUNT; i++) {
        guardTown_update(&guardTowns[i]);
      }

      if (player_isAlive()) {
        uiConsole_addMessage(ultimaStrings[98]);
      }
    }

    if (player_isAlive() && playerCastle_update(deltaTime)) {
      playerActed = true;
    }
  }
  
  float *viewMatrix = camera_getViewProjectionMatrix(&camera);
  geometry_render(&backgroundGeometry, ultimaAssets.castleScreen.textureId, backgroundTransformationMatrix, viewMatrix);
  playerCastle_render(viewMatrix);
  sceneTown_renderPerson(&jesterGeometry, &jesterPosition, personTransform, viewMatrix);
  sceneTown_renderPerson(&princessGeometry, &princessPosition, personTransform, viewMatrix);
  sceneTown_renderPerson(&kingGeometry, &kingPosition, personTransform, viewMatrix);
  guardTown_render(viewMatrix);
  uiConsole_update(deltaTime);
}

static void sceneCastle_free() {
  geometry_free(&backgroundGeometry);
  playerCastle_free();
  guardTown_free();
}

Scene sceneCastle = {
  .scene_init = sceneCastle_init,
  .scene_update = sceneCastle_update,
  .scene_free = sceneCastle_free
};