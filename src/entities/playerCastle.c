#include "playerCastle.h"
#include "engine/geometry.h"
#include "engine/input.h"
#include "playerTown.h"
#include "playerCommons.h"
#include "data/player.h"
#include "scenes/sceneDiskLoader.h"
#include "maths/matrix4.h"
#include "entities/ui/uiConsole.h"
#include "config.h"

static Geometry playerCastleGeometry;
static float transformMatrix[16];

void playerCastle_init() {
  float tx1 = (6.0f * OS_TOWN_CASTLE_SPRITE_WIDTH) / (float)ultimaAssets.townCastleSprites.width;
  float tx2 = (7.0f * OS_TOWN_CASTLE_SPRITE_WIDTH) / (float)ultimaAssets.townCastleSprites.width;

  geometry_setSprite(&playerCastleGeometry, OS_TOWN_CASTLE_SPRITE_WIDTH, OS_TOWN_CASTLE_SPRITE_HEIGHT, tx1, 0, tx2, 1);
}

bool playerCastle_update(float deltaTime) {
  bool acted = false;
  
  if (keyRepeatDelay <= 0) {
    switch (playerState) {
      case PLAYER_STATE_IDLE:
        if (playerCommons_updateZtats()) { acted = true; } else
        if (playerCommons_updateReady()) { acted = true; } else
        if (playerCommons_updateWait()) { acted = true; } else
        if (playerTown_updateMovement(deltaTime)) { acted = true; }
        break;

      case PLAYER_STATE_READY_TYPE:
        if (playerCommons_updateReady()) { acted = true; }
        break;

      default:
        break;
    }
  } else {
    keyRepeatDelay -= deltaTime;
    if (keyRepeatDelay < 0) {
      keyRepeatDelay = 0;
    }
  }

  return acted;
}

void playerCastle_render(float *viewMatrix) {
  matrix4_setIdentity(transformMatrix);
  matrix4_setPosition(transformMatrix, player.px * OS_TOWN_CASTLE_SPRITE_WIDTH, player.py * OS_TOWN_CASTLE_SPRITE_HEIGHT, 1);

  geometry_render(&playerCastleGeometry, ultimaAssets.townCastleSprites.textureId, transformMatrix, viewMatrix);
}

void playerCastle_free() {
  geometry_free(&playerCastleGeometry);
}