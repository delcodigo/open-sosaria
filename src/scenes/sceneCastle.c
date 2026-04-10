#include <math.h>
#include <stdlib.h>
#include "sceneCastle.h"
#include "sceneDiskLoader.h"
#include "sceneTown.h"
#include "sceneOverworld.h"
#include "data/player.h"
#include "engine/geometry.h"
#include "engine/camera.h"
#include "entities/guardTown.h"
#include "entities/ui/uiConsole.h"
#include "entities/vmExecuter.h"
#include "entities/playerCastle.h"
#include "entities/ui/uiztats.h"
#include "maths/matrix4.h"
#include "maths/vector2.h"
#include "config.h"
#include "utils.h"

static Geometry backgroundGeometry;
static Geometry jesterGeometry;
static Geometry princessGeometry;
static Geometry kingGeometry;
static Geometry openedCellDoorGeometry;
static Vector2 jesterPosition = { 0 };
static Vector2 kingPosition = { 0 };
static float backgroundTransformationMatrix[16];
static float personTransform[16];

Vector2 princessPosition = { 0 };
int castleKey = 0;
int allowToTakeItemsFromCastle = 0;
int openedCellDoorIndex = -1;

static void sceneCastle_initialiseEntities() {
  jesterPosition.x = 37; jesterPosition.y = 6;
  princessPosition.x = 37; princessPosition.y = 14;
  kingPosition.x = 35; kingPosition.y = 4;
}

static void sceneCastle_init() {
  geometry_setSprite(&backgroundGeometry, OS_SCREEN_WIDTH, OS_SCREEN_HEIGHT, 0, 0, 1, 1);
  matrix4_setIdentity(backgroundTransformationMatrix);

  geometry_setSprite(&openedCellDoorGeometry, OS_TOWN_CASTLE_SPRITE_WIDTH, OS_TOWN_CASTLE_SPRITE_HEIGHT, 0, 0, 1, 1);

  camera_setPosition3f(&camera, 0.0f, 0.0f, 10.0f);

  sceneTown_initializeGeometry(2, &jesterGeometry);
  sceneTown_initializeGeometry(5, &princessGeometry);
  sceneTown_initializeGeometry(0, &kingGeometry);

  matrix4_setIdentity(personTransform);

  isPlayerInCastle = true;
  castleKey = 0;
  openedCellDoorIndex = -1;
  enemyEncounter.monsterId = 0;

  player.px = 1;
  player.py = 11;
  playerCastle_init();
  guardCastle_init();
  sceneCastle_initialiseEntities();

  uiConsole_addMessage(ultimaStrings[98]);
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
  
  if (x == 33 && y == 16 && openedCellDoorIndex == 0) {
    return false;
  }

  if (x == 37 && y == 16 && openedCellDoorIndex == 1) {
    return false;
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

static void sceneCastle_updatePrincess() {
  if (princessPosition.x < 0) { return; }

  if (enemyEncounter.monsterId > 0) {
    int dx = player.px - princessPosition.x;
    int dy = player.py - princessPosition.y;
    float distance = sqrtf(dx * dx + dy * dy);
    if (distance < 2.0f) {
      return;
    }

    if (dx != 0) { dx = dx / abs(dx); }
    if (dy != 0) { dy = dy / abs(dy); }

    if (!sceneCastle_isSolid(princessPosition.x + dx, princessPosition.y + dy) &&
        (princessPosition.x + dx != player.px || princessPosition.y + dy != player.py)) {
      princessPosition.x += dx;
      princessPosition.y += dy;
    }
  } else {
    int dx = (int)(rand01() * 3) - 1;
    int dy = (int)(rand01() * 3) - 1;

    if (dx == 0 && dy == 0) { return; }
    if (sceneCastle_isSolid(princessPosition.x + dx, princessPosition.y + dy)) { return; }
    if (princessPosition.x + dx != player.px || princessPosition.y + dy != player.py) {
      princessPosition.x += dx;
      princessPosition.y += dy;
    }
  }
}

void sceneCastle_attackAt(int x, int y) {
  char *target = NULL;
  TOWN_ENTITY_TYPE targetType = TOWN_ENTITY_TYPE_NONE;
  int targetIndex = -1;
  int T = -1;

  for (int i=0; i<OS_GUARD_TOWN_COUNT; i++) {
    if (guardTowns[i].x == x && guardTowns[i].y == y) {
      target = ultimaStrings[347];
      targetType = TOWN_ENTITY_TYPE_GUARD;
      targetIndex = i;
      T = 32 + i;
      break;
    }
  }

  if (princessPosition.x == x && princessPosition.y == y) {
    target = ultimaStrings[656];
    T = 41;
    targetType = TOWN_ENTITY_TYPE_PRINCESS;
  }

  if (jesterPosition.x == x && jesterPosition.y == y) {
    target = ultimaStrings[653];
    T = 38;
    targetType = TOWN_ENTITY_TYPE_JESTER;
  }

  if (kingPosition.x == x && kingPosition.y == y) {
    target = ultimaStrings[651];
    T = 31;
    targetType = TOWN_ENTITY_TYPE_KING;
    enemyEncounter.monsterId = 1;
  }

  if (T == -1) {
    uiConsole_queueMessage(ultimaStrings[657]);
    return;
  }

  uiConsole_replaceLastMessageFormat("%s%s", ultimaStrings[98], ultimaStrings[342]);
  uiConsole_queueMessage(target);

  if (rand01() * ((player.agility + (T - 40) * 3 + player.weapon) + 50) < 50) {
    uiConsole_queueMessage(ultimaStrings[657]);
    return;
  }
  
  enemyEncounter.monsterId = 1;

  int damage = (int)(((player.strength + player.weapon) / 2.0f) * rand01() + 1);
  uiConsole_queueMessageFormat("%s%d", ultimaStrings[658], damage);

  if (targetType == TOWN_ENTITY_TYPE_JESTER) {
    jesterPosition.x = -1;
    jesterPosition.y = -1;
    player.experience += 30;
    castleKey = (int)(rand01() * 2 + 1);
    uiConsole_queueMessage(ultimaStrings[659]);
    uiConsole_queueMessage(ultimaStrings[660]);
  } else if (targetType == TOWN_ENTITY_TYPE_PRINCESS) {
    princessPosition.x = -1;
    princessPosition.y = -1;
    player.experience += 10;
    uiConsole_queueMessage(ultimaStrings[659]);
  } else if (targetType == TOWN_ENTITY_TYPE_KING && player.agility * pow(rand01(), 3) > 50) {
    kingPosition.x = -1;
    kingPosition.y = -1;
    player.experience += 10000;
    uiConsole_queueMessage(ultimaStrings[659]);
  } else if (targetType == TOWN_ENTITY_TYPE_GUARD) {
    guardTowns[targetIndex].hp -= damage;
    if (guardTowns[targetIndex].hp <= 0) {
      guardTowns[targetIndex].x = -1;
      guardTowns[targetIndex].y = -1;
      player.experience += 50;
    }
  }

  uiConsole_updateStats();
}

static void sceneCastle_update(float deltaTime) {
  if (lagTime > 0) {
    lagTime -= deltaTime;
    if (lagTime < 0) { lagTime = 0; }
  }

  if (!queuedMessagesCount && lagTime <= 0 && !vmExecuter_update(deltaTime)) {
    if (respawnPlayer) {
      scene_load(&sceneOverworld);
      return;
    }

    if (ztatsActive){
      uiZtats_update(deltaTime);
      return;
    }

    if (playerActed) {
      playerActed = false;

      sceneCastle_updateJester();
      sceneCastle_updatePrincess();

      for (int i=0; i<OS_GUARD_TOWN_COUNT; i++) {
        guardTown_update(&guardTowns[i]);
      }

      if (player_isAlive()) {
        uiConsole_addMessage(ultimaStrings[98]);
      } else {
        sceneOverworld_attemptResurrection();
      }
    }

    if (player_isAlive() && playerCastle_update(deltaTime)) {
      playerActed = true;
    }
  }
  
  float *viewMatrix = camera_getViewProjectionMatrix(&camera);
  geometry_render(&backgroundGeometry, ultimaAssets.castleScreen.textureId, backgroundTransformationMatrix, viewMatrix);
  
  if (openedCellDoorIndex == 0) {
    matrix4_setPosition(personTransform, 33 * OS_TOWN_CASTLE_SPRITE_WIDTH, 16 * OS_TOWN_CASTLE_SPRITE_HEIGHT, 0.1f);
    geometry_render(&openedCellDoorGeometry, ultimaAssets.blackSprite.textureId, personTransform, viewMatrix);
  } else if (openedCellDoorIndex == 1) {
    matrix4_setPosition(personTransform, 37 * OS_TOWN_CASTLE_SPRITE_WIDTH, 16 * OS_TOWN_CASTLE_SPRITE_HEIGHT, 0.1f);
    geometry_render(&openedCellDoorGeometry, ultimaAssets.blackSprite.textureId, personTransform, viewMatrix);
  }

  playerCastle_render(viewMatrix);
  sceneTown_renderPerson(&jesterGeometry, &jesterPosition, personTransform, viewMatrix);
  sceneTown_renderPerson(&princessGeometry, &princessPosition, personTransform, viewMatrix);
  sceneTown_renderPerson(&kingGeometry, &kingPosition, personTransform, viewMatrix);
  guardTown_render(viewMatrix);
  uiConsole_update(deltaTime);
}

static void sceneCastle_free() {
  geometry_free(&backgroundGeometry);
  geometry_free(&jesterGeometry);
  geometry_free(&princessGeometry);
  geometry_free(&kingGeometry);
  geometry_free(&openedCellDoorGeometry);
  playerCastle_free();
  guardTown_free();
}

Scene sceneCastle = {
  .scene_init = sceneCastle_init,
  .scene_update = sceneCastle_update,
  .scene_free = sceneCastle_free
};