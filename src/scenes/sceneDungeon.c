#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "sceneDungeon.h"
#include "entities/ui/uiztats.h"
#include "entities/ui/uiConsole.h"
#include "entities/dungeonRenderer.h"
#include "entities/playerDungeon.h"
#include "entities/vmExecuter.h"
#include "data/player.h"
#include "sceneDiskLoader.h"
#include "config.h"
#include "utils.h"

int dungeonMap[OS_DUNGEON_MAP_WIDTH][OS_DUNGEON_MAP_HEIGHT] = {0};
int monsters[100][4] = {0};
int monstersIndex;

static void sceneDungeon_spawnEnemies() {
  monstersIndex = 20 + ((int)(player.dungeonDepth / 2.0f + 0.5f) - 1) * 5;

  for (int i=monstersIndex;i<monstersIndex+5;i++) {
    monsters[i - 1][0] = 1;
  }

  if (monstersIndex > 40) {
    monstersIndex = 40;
  }

  for (int i=0;i<3;i++) {
    int mn = 0;
    do {
      mn = (int)(rand01() * 5 + 1);
    } while (monsters[monstersIndex + mn - 1][0] == 0);

    monsters[monstersIndex + mn - 1][0] = 0;
    monsters[monstersIndex + mn - 1][3] = (int)(mn * pow(player.dungeonDepth, 2) * rand01() + 10);

    do {
      monsters[monstersIndex + mn - 1][1] = (int)(rand01() * 9 + 1);
      monsters[monstersIndex + mn - 1][2] = (int)(rand01() * 9 + 1);
    } while (dungeonMap[monsters[monstersIndex + mn - 1][1]][monsters[monstersIndex + mn - 1][2]] != 0);

    dungeonMap[monsters[monstersIndex + mn - 1][1]][monsters[monstersIndex + mn - 1][2]] = 100 * mn;
  }
}

void sceneDungeon_generateFloor() {
  int tx = (int)(player.tx % OS_BTERRA_MAP_WIDTH);
  int ty = (int)(player.ty % OS_BTERRA_MAP_HEIGHT);
  int world = ((int)player.ty / OS_BTERRA_MAP_HEIGHT) * 2 + ((int)player.tx / OS_BTERRA_MAP_WIDTH);

  memset(monsters, 0, sizeof(monsters));

  srand(-world - tx * 16 - ty * 256 - player.dungeonDepth * pow(256, 2) - 1);
  
  for (int x=0;x<OS_DUNGEON_MAP_WIDTH;x++) {
    for (int y=0;y<OS_DUNGEON_MAP_HEIGHT;y++) {
      dungeonMap[x][y] = 0;
    }
  }

  for (int x=0;x<OS_DUNGEON_MAP_WIDTH;x++) {
    dungeonMap[x][0] = 1;
    dungeonMap[x][OS_DUNGEON_MAP_HEIGHT - 1] = 1;
    dungeonMap[0][x] = 1;
    dungeonMap[OS_DUNGEON_MAP_WIDTH - 1][x] = 1;
  }

  for (int x=2;x<=8;x+=2) {
    for (int y=1;y<=9;y++) {
      dungeonMap[x][y] = 1;
      dungeonMap[y][x] = 1;
    }
  }

  for (int zz=0;zz<2;zz++) {
    for (int x=2;x<=8;x+=2) {
      for (int y=1;y<=9;y+=2) {
        int zx = (int)(rand01() * 20 + 1);
        int xz = 0;
        if (zx < 3) {
          xz = 0;
        } else if (zx < 9) {
          xz = 3;
        } else if (zx < 19) {
          xz = 4;
        } else if (zx < 20 && player.dungeonDepth < 10) {
          xz = 9;
        } else if (zx == 20 && player.dungeonDepth < 10) {
          xz = 2;
        }

        if (zz == 0) {
          dungeonMap[x][y] = xz;
        } else {
          dungeonMap[y][x] = xz;
        }
      }
    }
  }

  srand(-player.time);
  for (int xx=1;xx<=player.dungeonDepth;xx++) {
    int x = (int)(rand01() * 4 + 1) * 2;
    int y = (int)(rand01() * 4 + 1) * 2;
    
    if (rand01() > 0.5f) {
      y += 1;
    } else {
      x += 1;
    }

    int zx = (int)(rand01() * 3 + 5);
    if (zx == 7) {
      zx = 12;
    }

    dungeonMap[x][y] = zx;
  }

  dungeonMap[1][2] = 0;

  if (player.dungeonDepth % 2 == 0) {
    dungeonMap[7][3] = 7;
    dungeonMap[3][7] = 8;
  } else {
    dungeonMap[7][3] = 8;
    dungeonMap[3][7] = 7;
  }

  if (player.dungeonDepth == 10) {
    dungeonMap[7][3] = 0;
  }

  if (player.dungeonDepth == 1) {
    dungeonMap[1][1] = 8;
    dungeonMap[7][3] = 0;
  }

  sceneDungeon_spawnEnemies();
}

static void sceneDungeon_moveEnemies() {
  for (int i=1;i<=5;i++) {
    if (monsters[monstersIndex + i - 1][0] == 1) { continue; }

    int xx = player.px - monsters[monstersIndex + i - 1][1];
    int yy = player.py - monsters[monstersIndex + i - 1][2];
    int dx = (xx > 0) - (xx < 0);
    int dy = (yy > 0) - (yy < 0);
    float ra = sqrtf(xx*xx + yy*yy);

    if (ra < 1.4) { return; }
    if (ra > 3 && player.px != monsters[monstersIndex + i - 1][1] && player.py != monsters[monstersIndex + i - 1][2]) { continue; }

    if (xx != 0) {
      int zz = dungeonMap[monsters[monstersIndex + i - 1][1] + dx][monsters[monstersIndex + i - 1][2]];
      if (zz != 1 && zz != 12 && zz != 2 && zz < 21) {
        dungeonMap[monsters[monstersIndex + i - 1][1]][monsters[monstersIndex + i - 1][2]] -= 100 * i;
        monsters[monstersIndex + i - 1][1] += dx;
        dungeonMap[monsters[monstersIndex + i - 1][1]][monsters[monstersIndex + i - 1][2]] += 100 * i;
      }
    } else {
      int zz = dungeonMap[monsters[monstersIndex + i - 1][1]][monsters[monstersIndex + i - 1][2] + dy];
      if (zz != 1 && zz != 12 && zz != 2 && zz < 21) {
        dungeonMap[monsters[monstersIndex + i - 1][1]][monsters[monstersIndex + i - 1][2]] -= 100 * i;
        monsters[monstersIndex + i - 1][2] += dy;
        dungeonMap[monsters[monstersIndex + i - 1][1]][monsters[monstersIndex + i - 1][2]] += 100 * i;
      }
    }
  }
}

static void sceneDungeon_init() {
  camera_setPosition3f(&camera, 0.0f, 0.0f, 10.0f);
  sceneDungeon_generateFloor();  
  dungeonRenderer_init();

  player.px = 1;
  player.py = 1;
  player.dx = 0;
  player.dy = 1;
  playerDungeon_init();
  dungeonRenderer_update();
}

bool sceneDungeon_isSolid(int x, int y) {
  if (x < 0 || x >= OS_DUNGEON_MAP_WIDTH || y < 0 || y >= OS_DUNGEON_MAP_HEIGHT) {
    return false;
  }

  int tile = dungeonMap[x][y];
  return tile == 1 || tile > 9;
}

static void sceneDungeon_update(float deltaTime) {
  if (!queuedMessagesCount && lagTime <= 0 && !vmExecuter_update(deltaTime)) {
    if (ztatsActive) {
      uiZtats_update(deltaTime);
      return;
    }

    if (playerActed) {
      playerActed = false;
      
      sceneDungeon_moveEnemies();
      dungeonRenderer_update();
      uiConsole_addMessage(ultimaStrings[98]);
    }

    if (player_isAlive() && playerDungeon_update(deltaTime)) {
      playerActed = true;
    }
  }

  float *viewMatrix = camera_getViewProjectionMatrix(&camera);
  dungeonRenderer_render(viewMatrix);

  uiConsole_update(deltaTime);
}

static void sceneDungeon_free() {
  dungeonRenderer_free();
  playerDungeon_free();
}

Scene sceneDungeon = {
  .scene_init = sceneDungeon_init,
  .scene_update = sceneDungeon_update,
  .scene_free = sceneDungeon_free
};