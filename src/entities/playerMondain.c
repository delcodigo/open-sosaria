#include "playerMondain.h"
#include "engine/geometry.h"
#include "engine/texture.h"
#include "data/player.h"
#include "scenes/sceneDiskLoader.h"
#include "scenes/sceneMondain.h"
#include "maths/matrix4.h"
#include "engine/input.h"
#include "entities/ui/uiConsole.h"
#include "playerCommons.h"
#include "mondain.h"

static Geometry playerGeometry;
static float transformMatrix[16];

void playerMondain_init() {
  float tx1 = 0;
  float tx2 = 16.0f / (float) ultimaAssets.mondainSprites.width;
  geometry_setSprite(&playerGeometry, 16, 16, tx1, 0, tx2, 1);

  matrix4_setIdentity(transformMatrix);

  playerState = PLAYER_STATE_IDLE;
}

void playerMondain_render(float *viewMatrix) {
  matrix4_setPosition(transformMatrix, player.px * 15, player.py * 15, 1);
  geometry_render(&playerGeometry, ultimaAssets.mondainSprites.textureId, transformMatrix, viewMatrix);
}

static bool playerMondain_updateMovement(float deltaTime) {
  int directionMessageIndex = 0;
  int dx = 0;
  int dy = 0;

  if (input.up) {
    directionMessageIndex = 117;
    dy = -1;
  } else if (input.down) {
    directionMessageIndex = 118;
    dy = 1;
  } else if (input.right) {
    directionMessageIndex = 119;
    dx = 1;
  } else if (input.left) {
    directionMessageIndex = 120;
    dx = -1;
  }

  if (dx != 0 || dy != 0) {
    uiConsole_replaceLastMessageFormat("%s%s", ultimaStrings[98], ultimaStrings[directionMessageIndex]);

    if (!sceneMondain_isValidPosition(player.px + dx, player.py + dy)) {
      uiConsole_queueMessage(ultimaStrings[1126]);
      int damage = (int)((float)player.health / 10.0f);
      player.health -= damage;
      keyRepeatDelay = 0.3f;
      uiConsole_queueMessageFormat("%s%d", ultimaStrings[1127], damage);
      uiConsole_updateStats();
      return true;
    }

    player.px += dx;
    player.py += dy;

    sceneMondain_checkForGemTransform();

    keyRepeatDelay = 0.1f;
    player_consumeTownFood();
    return true;
  } else {
    waitingTime += deltaTime;
    if (waitingTime >= 5.0f) {
      waitingTime = 0.0f;
      uiConsole_replaceLastMessageFormat("%s%s", ultimaStrings[98], ultimaStrings[99]);
      player_waitPenalty();
      return true;
    }
  }

  return false;
}

static bool playerMondain_updateBoard() {
  if (input.b == 1) {
    input.b = 2;

    uiConsole_replaceLastMessageFormat("%s%s", ultimaStrings[98], ultimaStrings[1143]);
    uiConsole_queueMessage(ultimaStrings[1144]);

    return true;
  }

  return false;
}

static bool playerMondain_updateDropping() {
  if (input.d == 1) {
    input.d = 2;

    uiConsole_replaceLastMessageFormat("%s%s", ultimaStrings[98], ultimaStrings[1160]);
    uiConsole_queueMessage(ultimaStrings[1161]);

    return true;
  }

  return false;
}

static bool playerMondain_updateEntering() {
  if (input.e == 1) {
    input.e = 2;

    uiConsole_replaceLastMessageFormat("%s%s", ultimaStrings[98], ultimaStrings[1162]);
    uiConsole_queueMessage(ultimaStrings[1163]);

    return true;
  }

  return false;
}

static bool playerMondain_updateFiring() {
  if (input.f == 1) {
    input.f = 2;

    uiConsole_replaceLastMessageFormat("%s%s", ultimaStrings[98], ultimaStrings[1164]);
    uiConsole_queueMessage(ultimaStrings[1165]);

    return true;
  }

  return false;
}

static bool playerMondain_updateInform() {
  if (input.i == 1) {
    input.i = 2;

    uiConsole_replaceLastMessageFormat("%s%s", ultimaStrings[98], ultimaStrings[1171]);
    uiConsole_queueMessageFormat("^T1%s", ultimaStrings[1172]);
    uiConsole_queueMessageFormat("^T1%s", ultimaStrings[1173]);
    uiConsole_queueMessageFormat("^T1%s", ultimaStrings[1174]);

    return true;
  }

  return false;
}

static bool playerMondain_updateOpen() {
  if (input.o == 1) {
    input.o = 2;

    uiConsole_replaceLastMessageFormat("%s%s", ultimaStrings[98], ultimaStrings[1180]);

    return true;
  }

  return false;
}

static bool playerMondain_updateSaving() {
  if (input.q == 1) {
    input.q = 2;

    uiConsole_replaceLastMessageFormat("%s%s", ultimaStrings[98], ultimaStrings[1182]);
    uiConsole_queueMessage(ultimaStrings[1183]);

    return true;
  }

  return false;
}

static bool playerMondain_updateStealing() {
  if (input.s == 1) {
    input.s = 2;

    uiConsole_replaceLastMessageFormat("%s%s", ultimaStrings[98], ultimaStrings[1208]);
    uiConsole_queueMessage(ultimaStrings[1209]);

    return true;
  }

  return false;
}

static bool playerMondain_updateTalking() {
  if (input.t == 1) {
    input.t = 2;

    uiConsole_replaceLastMessageFormat("%s%s", ultimaStrings[98], ultimaStrings[1210]);
    uiConsole_queueMessage(ultimaStrings[1211]);

    return true;
  }

  return false;
}

static bool playerMondain_updateUnlocking() {
  if (input.u == 1) {
    input.u = 2;

    uiConsole_replaceLastMessageFormat("%s%s", ultimaStrings[98], ultimaStrings[1212]);

    return true;
  }

  return false;
}

static bool playerMondain_updateXit() {
  if (input.x == 1) {
    input.x = 2;

    uiConsole_replaceLastMessageFormat("%s%s", ultimaStrings[98], ultimaStrings[1215]);
    uiConsole_queueMessage(ultimaStrings[1216]);

    return true;
  }

  return false;
}

static bool playerMondain_updateGet() {
  if (input.g == 1) {
    input.g = 2;

    uiConsole_replaceLastMessageFormat("%s%s", ultimaStrings[98], ultimaStrings[1166]);

    if (mondain.state == MONDAIN_STATE_IDLE) {
      mondain.state = MONDAIN_STATE_ACTIVE;
    }

    if (sceneMondain_getDistanceToGem() >= 1.5f || !sceneMondain_isGemActive()) {
      uiConsole_queueMessage(ultimaStrings[1167]);
      return true;
    }

    sceneMondain_destroyGem();
    int damage = (int)((float) player.health * 0.75f);
    player.health -= damage;

    uiConsole_queueMessageFormat("%s%d", ultimaStrings[1168], damage);
    uiConsole_queueMessage(ultimaStrings[1169]);
    uiConsole_updateStats();

    return true;
  }

  return false;
}

bool playerMondain_update(float deltaTime) {
  bool acted = false;

  if (keyRepeatDelay <= 0.0f) {
    switch (playerState) {
      case PLAYER_STATE_IDLE:
        if (playerMondain_updateBoard()) { acted = true; } else
        if (playerMondain_updateDropping()) { acted = true; } else
        if (playerMondain_updateEntering()) { acted = true; } else
        if (playerMondain_updateFiring()) { acted = true; } else
        if (playerMondain_updateInform()) { acted = true; } else
        if (playerMondain_updateOpen()) { acted = true; } else
        if (playerMondain_updateSaving()) { acted = true; } else
        if (playerCommons_updateReady()) { acted = true; } else
        if (playerMondain_updateStealing()) { acted = true; } else
        if (playerMondain_updateTalking()) { acted = true; } else
        if (playerMondain_updateUnlocking()) { acted = true; } else
        if (playerMondain_updateXit()) { acted = true; } else
        if (playerMondain_updateGet()) { acted = true; } else
        if (playerCommons_updateZtats()) { acted = true; } else
        if (playerCommons_updateWait()) { acted = true; } else
        if (playerMondain_updateMovement(deltaTime)) { acted = true; }
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

  if (acted) { 
    waitingTime = 0; 
  }

  return acted;
}

void playerMondain_free() {
  geometry_free(&playerGeometry);
}