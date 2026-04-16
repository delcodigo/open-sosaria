#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "sceneDungeon.h"
#include "entities/ui/uiztats.h"
#include "entities/ui/uiConsole.h"
#include "entities/dungeonRenderer.h"
#include "entities/playerDungeon.h"
#include "data/player.h"
#include "sceneDiskLoader.h"
#include "config.h"
#include "utils.h"

int dungeonMap[OS_DUNGEON_MAP_WIDTH][OS_DUNGEON_MAP_HEIGHT] = {0};

static void sceneDungeon_printDungeon() {
  printf("\n");
  for (int x=0;x<OS_DUNGEON_MAP_WIDTH;x++) {
    for (int y=0;y<OS_DUNGEON_MAP_HEIGHT;y++) {
      printf("%d ", dungeonMap[x][y]);
    }
    printf("\n");
  }
}

static void sceneDungeon_generateFloor() {
  int tx = (int)(player.tx % OS_BTERRA_MAP_WIDTH);
  int ty = (int)(player.ty % OS_BTERRA_MAP_HEIGHT);
  int world = ((int)player.ty / OS_BTERRA_MAP_HEIGHT) * 2 + ((int)player.tx / OS_BTERRA_MAP_WIDTH);

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

  sceneDungeon_printDungeon();
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

static void sceneDungeon_update(float deltaTime) {
  if (ztatsActive) {
    uiZtats_update(deltaTime);
  }

  if (playerDungeon_update(deltaTime)) {
    dungeonRenderer_update();
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