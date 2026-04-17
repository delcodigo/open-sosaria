#include "playerDungeon.h"
#include "data/player.h"
#include "engine/input.h"

void playerDungeon_init() {
}

static bool playerDungeon_updateRotation() {
  if (input.left) {
    int dx = player.dx;
    player.dx = player.dy;
    player.dy = -dx;
    keyRepeatDelay = 0.2f;
    return true;
  } else if (input.right) {
    int dx = player.dx;
    player.dx = -player.dy;
    player.dy = dx;
    keyRepeatDelay = 0.2f;
    return true;
  }

  return false;
}

static bool playerDungeon_updateMovement() {
  if (input.up) {
    player.px += player.dx;
    player.py += player.dy;
    keyRepeatDelay = 0.2f;
    return true;
  } else if (input.down) {
    player.px -= player.dx;
    player.py -= player.dy;
    keyRepeatDelay = 0.2f;
    return true;
  }

  return false;
}

bool playerDungeon_update(float deltaTime) {
  bool acted = false;

  if (keyRepeatDelay <= 0) {
    if (playerDungeon_updateRotation()) { acted = true; } else 
    if (playerDungeon_updateMovement()) { acted = true; }
  } else {
    keyRepeatDelay -= deltaTime;
    if (keyRepeatDelay < 0) {
      keyRepeatDelay = 0;
    }
  }

  return acted;
}

void playerDungeon_free() {

}
