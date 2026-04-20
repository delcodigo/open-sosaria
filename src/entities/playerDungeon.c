#include <math.h>
#include "entities/ui/uiConsole.h"
#include "entities/vmExecuter.h"
#include "playerDungeon.h"
#include "playerCommons.h"
#include "data/player.h"
#include "engine/input.h"
#include "scenes/sceneDiskLoader.h"
#include "scenes/sceneDungeon.h"
#include "scenes/sceneOverworld.h"
#include "utils.h"

static int hp = 0;

void playerDungeon_init() {
  hp = 0;
}

static bool playerDungeon_updateRotation() {
  if (input.left) {
    int dx = player.dx;
    player.dx = player.dy;
    player.dy = -dx;
    keyRepeatDelay = 0.2f;
    uiConsole_replaceLastMessageFormat("%s%s", ultimaStrings[98], ultimaStrings[866]);
    return true;
  } else if (input.right) {
    int dx = player.dx;
    player.dx = -player.dy;
    player.dy = dx;
    keyRepeatDelay = 0.2f;
    uiConsole_replaceLastMessageFormat("%s%s", ultimaStrings[98], ultimaStrings[867]);
    return true;
  } else if (input.down) {
    player.dx = -player.dx;
    player.dy = -player.dy;
    keyRepeatDelay = 0.2f;
    uiConsole_replaceLastMessageFormat("%s%s", ultimaStrings[98], ultimaStrings[865]);
    return true;
  }

  return false;
}

static bool playerDungeon_updateMovement() {
  if (input.up) {
    uiConsole_replaceLastMessageFormat("%s%s", ultimaStrings[98], ultimaStrings[859]);

    if (sceneDungeon_isSolid(player.px + player.dx, player.py + player.dy)) {
      uiConsole_addMessage(ultimaStrings[860]);
      return false;
    }

    player.px += player.dx;
    player.py += player.dy;
    keyRepeatDelay = 0.2f;
    return true;
  }

  return false;
}

static bool playerDungeon_updateBoard() {
  if (input.b == 1) {
    input.b = 2;
    uiConsole_replaceLastMessageFormat("%s%s", ultimaStrings[98], ultimaStrings[877]);
    return true;
  }

  return false;
}

static bool playerDungeon_updateDrop() {
  if (input.d == 1) {
    input.d = 2;
    uiConsole_replaceLastMessageFormat("%s%s", ultimaStrings[98], ultimaStrings[918]);
    return true;
  }

  return false;
}

static bool playerDungeon_updateEnter() {
  if (input.e == 1) {
    input.e = 2;
    uiConsole_replaceLastMessageFormat("%s%s", ultimaStrings[98], ultimaStrings[919]);
    return true;
  }

  return false;
}

static bool playerDungeon_updateFire() {
  if (input.f == 1) {
    input.f = 2;
    uiConsole_replaceLastMessageFormat("%s%s", ultimaStrings[98], ultimaStrings[920]);
    return true;
  }

  return false;
}

static bool playerDungeon_updateGet() {
  if (input.g == 1) {
    input.g = 2;
    uiConsole_replaceLastMessageFormat("%s%s", ultimaStrings[98], ultimaStrings[921]);
    return true;
  }

  return false;
}

static bool playerDungeon_updateInform() {
  if (input.i == 1) {
    input.i = 2;
    uiConsole_replaceLastMessageFormat("%s%s", ultimaStrings[98], ultimaStrings[923]);

    int frontTile = dungeonMap[player.px + player.dx][player.py + player.dy];
    if (frontTile == 2) {
      dungeonMap[player.px + player.dx][player.py + player.dy] = 9;
    } else if (frontTile == 3) {
      dungeonMap[player.px + player.dx][player.py + player.dy] = 4;
    }

    return true;
  }

  return false;
}

static bool playerDungeon_updateKlimb() {
  if (input.k == 1) {
    input.k = 2;

    uiConsole_replaceLastMessageFormat("%s%s", ultimaStrings[98], ultimaStrings[925]);

    int tile = dungeonMap[player.px][player.py];
    if (tile != 7 && tile != 8 && tile != 9) {
      uiConsole_queueMessage(ultimaStrings[926]);
      return true;
    }

    if (player.dy == 0 && tile != 9) {
      uiConsole_queueMessage(ultimaStrings[927]);
      return true;
    }

    if (tile == 8) {
      uiConsole_queueMessageFormat("%s%d", ultimaStrings[928], player.dungeonDepth - 1);
      player.dungeonDepth--;
      if (player.dungeonDepth < 1) {
        hp *= 2;
        uiConsole_queueMessageFormat("%s%d%s", ultimaStrings[929], hp, ultimaStrings[930]);
        uiConsole_queueMessage("");
        player.health += hp;
        uiConsole_updateStats();
        vmExecuter_createSceneTransition(1, &sceneOverworld);
        return true;
      }

      sceneDungeon_generateFloor();
    } else {
      uiConsole_queueMessageFormat("%s%d", ultimaStrings[934], player.dungeonDepth + 1);
      player.dungeonDepth++;
      sceneDungeon_generateFloor();
    }


    vmExecuter_createWait(1);
    return true;
  }

  return false;
}

static bool playerDungeon_updateOpen() {
  if (input.o == 1) {
    input.o = 2;
    uiConsole_replaceLastMessageFormat("%s%s", ultimaStrings[98], ultimaStrings[940]);

    int tile = dungeonMap[player.px][player.py];
    if (tile != 6) {
      uiConsole_queueMessage(ultimaStrings[941]);
      return true;
    }

    int frontTile = dungeonMap[player.px + player.dx][player.py + player.dy];
    if (rand01() < 0.4f || frontTile != 0 || rand01() > 0.8f) {
      dungeonMap[player.px][player.py] = 0;
      int gold = (int)(rand01() * pow(player.dungeonDepth, 2) * 9 + 9);
      player.gold += gold;
      uiConsole_queueMessageFormat("%s%d%s", ultimaStrings[916], gold, ultimaStrings[917]);
      uiConsole_updateStats();
      return true;
    }

    // TODO: Spawn a monster

    return true;
  }

  return false;
}

static bool playerDungeon_updateSave() {
  if (input.q == 1) {
    input.q = 2;
    uiConsole_replaceLastMessageFormat("%s%s", ultimaStrings[98], ultimaStrings[944]);
    uiConsole_queueMessage(ultimaStrings[945]);
    uiConsole_queueMessage(ultimaStrings[946]);
    return true;
  }

  return false;
}

static bool playerDungeon_updateAutoPass(float deltaTime) {
  waitingTime += deltaTime;
  if (waitingTime >= 5.0f) {
    waitingTime = 0.0f;
    uiConsole_replaceLastMessageFormat("%s%s", ultimaStrings[98], ultimaStrings[99]);
    player_waitPenalty();
    return true;
  }

  return false;
}

static bool playerDungeon_updateUnlock() {
  if (input.u == 1) {
    input.u = 2;
    uiConsole_replaceLastMessageFormat("%s%s", ultimaStrings[98], ultimaStrings[973]);

    int tile = dungeonMap[player.px][player.py];
    if (tile != 5) {
      uiConsole_queueMessage(ultimaStrings[974]);
      return true;
    }

    if (player.type != 4 && rand01() < 0.5f - (float) player.agility / 100.0f) {
      uiConsole_queueMessage(ultimaStrings[975]);
      player.health -= player.dungeonDepth;
      uiConsole_updateStats();
      return true;
    }

    dungeonMap[player.px][player.py] = 0;
    int gold = (int)(rand01() * pow(player.dungeonDepth, 2) * 9 + 9);
    player.gold += gold;
    uiConsole_queueMessageFormat("%s%d%s", ultimaStrings[916], gold, ultimaStrings[917]);
    uiConsole_updateStats();

    return true;
  }

  return false;
}

bool playerDungeon_update(float deltaTime) {
  bool acted = false;

  if (keyRepeatDelay <= 0) {
    switch (playerState) {
      case PLAYER_STATE_IDLE:
        if (playerDungeon_updateBoard()) { acted = true; } else
        if (playerDungeon_updateDrop()) { acted = true; } else
        if (playerDungeon_updateEnter()) { acted = true; } else
        if (playerDungeon_updateFire()) { acted = true; } else
        if (playerDungeon_updateGet()) { acted = true; } else
        if (playerDungeon_updateInform()) { acted = true; } else
        if (playerDungeon_updateKlimb()) { acted = true; } else
        if (playerDungeon_updateOpen()) { acted = true; } else
        if (playerDungeon_updateSave()) { acted = true; } else
        if (playerCommons_updateReady()) { acted = true; } else
        if (playerCommons_updateWait()) { acted = true; } else
        if (playerCommons_updateZtats()) { acted = true; } else
        if (playerDungeon_updateUnlock()) { acted = true; } else
        if (playerDungeon_updateRotation()) { acted = true; } else
        if (playerDungeon_updateMovement()) { acted = true; } else 
        if (playerDungeon_updateAutoPass(deltaTime)) { acted = true; }
        break;
      
      case PLAYER_STATE_READY_TYPE:
        if (playerCommons_updateReady()) { acted = true; } else
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
    waitingTime = 0.0f;
  }

  return acted;
}

void playerDungeon_free() {

}
