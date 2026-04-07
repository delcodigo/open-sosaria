#include <math.h>
#include <stdlib.h>
#include "guardTown.h"
#include "entities/ui/uiConsole.h"
#include "data/player.h"
#include "engine/geometry.h"
#include "scenes/sceneDiskLoader.h"
#include "scenes/sceneOverworld.h"
#include "scenes/sceneTown.h"
#include "maths/matrix4.h"
#include "config.h"
#include "utils.h"

GuardTown guardTowns[OS_GUARD_TOWN_COUNT] = { 0 };
static Geometry guardTownGeometry;
static float transformMatrix[16];

void guardTown_init() {
  guardTowns[0].x = 17; guardTowns[0].y = 20; guardTowns[0].hp = 20;
  guardTowns[1].x = 22; guardTowns[1].y = 20; guardTowns[1].hp = 20;
  guardTowns[2].x = 21; guardTowns[2].y = 10; guardTowns[2].hp = 20;
  guardTowns[3].x =  3; guardTowns[3].y = 11; guardTowns[3].hp = 20;
  guardTowns[4].x = 37; guardTowns[4].y =  9; guardTowns[4].hp = 20;
  guardTowns[5].x = 19; guardTowns[5].y =  2; guardTowns[5].hp = 20;

  float tx1 = (1.0f * OS_TOWN_CASTLE_SPRITE_WIDTH) / (float)ultimaAssets.townCastleSprites.width;
  float tx2 = (2.0f * OS_TOWN_CASTLE_SPRITE_WIDTH) / (float)ultimaAssets.townCastleSprites.width;
  geometry_setSprite(&guardTownGeometry, OS_TOWN_CASTLE_SPRITE_WIDTH, OS_TOWN_CASTLE_SPRITE_HEIGHT, tx1, 0.0f, tx2, 1.0f);

  matrix4_setIdentity(transformMatrix);
}

void guardCastle_init() {
  guardTowns[0].x =  1; guardTowns[0].y =  8; guardTowns[0].hp = 20;
  guardTowns[1].x = 17; guardTowns[1].y =  8; guardTowns[1].hp = 20;
  guardTowns[2].x = 17; guardTowns[2].y = 13; guardTowns[2].hp = 20;
  guardTowns[3].x = 26; guardTowns[3].y =  8; guardTowns[3].hp = 20;
  guardTowns[4].x = 27; guardTowns[4].y = 15; guardTowns[4].hp = 20;
  guardTowns[5].x = 38; guardTowns[5].y = 11; guardTowns[5].hp = 20;

  float tx1 = (1.0f * OS_TOWN_CASTLE_SPRITE_WIDTH) / (float)ultimaAssets.townCastleSprites.width;
  float tx2 = (2.0f * OS_TOWN_CASTLE_SPRITE_WIDTH) / (float)ultimaAssets.townCastleSprites.width;
  geometry_setSprite(&guardTownGeometry, OS_TOWN_CASTLE_SPRITE_WIDTH, OS_TOWN_CASTLE_SPRITE_HEIGHT, tx1, 0.0f, tx2, 1.0f);

  matrix4_setIdentity(transformMatrix);
}

void guardTown_update(GuardTown *guard) {
  if (enemyEncounter.monsterId == 0) { return;}
  if (guard->hp <= 0) { return; }

  int dx = player.px - guard->x;
  int dy = player.py - guard->y;
  float distance = sqrtf(dx * dx + dy * dy);

  if (distance > 9.0f) { return; }

  if (distance < 2) {
    uiConsole_queueMessage(ultimaStrings[593]);
    if (rand01() * player.strength + player.armor * 4 > 30 || rand01() > 0.75f) {
      uiConsole_queueMessage(ultimaStrings[594]);
      lagTime += 0.1f;
      return;
    }

    int damage = (int) (rand01() * 10 + player.health / 100);
    player.health -= damage;
    if (player.health < 0) { player.health = 0; }
    uiConsole_updateStats();
    uiConsole_queueMessageFormat("%s%d", ultimaStrings[595], damage);
    return;
  }

  if (dy != 0) { dy = dy / abs(dy); }
  if (dx != 0) { dx = dx / abs(dx); }

  if (!sceneTown_isSolid(guard->x, guard->y + dy)) {
    guard->y += dy;
  } else if (!sceneTown_isSolid(guard->x + dx, guard->y)) {
    guard->x += dx;
  }
}

void guardTown_render(float *viewMatrix) {
  for (int i=0; i<OS_GUARD_TOWN_COUNT; i++) {
    matrix4_setPosition(transformMatrix, guardTowns[i].x * OS_TOWN_CASTLE_SPRITE_WIDTH, guardTowns[i].y * OS_TOWN_CASTLE_SPRITE_HEIGHT, 2.0f);

    geometry_render(&guardTownGeometry, ultimaAssets.townCastleSprites.textureId, transformMatrix, viewMatrix);
  }
}

void guardTown_free() {
  geometry_free(&guardTownGeometry);
}