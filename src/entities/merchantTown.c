#include <math.h>
#include <stdio.h>
#include <string.h>
#include "engine/engine.h"
#include "merchantTown.h"
#include "data/player.h"
#include "data/bevery.h"
#include "engine/input.h"
#include "scenes/sceneDiskLoader.h"
#include "scenes/sceneOverworld.h"
#include "scenes/sceneTown.h"
#include "entities/ui/uiConsole.h"
#include "playerCommons.h"
#include "vehicleOverworld.h"
#include "vmExecuter.h"
#include "worldMap.h"
#include "utils.h"

static TRANSACT_STEP transactStep = TRANSACT_STEP_START;
static MERCHANT_TYPE merchantType = MERCHANT_TYPE_TRANSPORT;
static int wx = 0;
static int wy = 0;
static int lx = 0;
static int ly = 0;
static bool buyingLadder = false;
static int selectedItemId = -1;
static char selectedWeaponName[3] = { 0 };
int drunkLevel = 0;

static void merchantTown_endTransact() {
  transactStep = TRANSACT_STEP_START;
  playerState = PLAYER_STATE_IDLE;
}

static bool merchantTown_updateTransactStart() {
  if (input.t == 1) {
    input.t = 2;
    
    bool foundMerchant = false;
    for (int dy=-1; dy<=1; dy++) {
      for (int dx=-1; dx<=1; dx++) {
        int tile = ultimaAssets.townCollisionMap[player.py + dy][player.px + dx];
        if (tile > 2 && tile < 9) {
          foundMerchant = true;
          dy = 2;
          break;
        }
      }
    }

    if (!foundMerchant) {
      uiConsole_replaceLastMessageFormat("%.14s%.15s", ultimaStrings[98], ultimaStrings[416]);
      uiConsole_queueMessage(ultimaStrings[417]);
      return true;
    }

    if (enemyEncounter.monsterId > 0) {
      uiConsole_replaceLastMessageFormat("%.14s%.15s", ultimaStrings[98], ultimaStrings[418]);
      uiConsole_queueMessage(ultimaStrings[419]);
      return true;
    }

    uiConsole_replaceLastMessageFormat("%.14s%.15s", ultimaStrings[98], ultimaStrings[420]);
    uiConsole_queueMessage(ultimaStrings[421]);

    transactStep = TRANSACT_STEP_SELECT_TRANSACTION;
    playerState = PLAYER_STATE_TRANSACT;

    return false;
  }

  return false;
}

static bool merchantTown_updateTransactSelectTransaction() {
  if (input.b == 1 || input.s == 1) {
    if (input.b == 1) {
      uiConsole_replaceLastMessageFormat("%s BUY", ultimaStrings[421]);
      transactStep = TRANSACT_STEP_BUY_ITEM;
    } else {
      uiConsole_replaceLastMessageFormat("%s SELL", ultimaStrings[421]);
      transactStep = TRANSACT_STEP_SELL_ITEM;
    }

    if (player.px > 3 && player.px < 10 && player.py > 3 && player.py < 8) {
      uiConsole_queueMessage(ultimaStrings[425]);
      uiConsole_queueMessage(ultimaStrings[426]);
      merchantType = MERCHANT_TYPE_TRANSPORT;
    } else if (player.px > 21 && player.px < 26 && player.py > 0 && player.py < 7) {
      uiConsole_queueMessage(ultimaStrings[427]);
      uiConsole_queueMessage(ultimaStrings[428]);
      merchantType = MERCHANT_TYPE_MAGIC;
    } else if (player.px > 29 && player.px < 36 && player.py > 3 && player.py < 8) {
      uiConsole_queueMessage(ultimaStrings[429]);
      merchantType = MERCHANT_TYPE_PUB;
    } else if (player.px > 3 && player.px < 10 && player.py > 13 && player.py < 18) {
      uiConsole_queueMessage(ultimaStrings[430]);
      uiConsole_queueMessage(ultimaStrings[431]);
      merchantType = MERCHANT_TYPE_ARMORY;
    } else if (player.px > 10 && player.px < 17 && player.py > 13 && player.py < 18) {
      uiConsole_queueMessage(ultimaStrings[432]);
      uiConsole_queueMessage(ultimaStrings[433]);
      merchantType = MERCHANT_TYPE_WEAPONS;
    } else if (player.px > 21 && player.px < 27 && player.py > 11 && player.py < 18) {
      uiConsole_queueMessage(ultimaStrings[434]);
      uiConsole_queueMessage(ultimaStrings[435]);
      merchantType = MERCHANT_TYPE_FOOD;
    }

    memset(selectedWeaponName, 0, sizeof(selectedWeaponName));
    lastKey = 0;
    uiConsole_queueMessage("");
    vmExecuter_createWait(1.0f);

    return false;
  } else if (lastKey != 0 && lastKey != GLFW_KEY_T) {
    uiConsole_queueMessage(ultimaStrings[424]);
    merchantTown_endTransact();
    return true;
  }

  return false;
}

static int merchantTown_getTransportCost(int vehicleId) {
  return (int)((200 - player.intelligence) / 80.0f * pow(vehicleId * 4, 2));
}

static int merchantTown_getSpellCost(int spellId) {
  return (int)((200 - player.wisdom) / 200.0f * spellId * 5);
}

static int merchantTown_getArmorCost(int armorId) {
  return (int)((200 - player.intelligence) / 200.0f * 50 * armorId);
}

static int merchantTown_getWeaponCost(int weaponId) {
  return (int)((200 - player.intelligence) / 200.0f * pow(weaponId, 2) + 5);
}

static int merchantTown_getWeaponTime() {
  int tx = (int)(player.tx % OS_BTERRA_MAP_WIDTH);
  int ty = (int)(player.ty % OS_BTERRA_MAP_HEIGHT);
  int world = ((int)player.ty / OS_BTERRA_MAP_HEIGHT) * 2 + ((int)player.tx / OS_BTERRA_MAP_WIDTH);
  int tileType = ultimaAssets.bterraMaps[world][ty][tx] & 0x0F;

  int time = (int)((player.time / 3000.0f) * 2 + 1);
  if (time > 5) { time = 5; }
  if (tileType % 2 == 0) { time += 1; }

  return time;
}

static int merchantTown_getFoodCost() {
  return (int)(5 - player.intelligence / 20.0f);
}

static bool merchantTown_updateTransactBuyItem() {
  if (merchantType == MERCHANT_TYPE_TRANSPORT) {
    wx = 0; wy = 0; lx = 0; ly = 0;
    for (int dy=-1; dy<=1; dy++) {
      for (int dx=-1; dx<=1; dx++) {
        int tile = (worldMap_getTileAt(player.tx + dx, player.ty + dy) >> 4) & 0x0F;
        int vehicleTile = vehiclesMap[player.ty + dy][player.tx + dx];

        if (tile == 0 && vehicleTile == 0 && (dx == 0 || dy == 0)) {
          wx = player.tx + dx;
          wy = player.ty + dy;
        } 
        
        if (tile == 1 && vehicleTile == 0 && (dx == 0 || dy == 0)) {
          lx = player.tx + dx;
          ly = player.ty + dy;
        }
      }
    }

    uiConsole_queueMessageFormat("%.12s%d%.12s%d", ultimaStrings[570], merchantTown_getTransportCost(1), ultimaStrings[571], merchantTown_getTransportCost(2));

    if (wx != 0 || wy != 0) {
      uiConsole_queueMessageFormat("%.12s%d%.12s%d", ultimaStrings[572], merchantTown_getTransportCost(3), ultimaStrings[573], merchantTown_getTransportCost(4));
    }

    if ((lx != 0 || ly != 0) && player.time > 3000) {
      uiConsole_queueMessageFormat("%.12s%d%.12s%d", ultimaStrings[574], merchantTown_getTransportCost(6), ultimaStrings[575], merchantTown_getTransportCost(7));
    }

    uiConsole_queueMessage(ultimaStrings[576]);

    transactStep = TRANSACT_STEP_SELECT_ITEM;
  } else if (merchantType == MERCHANT_TYPE_MAGIC) {
    uiConsole_queueMessageFormat("%.8s%d%.8s%d%.8s%d", ultimaStrings[481], merchantTown_getSpellCost(1), ultimaStrings[482], merchantTown_getSpellCost(2), ultimaStrings[483], merchantTown_getSpellCost(3));
    uiConsole_queueMessageFormat("%.8s%d%.8s%d%.8s%d", ultimaStrings[484], merchantTown_getSpellCost(4), ultimaStrings[485], merchantTown_getSpellCost(5), ultimaStrings[486], merchantTown_getSpellCost(6));
    uiConsole_queueMessageFormat("%.8s%d%.8s%d%.8s%d", ultimaStrings[487], merchantTown_getSpellCost(7), ultimaStrings[488], merchantTown_getSpellCost(8), ultimaStrings[489], merchantTown_getSpellCost(9));

    char consoleMessage1[31] = {0};
    char consoleMessage2[31] = {0};
    snprintf(consoleMessage1, 30, "%.8s%d", ultimaStrings[490], merchantTown_getSpellCost(10));
    snprintf(consoleMessage2, 30, "%.8s%d%.12s", ultimaStrings[490], merchantTown_getSpellCost(10), ultimaStrings[491]);
    
    vmExecuter_queueAndReplaceConsoleMessage(consoleMessage1, consoleMessage2, 1.0f);

    transactStep = TRANSACT_STEP_SELECT_ITEM;
  } else if (merchantType == MERCHANT_TYPE_PUB) {
    if (player.gold < 1) {
      uiConsole_queueMessage(ultimaStrings[503]);
      merchantTown_endTransact();
      return true;
    }

    player.gold -= 1;
    uiConsole_updateStats();

    uiConsole_queueMessage(ultimaStrings[504]);
    uiConsole_queueMessage(ultimaStrings[505]);
    vmExecuter_createWait(1.0f);

    transactStep = TRANSACT_STEP_DRINK_AT_PUB;
  } else if (merchantType == MERCHANT_TYPE_ARMORY) {
    uiConsole_queueMessageFormat("%s%d%s%d", ultimaStrings[547], merchantTown_getArmorCost(1), ultimaStrings[548], merchantTown_getArmorCost(2));
    uiConsole_queueMessageFormat("%s%d", ultimaStrings[549], merchantTown_getArmorCost(3));

    if (player.time > 2000) {
      uiConsole_queueMessageFormat("%s%d%s%d", ultimaStrings[550], merchantTown_getArmorCost(4), ultimaStrings[551], merchantTown_getArmorCost(5));
    }

    uiConsole_queueMessage(ultimaStrings[552]);

    transactStep = TRANSACT_STEP_SELECT_ITEM;
  } else if (merchantType == MERCHANT_TYPE_WEAPONS) {
    int time = merchantTown_getWeaponTime();

    if (time == 1 || time == 3 || time == 5) {
      uiConsole_queueMessageFormat("%s%d%s%d%s%d", ultimaStrings[447], merchantTown_getWeaponCost(1), ultimaStrings[448], merchantTown_getWeaponCost(2), ultimaStrings[449], merchantTown_getWeaponCost(3));
    }
    if (time == 2 || time == 4 || time == 6) {
      uiConsole_queueMessageFormat("%s%d%s%d", ultimaStrings[450], merchantTown_getWeaponCost(4), ultimaStrings[451], merchantTown_getWeaponCost(5));
    }
    if (time == 3 || time == 5) {
      uiConsole_queueMessageFormat("%s%d%s%d", ultimaStrings[452], merchantTown_getWeaponCost(6), ultimaStrings[453], merchantTown_getWeaponCost(7));
    }
    if (time == 4 || time == 6) {
      uiConsole_queueMessageFormat("%s%d%s%d%s%d", ultimaStrings[454], merchantTown_getWeaponCost(8), ultimaStrings[455], merchantTown_getWeaponCost(9), ultimaStrings[456], merchantTown_getWeaponCost(10));
    }
    if (time == 5) {
      uiConsole_queueMessageFormat("%s%d%s%d%s%d", ultimaStrings[457], merchantTown_getWeaponCost(11), ultimaStrings[458], merchantTown_getWeaponCost(12), ultimaStrings[459], merchantTown_getWeaponCost(13));
    }
    if (time == 6) {
      uiConsole_queueMessageFormat("%s%d%s%d", ultimaStrings[460], merchantTown_getWeaponCost(14), ultimaStrings[461], merchantTown_getWeaponCost(15));
    }

    uiConsole_queueMessage(ultimaStrings[462]);

    memset(selectedWeaponName, 0, sizeof(selectedWeaponName));
    lastKey = 0;
    transactStep = TRANSACT_STEP_SELECT_ITEM;
  } else if (merchantType == MERCHANT_TYPE_FOOD) {
    uiConsole_queueMessage(ultimaStrings[584]);
    uiConsole_queueMessageFormat("%s%d", ultimaStrings[585], merchantTown_getFoodCost());
    uiConsole_queueMessage(ultimaStrings[586]);

    lastKey = 0;
    transactStep = TRANSACT_STEP_SELECT_ITEM;
  }

  return false;
}

static bool merchantTown_updateTransactSellItem() {
  if (merchantType == MERCHANT_TYPE_TRANSPORT) {
    uiConsole_queueMessage(ultimaStrings[568]);
    uiConsole_queueMessage(ultimaStrings[569]);

    merchantTown_endTransact();

    return true;
  } else if (merchantType == MERCHANT_TYPE_MAGIC) {
    uiConsole_queueMessage(ultimaStrings[480]);

    merchantTown_endTransact();

    return true;
  } else if (merchantType == MERCHANT_TYPE_PUB) {
    uiConsole_queueMessage(ultimaStrings[502]);

    merchantTown_endTransact();

    return true;
  } else if (merchantType == MERCHANT_TYPE_ARMORY) {
    uiConsole_queueMessage(ultimaStrings[558]);

    lastKey = 0;
    transactStep = TRANSACT_STEP_SELECT_SELL_ITEM;
  } else if (merchantType == MERCHANT_TYPE_WEAPONS) {
    uiConsole_queueMessage(ultimaStrings[470]);

    lastKey = 0;
    transactStep = TRANSACT_STEP_SELECT_SELL_ITEM;
  } else if (merchantType == MERCHANT_TYPE_FOOD) {
    uiConsole_queueMessage(ultimaStrings[583]);

    merchantTown_endTransact();

    return true;
  }

  return false;
}

static bool merchantTown_updateTransactSelectItem() {
  if (merchantType == MERCHANT_TYPE_TRANSPORT) {
    if (lastKey == 0 || lastKey < GLFW_KEY_0 || lastKey > GLFW_KEY_9) {
      return false;
    }

    int keyNumber = lastKey - GLFW_KEY_0;
    uiConsole_replaceLastMessageFormat("%s%d", ultimaStrings[576], keyNumber);

    if ((keyNumber > 2 && keyNumber < 5 && wx == 0 && wy == 0) || (player.time < 3000 && keyNumber > 4 && keyNumber < 7)) {
      uiConsole_queueMessage(ultimaStrings[577]);
      merchantTown_endTransact();
      return true;
    }
    
    if ((keyNumber < 1 || keyNumber > 6)) {
      uiConsole_queueMessageFormat("%d%s", keyNumber, ultimaStrings[578]);
      merchantTown_endTransact();
      return true;
    }

    if (player.gold < merchantTown_getTransportCost(keyNumber)) {
      uiConsole_queueMessage(ultimaStrings[579]);
      merchantTown_endTransact();
      return true;
    }

    int dx = lx;
    int dy = ly;
    if (keyNumber > 2 && keyNumber < 5) {
      dx = wx;
      dy = wy;
    }

    if (dx == 0 && dy == 0) {
      uiConsole_queueMessage(ultimaStrings[580]);
      merchantTown_endTransact();
      return true;
    }

    player.gold -= merchantTown_getTransportCost(keyNumber);
    player.vehicles[keyNumber - 1] += 1;
    vehiclesMap[dy][dx] = (unsigned char) keyNumber;

    uiConsole_queueMessageFormat("%s%s", vehicleNames[keyNumber], ultimaStrings[581]);
    merchantTown_endTransact();
    return true;
  } else if (merchantType == MERCHANT_TYPE_MAGIC) {
    if (lastKey == 0 || lastKey < GLFW_KEY_A || lastKey > GLFW_KEY_Z) {
      return false;
    }

    int selectedSpellId = -1;

    if (buyingLadder) {
      buyingLadder = false;
      if (lastKey == GLFW_KEY_U) {
        selectedSpellId = 6;
      } else if (lastKey == GLFW_KEY_D) {
        selectedSpellId = 5;
      } else {
        uiConsole_queueMessageFormat("%s%c", ultimaStrings[500], (char) lastKey);
        merchantTown_endTransact();
        return true;
      }
    } else {
      uiConsole_replaceLastMessageFormat("%.8s%d%.12s%c", ultimaStrings[490], merchantTown_getSpellCost(10), ultimaStrings[491], (char) lastKey);
      if (lastKey == GLFW_KEY_L) {
        uiConsole_queueMessage(ultimaStrings[493]);
        buyingLadder = true;
        lastKey = 0;
        return false;
      } else {
        if (lastKey == GLFW_KEY_M) {
          lastKey = GLFW_KEY_P;
        }

        for (int i=1;i<=10;i++) {
          if (spellNames[i][0] == (char) lastKey) {
            selectedSpellId = i;
            break;
          }
        }
      }
    }

    if (selectedSpellId == -1) {
      uiConsole_queueMessageFormat("%s%c", ultimaStrings[500], (char) lastKey);
      merchantTown_endTransact();
      return true;
    }

    int spellCost = merchantTown_getSpellCost(selectedSpellId);
    if (player.gold < spellCost || player.experience - spellCost < 0) {
      uiConsole_queueMessage(ultimaStrings[495]);
      merchantTown_endTransact();
      return true;
    }

    if (player.type != 3 && selectedSpellId > 6) {
      uiConsole_queueMessage(ultimaStrings[496]);
      merchantTown_endTransact();
      return true;
    }

    player.gold -= spellCost;
    player.experience -= spellCost;
    player.spells[selectedSpellId] += 1;

    uiConsole_queueMessageFormat("%s%s", spellNames[selectedSpellId], ultimaStrings[497]);
    merchantTown_endTransact();
    return true;
  } else if (merchantType == MERCHANT_TYPE_ARMORY) {
    if (lastKey != 0) {
      int selectedArmorId = -1;
      for (int i=1;i<OS_ARMORS_COUNT;i++) {
        if (lastKey == armorNames[i][0]) {
          selectedArmorId = i;
          break;
        }
      }

      if (selectedArmorId == -1) {
        uiConsole_queueMessageFormat("%c%s", (char) lastKey, ultimaStrings[553]);
        merchantTown_endTransact();
        return true;
      }

      if (selectedArmorId > 3 && player.time < 2000) {
        uiConsole_queueMessageFormat("%s%s", armorNames[selectedArmorId], ultimaStrings[554]);
        merchantTown_endTransact();
        return true;
      }

      if (player.gold < merchantTown_getArmorCost(selectedArmorId)) {
        uiConsole_queueMessage(ultimaStrings[555]);
        merchantTown_endTransact();
        return true;
      }

      if (player_getEncumbrance() > 0) {
        uiConsole_queueMessage(ultimaStrings[556]);
        merchantTown_endTransact();
        return true;
      }

      player.gold -= merchantTown_getArmorCost(selectedArmorId);
      player.armors[selectedArmorId - 1] += 1;

      uiConsole_queueMessageFormat("%s%s", armorNames[selectedArmorId], ultimaStrings[557]);
      uiConsole_updateStats();
      merchantTown_endTransact();

      return true;
    }
  } else if (merchantType == MERCHANT_TYPE_WEAPONS) {
    if (lastKey != 0 && lastKey >= GLFW_KEY_A && lastKey <= GLFW_KEY_Z) {
      if (selectedWeaponName[0] == 0) {
        selectedWeaponName[0] = (char) lastKey;
        lastKey = 0;
        uiConsole_replaceLastMessageFormat("%s%c", ultimaStrings[462], selectedWeaponName[0]);
        return false;
      } else if (selectedWeaponName[1] == 0) {
        selectedWeaponName[1] = (char) lastKey;
        lastKey = 0;
        uiConsole_replaceLastMessageFormat("%s%s", ultimaStrings[462], selectedWeaponName);

        return false;
      }
    }

    if (selectedWeaponName[0] != 0 && selectedWeaponName[1] != 0) {
      int selectedWeaponId = -1;
      for (int i=1;i<=OS_WEAPONS_COUNT;i++) {
        if (weaponNames[i][0] == selectedWeaponName[0] && weaponNames[i][1] == selectedWeaponName[1]) {
          selectedWeaponId = i;
          break;
        }
      }

      if (selectedWeaponId == -1) {
        uiConsole_queueMessageFormat("%s%s", selectedWeaponName, ultimaStrings[463]);
        merchantTown_endTransact();
        return true;
      }

      int time = merchantTown_getWeaponTime();
      if ((int)((time + 1) / 2.0f) * 5 < selectedWeaponId) {
        uiConsole_queueMessageFormat("%s%s", weaponNames[selectedWeaponId], ultimaStrings[464]);
        merchantTown_endTransact();
        return true;
      } 
      if (time % 2 != 0 && selectedWeaponId % 2 == 0) {
        uiConsole_queueMessage(ultimaStrings[465]);
        uiConsole_queueMessage(weaponNames[selectedWeaponId]);
        merchantTown_endTransact();
        return true;
      } 
      if (time % 2 == 0 && selectedWeaponId % 2 != 0) {
        uiConsole_queueMessage(ultimaStrings[466]);
        uiConsole_queueMessage(weaponNames[selectedWeaponId]);
        merchantTown_endTransact();
        return true;
      }

      if (player.gold < merchantTown_getWeaponCost(selectedWeaponId)) {
        uiConsole_queueMessage(ultimaStrings[467]);
        merchantTown_endTransact();
        return true;
      }

      if (player_getEncumbrance() > 0) {
        uiConsole_queueMessage(ultimaStrings[468]);
        merchantTown_endTransact();
        return true;
      }

      player.gold -= merchantTown_getWeaponCost(selectedWeaponId);
      player.weapons[selectedWeaponId - 1] += 1;

      uiConsole_queueMessageFormat("%s%s", weaponNames[selectedWeaponId], ultimaStrings[469]);
      uiConsole_updateStats();
      merchantTown_endTransact();

      return true;
    }
  } else if (merchantType == MERCHANT_TYPE_FOOD && lastKey != 0) {
    if (lastKey < GLFW_KEY_0 || lastKey > GLFW_KEY_9) {
      return false;
    }

    uiConsole_replaceLastMessageFormat("%s%c", ultimaStrings[586], (char) lastKey);

    if (lastKey == GLFW_KEY_0) {
      uiConsole_queueMessage(ultimaStrings[587]);
      merchantTown_endTransact();
      return true;
    }

    if (player.gold < merchantTown_getFoodCost()) {
      uiConsole_queueMessage(ultimaStrings[588]);
      uiConsole_queueMessage(ultimaStrings[589]);
      merchantTown_endTransact();
      return true;
    }

    player.food += (int)(lastKey - GLFW_KEY_0) * 10;
    player.gold -= merchantTown_getFoodCost();
    uiConsole_updateStats();
    merchantTown_endTransact();
    return true;
  }

  return false;
}

static bool merchantTown_updateDrinkAtPub() {
  drunkLevel += 1;

  if (drunkLevel > player.stamina / 5 || drunkLevel > player.wisdom / 5) {
    float wenchRadius = sqrt(pow(player.px - wenchPosition.x, 2) + pow(player.py - wenchPosition.y, 2));
    if (wenchRadius <= 1.5f) {
      uiConsole_queueMessageFormat("^T1%s", player.name);
      uiConsole_queueMessageFormat("^T1%s", ultimaStrings[543]);
      
      player.gold = 0;
      player.wisdom -= 1;
      if (player.wisdom < 5) { player.wisdom = 5; }
      uiConsole_updateStats();

      uiConsole_queueMessageFormat("^T1%s", ultimaStrings[544]);
      uiConsole_queueMessageFormat("^T1%s", ultimaStrings[545]);

      merchantTown_endTransact();
      return true;
    }
  }

  if (rand01() > 0.7f) {
    merchantTown_endTransact();
    return true;
  }

  uiConsole_queueMessage(ultimaStrings[506]);

  int gossip = (int)(rand01() * 10 + 1);

  switch (gossip) {
    case 1:
      for (int i=0;i<5;i++) {
        uiConsole_queueMessageFormat("^T1%s", ultimaStrings[507 + i]);
      }
      break;
    case 2:
      uiConsole_queueMessageFormat("^T1%s", ultimaStrings[512]);
      break;
    case 3:
      for (int i=0;i<5;i++) {
        uiConsole_queueMessageFormat("^T1%s", ultimaStrings[513 + i]);
      }
      break;
    case 4:
    case 5:
      uiConsole_queueMessageFormat("^T1%s", ultimaStrings[518]);
      break;
    case 6:
      uiConsole_queueMessageFormat("^T1%s", ultimaStrings[519]);
      uiConsole_queueMessageFormat("^T1%s", ultimaStrings[520]);
      break;
    case 7:
      for (int i=0;i<3;i++) {
        uiConsole_queueMessageFormat("^T1%s", ultimaStrings[521 + i]);
      }
      break;
    case 8:
    case 9:
      uiConsole_queueMessageFormat("^T1%s", ultimaStrings[524]);
      break;
    case 10:
      for (int i=0;i<17;i++) {
        uiConsole_queueMessageFormat("^T1%s", ultimaStrings[525 + i]);
      }
      break;
  }

  vmExecuter_createWait(1.0f);
  merchantTown_endTransact();
  return true;
}

bool merchantTown_updateSelectSellItem() {
  if (merchantType == MERCHANT_TYPE_ARMORY) {
    if (lastKey != 0) {
      for (int i=1;i<OS_ARMORS_COUNT;i++) {
        if (lastKey == armorNames[i][0]) {
          selectedItemId = i;
          break;
        }
      }

      uiConsole_replaceLastMessageFormat("%s%c", ultimaStrings[558], (char) lastKey);

      if (selectedItemId == -1) {
        uiConsole_queueMessageFormat("%c%s", (char) lastKey, ultimaStrings[559]);
        merchantTown_endTransact();
        return true;
      }

      int price = (int)((float) player.charisma / 50.0f * merchantTown_getArmorCost(selectedItemId));

      uiConsole_queueMessageFormat("%s%s", ultimaStrings[560], armorNames[selectedItemId]);
      uiConsole_queueMessageFormat("%s%d%s", ultimaStrings[561], price, ultimaStrings[562]);

      lastKey = 0;
      transactStep = TRANSACT_STEP_CONFIRM_SELL_ITEM;
    }
  } else if (merchantType == MERCHANT_TYPE_WEAPONS) {
    if (lastKey != 0 && lastKey >= GLFW_KEY_A && lastKey <= GLFW_KEY_Z) {
      if (selectedWeaponName[0] == 0) {
        selectedWeaponName[0] = (char) lastKey;
        lastKey = 0;
        uiConsole_replaceLastMessageFormat("%s%c", ultimaStrings[470], selectedWeaponName[0]);
        return false;
      } else if (selectedWeaponName[1] == 0) {
        selectedWeaponName[1] = (char) lastKey;
        lastKey = 0;
        uiConsole_replaceLastMessageFormat("%s%s", ultimaStrings[470], selectedWeaponName);
        return false;
      }

      return false;
    }

    if (selectedWeaponName[0] != 0 && selectedWeaponName[1] != 0) {
      int selectedWeaponId = -1;
      for (int i=1;i<=OS_WEAPONS_COUNT;i++) {
        if (weaponNames[i][0] == selectedWeaponName[0] && weaponNames[i][1] == selectedWeaponName[1]) {
          selectedWeaponId = i;
          break;
        }
      }

      if (selectedWeaponId == -1) {
        uiConsole_queueMessageFormat("%s%s", selectedWeaponName, ultimaStrings[471]);
        merchantTown_endTransact();
        return true;
      }

      int price = (int)((float) player.charisma / 50.0f * merchantTown_getWeaponCost(selectedWeaponId));

      uiConsole_queueMessageFormat("%s%s", ultimaStrings[472], weaponNames[selectedWeaponId]);
      uiConsole_queueMessageFormat("%s%d%s", ultimaStrings[473], price, ultimaStrings[562]);

      lastKey = 0;
      selectedItemId = selectedWeaponId;
      transactStep = TRANSACT_STEP_CONFIRM_SELL_ITEM;
    }
  }

  return false;
}

bool merchantTown_updateConfirmSellItem() {
  if (merchantType == MERCHANT_TYPE_ARMORY && lastKey != 0) {
    int price = (int)((float) player.charisma / 50.0f * merchantTown_getArmorCost(selectedItemId));
    if (lastKey == GLFW_KEY_Y) {
      uiConsole_replaceLastMessageFormat("%s%d%s Y", ultimaStrings[561], price, ultimaStrings[562]);

      if (player.armors[selectedItemId - 1] <= 0) {
        uiConsole_queueMessage(ultimaStrings[565]);
        uiConsole_queueMessageFormat("%s%s", armorNames[selectedItemId], ultimaStrings[566]);
        merchantTown_endTransact();
        return true;
      }


      player.gold += price;
      player.armors[selectedItemId - 1] -= 1;


      if (player.armor == selectedItemId - 1 && player.armors[selectedItemId - 1] < 1) {
        player.armor = 0;
      }
    } else {
      uiConsole_replaceLastMessageFormat("%s%d%s N", ultimaStrings[561], price, ultimaStrings[562]);
      uiConsole_queueMessage(ultimaStrings[564]);
    }

    uiConsole_updateStats();

    merchantTown_endTransact();
    return true;
  } else if (merchantType == MERCHANT_TYPE_WEAPONS && lastKey != 0) {
    int price = (int)((float) player.charisma / 50.0f * merchantTown_getWeaponCost(selectedItemId));
    if (lastKey == GLFW_KEY_Y) {
      uiConsole_replaceLastMessageFormat("%s%d%s Y", ultimaStrings[473], price, ultimaStrings[562]);

      if (player.weapons[selectedItemId - 1] <= 0) {
        uiConsole_queueMessage(ultimaStrings[477]);
        uiConsole_queueMessageFormat("%s%s", weaponNames[selectedItemId], ultimaStrings[478]);
        merchantTown_endTransact();
        return true;
      }


      player.gold += price;
      player.weapons[selectedItemId - 1] -= 1;


      if (player.weapon == selectedItemId - 1 && player.weapons[selectedItemId - 1] < 1) {
        player.weapon = 0;
      }

      uiConsole_updateStats();
    } else {
      uiConsole_replaceLastMessageFormat("%s%d%s N", ultimaStrings[473], price, ultimaStrings[562]);
      uiConsole_queueMessage(ultimaStrings[476]);
    }

    merchantTown_endTransact();
    return true;
  }

  return false;
}

bool merchantTown_updateTransact() {
  if (transactStep == TRANSACT_STEP_START) {
    return merchantTown_updateTransactStart();
  } else if (transactStep == TRANSACT_STEP_SELECT_TRANSACTION) {
    return merchantTown_updateTransactSelectTransaction();
  } else if (transactStep == TRANSACT_STEP_BUY_ITEM) {
    return merchantTown_updateTransactBuyItem();
  } else if (transactStep == TRANSACT_STEP_SELL_ITEM) {
    return merchantTown_updateTransactSellItem();
  } else if (transactStep == TRANSACT_STEP_SELECT_ITEM) {
    return merchantTown_updateTransactSelectItem();
  } else if (transactStep == TRANSACT_STEP_DRINK_AT_PUB) {
    return merchantTown_updateDrinkAtPub();
  } else if (transactStep == TRANSACT_STEP_SELECT_SELL_ITEM) {
    return merchantTown_updateSelectSellItem();
  } else if (transactStep == TRANSACT_STEP_CONFIRM_SELL_ITEM) {
    return merchantTown_updateConfirmSellItem();
  }

  return false;
}