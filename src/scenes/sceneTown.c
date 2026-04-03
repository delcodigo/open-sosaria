#include "sceneTown.h"
#include "sceneDiskLoader.h"
#include "sceneOverworld.h"
#include "data/player.h"
#include "engine/geometry.h"
#include "maths/matrix4.h"
#include "maths/vector2.h"
#include "entities/ui/uiConsole.h"
#include "entities/ui/uiztats.h"
#include "entities/playerTown.h"
#include "entities/vmExecuter.h"
#include "entities/guardTown.h"
#include "entities/merchantTown.h"
#include "utils.h"
#include "config.h"

#define OS_TOWN_MERCHANTS_COUNT 6

static Vector2 merchantsPositions[OS_TOWN_MERCHANTS_COUNT] = { 0 };
Vector2 wenchPosition = { 0 };
static Vector2 bardPosition = { 0 };
static Geometry merchantGeometry;
static Geometry wenchGeometry;
static Geometry bardGeometry;
static float personTransform[16];

static Geometry townGeometry;
static float townTransform[16];

static void sceneTown_initializeGeometry(int spriteIndex, Geometry *geometry) {
  float tx1 = (spriteIndex * OS_TOWN_CASTLE_SPRITE_WIDTH) / (float)ultimaAssets.townCastleSprites.width;
  float tx2 = ((spriteIndex + 1) * OS_TOWN_CASTLE_SPRITE_WIDTH) / (float)ultimaAssets.townCastleSprites.width;
  geometry_setSprite(geometry, OS_TOWN_CASTLE_SPRITE_WIDTH, OS_TOWN_CASTLE_SPRITE_HEIGHT, tx1, 0.0f, tx2, 1.0f);
}

static void sceneTown_initializePositions() {
  wenchPosition.x = 35; wenchPosition.y = 7;
  bardPosition.x = 15; bardPosition.y = 6;

  merchantsPositions[0].x = 6; merchantsPositions[0].y = 4;
  merchantsPositions[1].x = 7; merchantsPositions[1].y = 17;
  merchantsPositions[2].x = 14; merchantsPositions[2].y = 17;
  merchantsPositions[3].x = 25; merchantsPositions[3].y = 4;
  merchantsPositions[4].x = 26; merchantsPositions[4].y = 14;
  merchantsPositions[5].x = 32; merchantsPositions[5].y = 4;
}

static void sceneTown_init() {
  geometry_setSprite(&townGeometry, OS_SCREEN_WIDTH, OS_SCREEN_HEIGHT, 0.0f, 0.0f, 1.0f, 1.0f);
  matrix4_setIdentity(townTransform);

  sceneTown_initializeGeometry(4, &merchantGeometry);
  sceneTown_initializeGeometry(5, &wenchGeometry);
  sceneTown_initializeGeometry(2, &bardGeometry);
  matrix4_setIdentity(personTransform);

  camera_setPosition3f(&camera, 0.0f, 0.0f, 10.0f);

  drunkLevel = 0;

  player.px = 20;
  player.py = 20;
  playerTown_init();
  guardTown_init();
  sceneTown_initializePositions();
}

bool sceneTown_isSolid(int x, int y) {
  if (ultimaAssets.townCollisionMap[y][x]) {
    return true;
  }

  for (int i=0; i<OS_GUARD_TOWN_COUNT; i++) {
    if (guardTowns[i].x == x && guardTowns[i].y == y) {
      return true;
    }
  }

  for (int i=0; i<OS_TOWN_MERCHANTS_COUNT; i++) {
    if (merchantsPositions[i].x == x && merchantsPositions[i].y == y) {
      return true;
    }
  }

  if (wenchPosition.x == x && wenchPosition.y == y) {
    return true;
  }

  if (bardPosition.x == x && bardPosition.y == y) {
    return true;
  }

  return false;
}

void sceneTown_attackAt(int x, int y) {
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

  for (int i=0; i<OS_TOWN_MERCHANTS_COUNT; i++) {
    if (merchantsPositions[i].x == x && merchantsPositions[i].y == y) {
      target = ultimaStrings[350];
      targetType = TOWN_ENTITY_TYPE_MERCHANT;
      targetIndex = i;
      T = 40;
      break;
    }
  }

  if (wenchPosition.x == x && wenchPosition.y == y) {
    target = ultimaStrings[351];
    T = 41;
    targetType = TOWN_ENTITY_TYPE_WENCH;
  }

  if (bardPosition.x == x && bardPosition.y == y) {
    target = ultimaStrings[348];
    T = 38;
    targetType = TOWN_ENTITY_TYPE_BARD;
  }

  if (T == -1) {
    uiConsole_queueMessage(ultimaStrings[344]);
    return;
  }

  uiConsole_replaceLastMessageFormat("%s%s", ultimaStrings[98], ultimaStrings[342]);
  uiConsole_queueMessage(target);

  if (rand01() * ((player.strength + (T - 40) * 3 + player.weapon) + 75) < 50) {
    uiConsole_queueMessage(ultimaStrings[352]);
    return;
  }
  
  enemyEncounter.monsterId = 1;

  int damage = (int)(((player.strength + player.weapon) / 2.0f) * rand01() + 1);
  uiConsole_queueMessageFormat("%s%d", ultimaStrings[353], damage);

  if (targetType == TOWN_ENTITY_TYPE_BARD) {
    bardPosition.x = -1;
    bardPosition.y = -1;
    player.eptns += 20;
  } else if (targetType == TOWN_ENTITY_TYPE_WENCH) {
    wenchPosition.x = -1;
    wenchPosition.y = -1;
    player.eptns += 10;
  } else if (targetType == TOWN_ENTITY_TYPE_MERCHANT) {
    merchantsPositions[targetIndex].x = -1;
    merchantsPositions[targetIndex].y = -1;
    player.eptns += 25;
  } else if (targetType == TOWN_ENTITY_TYPE_GUARD) {
    guardTowns[targetIndex].hp -= damage;
    if (guardTowns[targetIndex].hp <= 0) {
      guardTowns[targetIndex].x = -1;
      guardTowns[targetIndex].y = -1;
      player.eptns += 40;
    } else {
      return;
    }
  }

  uiConsole_queueMessage(ultimaStrings[354]);
}

static void sceneTown_renderPerson(Geometry *geometry, Vector2 *position, float *transform, float *viewMatrix) {
  matrix4_setPosition(transform, position->x * OS_TOWN_CASTLE_SPRITE_WIDTH, position->y * OS_TOWN_CASTLE_SPRITE_HEIGHT, 1);
  geometry_render(geometry, ultimaAssets.townCastleSprites.textureId, transform, viewMatrix);
}

static void sceneTown_updateBard() {
  if (bardPosition.x < 0) { return; }

  int dx = (int)(rand01() * 3) - 1;
  int dy = (int)(rand01() * 3) - 1;

  if (dx == 0 && dy == 0) { return; }
  if (bardPosition.y + dy > 20) { return; }

  if (sceneTown_isSolid(bardPosition.x + dx, bardPosition.y + dy)) { 
    if (rand01() > 0.7f){
      uiConsole_addMessage(ultimaStrings[596]);
      uiConsole_addMessage(ultimaStrings[597]);
    }

    return;
  }

  if (bardPosition.x + dx != player.px || bardPosition.y + dy != player.py) {
    bardPosition.x += dx;
    bardPosition.y += dy;
    return;
  }

  for (int i=0;i<OS_WEAPONS_COUNT;i++) {
    if (player.weapons[i] > 0 && player.weapon != i) {
      player.weapons[i] -= 1;
    }
  }

  if (rand01() * 50 < player.wisdom) {
    uiConsole_addMessage(ultimaStrings[598]);
  }
}

static void sceneTown_updateWench() {
  if (wenchPosition.x < 0) { return; }

  int dx = (int)(rand01() * 3) - 1;
  int dy = (int)(rand01() * 3) - 1;

  if (dx == 0 && dy == 0) { return; }
  if (wenchPosition.y + dy > 8) { return; }
  if (sceneTown_isSolid(wenchPosition.x + dx, wenchPosition.y + dy)) { return; }
  if (wenchPosition.x + dx != player.px || wenchPosition.y + dy != player.py) {
    wenchPosition.x += dx;
    wenchPosition.y += dy;
  }
}

static void sceneTown_update(float deltaTime) {
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

      sceneTown_updateBard();
      sceneTown_updateWench();
      
      for (int i=0; i<OS_GUARD_TOWN_COUNT; i++) {
        guardTown_update(&guardTowns[i]);
      }
      
      if (player_isAlive()) {
        uiConsole_addMessage(ultimaStrings[98]);
      } else {
        sceneOverworld_attemptResurrection();
      }
    }

    if (player_isAlive() && playerTown_update(deltaTime)) {
      playerActed = true;
    }
  }

  float *viewMatrix = camera_getViewProjectionMatrix(&camera);
  geometry_render(&townGeometry, ultimaAssets.townScreen.textureId, townTransform, viewMatrix);

  for (int i=0; i<OS_TOWN_MERCHANTS_COUNT; i++) {
    sceneTown_renderPerson(&merchantGeometry, &merchantsPositions[i], personTransform, viewMatrix);
  }

  sceneTown_renderPerson(&wenchGeometry, &wenchPosition, personTransform, viewMatrix);
  sceneTown_renderPerson(&bardGeometry, &bardPosition, personTransform, viewMatrix);

  playerTown_render(viewMatrix);
  guardTown_render(viewMatrix);
  uiConsole_update(deltaTime);
}

static void sceneTown_free() {
  geometry_free(&townGeometry);
  geometry_free(&merchantGeometry);
  geometry_free(&wenchGeometry);
  geometry_free(&bardGeometry);
  playerTown_free();
  guardTown_free();
}

Scene sceneTown = {
  .scene_init = sceneTown_init,
  .scene_update = sceneTown_update,
  .scene_free = sceneTown_free
};