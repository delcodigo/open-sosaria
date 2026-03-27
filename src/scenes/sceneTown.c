#include "sceneTown.h"
#include "sceneDiskLoader.h"
#include "data/player.h"
#include "engine/geometry.h"
#include "maths/matrix4.h"
#include "maths/vector2.h"
#include "entities/ui/uiConsole.h"
#include "entities/playerTown.h"
#include "entities/vmExecuter.h"
#include "entities/guardTown.h"
#include "config.h"

#define OS_TOWN_MERCHANTS_COUNT 6

static Vector2 merchantsPositions[OS_TOWN_MERCHANTS_COUNT] = {
  {  6,  4 },
  {  7, 17 },
  { 14, 17 },
  { 25,  4 },
  { 26, 14 },
  { 32,  4 }
};
static Vector2 wenchPosition = { 35, 7 };
static Vector2 bardPosition = { 15, 6 };
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

static void sceneTown_init() {
  geometry_setSprite(&townGeometry, OS_SCREEN_WIDTH, OS_SCREEN_HEIGHT, 0.0f, 0.0f, 1.0f, 1.0f);
  matrix4_setIdentity(townTransform);

  sceneTown_initializeGeometry(4, &merchantGeometry);
  sceneTown_initializeGeometry(5, &wenchGeometry);
  sceneTown_initializeGeometry(6, &bardGeometry);
  matrix4_setIdentity(personTransform);

  camera_setPosition3f(&camera, 0.0f, 0.0f, 10.0f);

  player.px = 20;
  player.py = 20;
  playerTown_init();
  guardTown_init();
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

static void sceneTown_renderPerson(Geometry *geometry, Vector2 *position, float *transform, float *viewMatrix) {
  matrix4_setPosition(transform, position->x * OS_TOWN_CASTLE_SPRITE_WIDTH, position->y * OS_TOWN_CASTLE_SPRITE_HEIGHT, 1);
  geometry_render(geometry, ultimaAssets.townCastleSprites.textureId, transform, viewMatrix);
}

static void sceneTown_update(float deltaTime) {
  if (lagTime > 0) {
    lagTime -= deltaTime;
    if (lagTime < 0) { lagTime = 0; }
  }
  
  if (!queuedMessagesCount && lagTime <= 0 && !vmExecuter_update(deltaTime)) {
    if (playerActed) {
      playerActed = false;
      
      if (player_isAlive()) {
        uiConsole_addMessage(ultimaStrings[98]);
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