#include "mondain.h"
#include "entities/ui/uiConsole.h"
#include "entities/lightningBoltEffect.h"
#include "scenes/sceneDiskLoader.h"
#include "maths/matrix4.h"
#include "utils.h"

Mondain mondain;

static void mondain_updateSprite(int spriteIndex) {
  float tx1 = (16.0f * spriteIndex) / (float) ultimaAssets.mondainSprites.width;
  float tx2 = (16.0f * spriteIndex + 16.0f) / (float) ultimaAssets.mondainSprites.width;
  geometry_setSprite(&mondain.geometry, 16, 16, tx1, 0, tx2, 1);
}

void mondain_init() {
  mondain.position.x = 15;
  mondain.position.y = 6;
  mondain.state = MONDAIN_STATE_IDLE;
  mondain.hp = 1000;

  matrix4_setIdentity(mondain.transformMatrix);

  mondain_updateSprite(1);
}

void mondain_update() {
  if (mondain.state == MONDAIN_STATE_IDLE) {
    if (rand01() > 0.8f) {
      uiConsole_queueMessage(ultimaStrings[1224]);
    }

    return;
  }

  if (mondain.state == MONDAIN_STATE_DEFEATED) {
    mondain.hp += 25;
    if (mondain.hp <= 0) {
      return;
    }
  }

  if (mondain.state == MONDAIN_STATE_DEFEATED && mondain.hp > 0) {
    mondain.state = MONDAIN_STATE_TRANSFORMED;
    geometry_free(&mondain.geometry);
    mondain_updateSprite(6);
  }

  if (mondain.state == MONDAIN_STATE_DEFEATED) {
    return;
  }

  lightningBoltEffect_cast();
}

void mondain_render(float *viewMatrix) {
  matrix4_setPosition(mondain.transformMatrix, mondain.position.x * 15, mondain.position.y * 15, 1);
  geometry_render(&mondain.geometry, ultimaAssets.mondainSprites.textureId, mondain.transformMatrix, viewMatrix);
}

void mondain_free() {
  geometry_free(&mondain.geometry);
}