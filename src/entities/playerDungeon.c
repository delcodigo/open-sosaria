#include <math.h>
#include "entities/ui/uiConsole.h"
#include "entities/vmExecuter.h"
#include "playerDungeon.h"
#include "playerCommons.h"
#include "data/player.h"
#include "data/bevery.h"
#include "data/enemy.h"
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
  if (input.left == 1) {
    input.left = 2;
    int dx = player.dx;
    player.dx = player.dy;
    player.dy = -dx;
    keyRepeatDelay = 0.1f;
    uiConsole_replaceLastMessageFormat("%s%s", ultimaStrings[98], ultimaStrings[866]);
    player_consumeDungeonFood();
    return true;
  } else if (input.right == 1) {
    input.right = 2;
    int dx = player.dx;
    player.dx = -player.dy;
    player.dy = dx;
    keyRepeatDelay = 0.1f;
    uiConsole_replaceLastMessageFormat("%s%s", ultimaStrings[98], ultimaStrings[867]);
    player_consumeDungeonFood();
    return true;
  } else if (input.down == 1) {
    input.down = 2;
    player.dx = -player.dx;
    player.dy = -player.dy;
    keyRepeatDelay = 0.1f;
    uiConsole_replaceLastMessageFormat("%s%s", ultimaStrings[98], ultimaStrings[865]);
    player_consumeDungeonFood();
    return true;
  }

  return false;
}

static bool playerDungeon_updateMovement() {
  if (input.up == 1) {
    input.up = 2;
    uiConsole_replaceLastMessageFormat("%s%s", ultimaStrings[98], ultimaStrings[859]);

    if (sceneDungeon_isSolid(player.px + player.dx, player.py + player.dy)) {
      uiConsole_addMessage(ultimaStrings[860]);
      return false;
    }

    player.px += player.dx;
    player.py += player.dy;
    keyRepeatDelay = 0.1f;

    int tile = dungeonMap[player.px][player.py];
    if (tile == 2) {
      if (player.weapons[3] > 0) {
        player.weapons[3]--;
        uiConsole_queueMessage(ultimaStrings[862]);
        uiConsole_queueMessage(ultimaStrings[863]);
      } else {
        uiConsole_queueMessage(ultimaStrings[864]);
        player.health -= (int)(rand01() * player.dungeonDepth * 10 + player.dungeonDepth);
        uiConsole_queueMessageFormat("%s%d", ultimaStrings[934], player.dungeonDepth + 1);
        player.dungeonDepth++;
        sceneDungeon_generateFloor();
      }

      vmExecuter_createWait(1);
    }

    player_consumeDungeonFood();

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

    int monster = 0;
    int tries = 0;
    do {
      monster = (int)(rand01() * 5 + 1);
    }while (monsters[monstersIndex + monster][0] != 0 && tries++ < 6);

    if (tries >= 6) {
      return true;
    }

    uiConsole_queueMessage(ultimaStrings[942]);
    vmExecuter_createWait(0.6f);
    monsters[monstersIndex + monster][0] = 0;
    monsters[monstersIndex + monster][1] = player.px + player.dx;
    monsters[monstersIndex + monster][2] = player.py + player.dy;
    monsters[monstersIndex + monster][3] = (int)(monster * pow(player.dungeonDepth, 2) * rand01() * 2 + 15);
    dungeonMap[player.px + player.dx][player.py + player.dy] = monster * 100;

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

static bool playerDungeon_updateExit() {
  if (input.x == 1) {
    input.x = 2;
    uiConsole_replaceLastMessageFormat("%s%s", ultimaStrings[98], ultimaStrings[978]);
    return true;
  }

  return false;
}

static bool playerDungeon_updateAttack() {
  if (input.a == 1) {
    input.a = 2;
    uiConsole_replaceLastMessageFormat("%s%s", ultimaStrings[98], ultimaStrings[868]);

    int weapon = player.weapon;
    if (weapon == 4 || (weapon > 7 && weapon < 11)) {
      uiConsole_queueMessageFormat("%s%s", ultimaStrings[869], weaponNames[player.weapon]);
      uiConsole_queueMessage(ultimaStrings[870]);
      return true;
    }

    int range = 1;
    if (weapon == 7 || weapon == 12 || weapon > 13) {
      range = 9;
    }

    int enemy = -1;
    for (int i=1;i<=range;i++) {
      int tile = dungeonMap[player.px + player.dx * i][player.py + player.dy * i];
      if (tile == 1 || tile == 3 || tile == 4 || tile == 12) {
        uiConsole_queueMessage(ultimaStrings[871]);
        return true;
      }

      if (tile > 19) {
        enemy = (int)(tile / 100);
        break;
      }
    }

    if (enemy == -1) {
      uiConsole_queueMessage(ultimaStrings[872]);
      return true;
    }

    uiConsole_queueMessage(enemyDefinitions[monstersIndex + enemy].name);
    uiConsole_queueMessageFormat("%s%s", ultimaStrings[873], weaponNames[player.weapon]);

    int agility = (player.agility / 4 + player.weapon) * rand01();
    int enemyAgility = (enemy + (monstersIndex - 20) / 3) * rand01() + player.dungeonDepth;
    
    if (agility < enemyAgility && agility < 20) {
      uiConsole_queueMessage(ultimaStrings[874]);
      return true;
    }

    int damage = (int)((player.strength / 5 + player.weapon * 3) * rand01() + (int)(player.strength / 5));
    uiConsole_queueMessageFormat("%s%d", ultimaStrings[875], damage);

    monsters[monstersIndex + enemy][3] -= damage;
    if (monsters[monstersIndex + enemy][3] < 0) {
      monsters[monstersIndex + enemy][0] = 1;
      uiConsole_queueMessageFormat("%s%s", enemyDefinitions[monstersIndex + enemy].name, ultimaStrings[876]);
      dungeonMap[monsters[monstersIndex + enemy][1]][monsters[monstersIndex + enemy][2]] -= enemy * 100;

      vmExecuter_createWait(0.6f);

      int monster = 0;
      do {
        monster = (int)(rand01() * 5 + 1);
      } while (monsters[monstersIndex + monster][0] == 0);

      monsters[monstersIndex + monster][0] = 0;
      monsters[monstersIndex + monster][3] = (int)(monster * pow(player.dungeonDepth, 2) * rand01() * 2 + 15);
      
      do {
        monsters[monstersIndex + monster][1] = (int)(rand01() * 9 + 1);
        monsters[monstersIndex + monster][2] = (int)(rand01() * 9 + 1);
      } while (dungeonMap[monsters[monstersIndex + monster][1]][monsters[monstersIndex + monster][2]] != 0 || monsters[monstersIndex + monster][1] == player.px || monsters[monstersIndex + monster][2] == player.py);

      dungeonMap[monsters[monstersIndex + monster][1]][monsters[monstersIndex + monster][2]] = monster * 100;

      int earnedXP = (int)(rand01() * player.dungeonDepth * enemy * 5 + player.dungeonDepth);
      player.experience += earnedXP;
      hp += earnedXP;
      uiConsole_queueMessageFormat("%s%d%s", ultimaStrings[913], earnedXP, ultimaStrings[914]);
      uiConsole_updateStats();

      if (enemy == 5 && monstersIndex > 20) {
        int questIndex = (int)(monstersIndex / 5 - 5);
        if (player.quests[questIndex * 2 + 1] > 0) {
          player.quests[questIndex * 2 + 1] = -player.quests[questIndex * 2 + 1];
          uiConsole_queueMessage(ultimaStrings[915]);
        }
      }

      int earnedGold = (int)(rand01() * pow(player.dungeonDepth, 2) * 9 + 9);
      player.gold += earnedGold;
      uiConsole_queueMessageFormat("%s%d%s", ultimaStrings[916], earnedGold, ultimaStrings[917]);
      uiConsole_updateStats();
    }

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
        if (playerDungeon_updateExit()) { acted = true; } else
        if (playerDungeon_updateAttack()) { acted = true; } else
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
