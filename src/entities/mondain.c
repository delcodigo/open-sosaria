#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "mondain.h"
#include "entities/ui/uiConsole.h"
#include "entities/lightningBoltEffect.h"
#include "entities/vmExecuter.h"
#include "data/player.h"
#include "scenes/sceneDiskLoader.h"
#include "scenes/sceneMondain.h"
#include "maths/matrix4.h"
#include "utils.h"

Mondain mondain;

static void mondain_updateSprite(int spriteIndex) {
  if (mondain.geometryIndex == spriteIndex) { 
    return; 
  }

  if (mondain.geometryIndex != 0) {
    geometry_free(&mondain.geometry);
  }

  float tx1 = (16.0f * spriteIndex) / (float) ultimaAssets.mondainSprites.width;
  float tx2 = (16.0f * spriteIndex + 16.0f) / (float) ultimaAssets.mondainSprites.width;
  geometry_setSprite(&mondain.geometry, 16, 16, tx1, 0, tx2, 1);
  mondain.geometryIndex = spriteIndex;
}

void mondain_init() {
  mondain.px = 15;
  mondain.py = 6;
  mondain.state = MONDAIN_STATE_IDLE;
  mondain.geometryIndex = 0;
  mondain.hp = 1000;

  matrix4_setIdentity(mondain.transformMatrix);

  mondain_updateSprite(1);
}

static void mondain_updateFlee() {
  int dx = -getSign(player.px - mondain.px);
  int dy = -getSign(player.py - mondain.py);

  if (sceneMondain_isValidPosition(mondain.px, mondain.py+dy) && dy != 0) {
    dx = 0;
  } else if (sceneMondain_isValidPosition(mondain.px+dx, mondain.py)) {
    dy = 0;
  }

  if ((dx != 0 || dy != 0) && sceneMondain_isValidPosition(mondain.px+dx, mondain.py+dy)) {
    mondain.px += dx;
    mondain.py += dy;
    mondain_updateSprite(6);
  } else {
    mondain.hp += 10;
    if (mondain.hp > 500) {
      mondain.state = MONDAIN_STATE_ACTIVE;
      mondain_updateSprite(1);
    }
  }
}

static void mondain_attack() {
  int dx = player.px - mondain.px;
  int dy = player.py - mondain.py;
  float distance = sqrtf(dx*dx + dy*dy);
  int attackType = 0;

  if (distance < 1.5f) {
    uiConsole_queueMessage(ultimaStrings[1225]);
    attackType = 1;
  } else if (distance < 7 && rand01() > 0.5f) {
    uiConsole_queueMessage(ultimaStrings[1226]);
    attackType = 2;
  } else {
    dx = getSign(player.px - mondain.px);
    dy = getSign(player.py - mondain.py);

    if (sceneMondain_isValidPosition(mondain.px, mondain.py+dy) && dy != 0) {
      dx = 0;
    } else if (sceneMondain_isValidPosition(mondain.px+dx, mondain.py)) {
      dy = 0;
    }

    if ((dx != 0 || dy != 0) && sceneMondain_isValidPosition(mondain.px+dx, mondain.py+dy)) {
      mondain.px += dx;
      mondain.py += dy;
      mondain_updateSprite(1);
    }
  }

  if (attackType == 1) {
    float accuracy = rand01() * 300.0f;
    float defense = (float)(player.strength + player.agility + player.stamina) / 3.0f + player.armor * 2.0f;
    if (accuracy < defense) {
      uiConsole_queueMessage(ultimaStrings[1227]);
      return;
    }

    int damage = (int)((float)player.health / 25.0f + rand01() * 20.0f);
    player.health -= damage;
    
    uiConsole_queueMessageFormat("%s%d", ultimaStrings[1228], damage);
    uiConsole_updateStats();
  } else if (attackType == 2) {
    int magicType = (int)(rand01() * 3 + 1);
    switch (magicType) {
      case 1:
        uiConsole_queueMessage(ultimaStrings[1229]);
        if (rand01() * 100 > player.agility && rand01() * 100 > player.intelligence) {
          int damage = (int)((float)player.health / 500.0f + rand01() * 100.0f);
          player.health -= damage;
          uiConsole_queueMessageFormat("%s%d", ultimaStrings[1230], damage);
          uiConsole_updateStats();
        } else {
          uiConsole_queueMessage(ultimaStrings[1231]);
        }
        break;
      case 2:
        uiConsole_queueMessage(ultimaStrings[1232]);
        if (rand01() > 0.7f) {
          uiConsole_queueMessage(ultimaStrings[1233]);
          for (int i=1;i<=6;i++) {
            *((&player.health)+i) = (int)(*((&player.health)+i) * 0.9f);
          }
        } else {
          uiConsole_queueMessage(ultimaStrings[1234]);
        }
        break;
      case 3:
        uiConsole_queueMessage(ultimaStrings[1235]);
        if (rand01() >= 0.7) {
          int damage = (int)(rand01() * ((float) player.health / 20.0f));
          player.health -= damage;
          uiConsole_queueMessageFormat("%s%d", ultimaStrings[1237], damage);
          uiConsole_updateStats();
        } else {
          uiConsole_queueMessage(ultimaStrings[1236]);
        }
        break;
    }
  }
}

void mondain_receiveDamage(int damage) {
  mondain.hp -= damage;
  
  if (mondain.hp > 500 || mondain.state == MONDAIN_STATE_DEFEATED) {
    return;
  }

  if (mondain.hp > 0) {
    mondain_transform();
  } else {
    mondain_defeat();
    uiConsole_queueMessage(ultimaStrings[1134]);

    VMInstruction *instructions = malloc(3 * sizeof(VMInstruction));
    instructions[0].type = VM_INSTRUCTION_TYPE_WAIT;
    instructions[0].wait.duration = 2.0f;

    instructions[1].type = VM_INSTRUCTION_TYPE_QUEUE_CONSOLE_MESSAGE;
    memset(instructions[1].consoleMessage.message, 0, sizeof(instructions[1].consoleMessage.message));
    strcpy(instructions[1].consoleMessage.message, ultimaStrings[1135]);

    instructions[2].type = VM_INSTRUCTION_TYPE_WAIT;
    instructions[2].wait.duration = 2.0f;

    vmExecuter_init(instructions, 3);
  }
}

void mondain_transform() {
  mondain.state = MONDAIN_STATE_TRANSFORMED;
  mondain_updateSprite(6);
}

void mondain_defeat() {
  mondain.state = MONDAIN_STATE_DEFEATED;
  mondain_updateSprite(7);
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
    mondain_transform();
  }

  if (mondain.state == MONDAIN_STATE_DEFEATED) {
    return;
  }

  lightningBoltEffect_cast();
  if (mondain.state == MONDAIN_STATE_TRANSFORMED) {
    mondain_updateFlee();
  } else {
    mondain_attack();
  }
}

void mondain_render(float *viewMatrix) {
  matrix4_setPosition(mondain.transformMatrix, mondain.px * 15, mondain.py * 15, 1);
  geometry_render(&mondain.geometry, ultimaAssets.mondainSprites.textureId, mondain.transformMatrix, viewMatrix);
}

void mondain_free() {
  geometry_free(&mondain.geometry);
}