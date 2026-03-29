#include <math.h>
#include "engine/engine.h"
#include "merchantTown.h"
#include "data/player.h"
#include "data/bevery.h"
#include "engine/input.h"
#include "scenes/sceneDiskLoader.h"
#include "scenes/sceneOverworld.h"
#include "entities/ui/uiConsole.h"
#include "playerCommons.h"
#include "vehicleOverworld.h"
#include "vmExecuter.h"
#include "worldMap.h"

static TRANSACT_STEP transactStep = TRANSACT_STEP_START;
static MERCHANT_TYPE merchantType = MERCHANT_TYPE_TRANSPORT;
static int wx = 0;
static int wy = 0;
static int lx = 0;
static int ly = 0;

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
    transactStep = input.b == 1 ? TRANSACT_STEP_BUY_ITEM : TRANSACT_STEP_SELL_ITEM;

    if (player.px > 3 && player.px < 10 && player.py > 3 && player.py < 8) {
      uiConsole_queueMessage(ultimaStrings[425]);
      uiConsole_queueMessage(ultimaStrings[426]);
      uiConsole_queueMessage("");
      vmExecuter_createWait(1.0f);
      merchantType = MERCHANT_TYPE_TRANSPORT;
    }

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
  }

  return false;
}

static bool merchantTown_updateTransactSellItem() {
  if (merchantType == MERCHANT_TYPE_TRANSPORT) {
    uiConsole_queueMessage(ultimaStrings[568]);
    uiConsole_queueMessage(ultimaStrings[569]);

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
  }

  return false;
}