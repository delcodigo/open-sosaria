#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "sceneDungeon.h"
#include "sceneOverworld.h"
#include "entities/ui/uiztats.h"
#include "entities/ui/uiConsole.h"
#include "entities/dungeonRenderer.h"
#include "entities/playerDungeon.h"
#include "entities/vmExecuter.h"
#include "data/player.h"
#include "data/enemy.h"
#include "data/bevery.h"
#include "sceneDiskLoader.h"
#include "config.h"
#include "utils.h"

int dungeonMap[OS_DUNGEON_MAP_WIDTH][OS_DUNGEON_MAP_HEIGHT] = {0};
int monsters[100][4] = {0};
int monstersIndex;

static void sceneDungeon_spawnEnemies() {
  monstersIndex = 20 + ((int)(player.dungeonDepth / 2.0f + 0.5f) - 1) * 5;

  for (int i=monstersIndex;i<=monstersIndex+5;i++) {
    monsters[i][0] = 1;
  }

  if (monstersIndex > 40) {
    monstersIndex = 40;
  }

  for (int i=0;i<3;i++) {
    int mn = 0;
    do {
      mn = (int)(rand01() * 5 + 1);
    } while (monsters[monstersIndex + mn][0] == 0);

    monsters[monstersIndex + mn][0] = 0;
    monsters[monstersIndex + mn][3] = (int)(mn * pow(player.dungeonDepth, 2) * rand01() + 10);

    do {
      monsters[monstersIndex + mn][1] = (int)(rand01() * 9 + 1);
      monsters[monstersIndex + mn][2] = (int)(rand01() * 9 + 1);
    } while (dungeonMap[monsters[monstersIndex + mn][1]][monsters[monstersIndex + mn][2]] != 0);

    dungeonMap[monsters[monstersIndex + mn][1]][monsters[monstersIndex + mn][2]] = 100 * mn;
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

static bool sceneDungeon_enemySpecialAttack(int monster) {
  int enemy = monstersIndex + monster;
  if (enemy == 23) {
    for (int i=0;i<OS_WEAPONS_COUNT;i++) {
      if (player.weapons[i] > 0 && player.weapon-1 != i) {
        uiConsole_queueMessageFormat("%s%s", ultimaStrings[999], weaponNames[i+1]);
        player.weapons[i] -= 1;
        return true;
      }
    }
  } else if (enemy == 30) {
    if (player.armor == 0) {
      return false;
    }

    uiConsole_queueMessage(ultimaStrings[1000]);
    player.armors[player.armor-1] -= 1;
    player.armor = 0;
    return true;
  } else if (enemy == 37) {
    player.food /= 2;
    uiConsole_queueMessage(ultimaStrings[1001]);
    uiConsole_updateStats();
    return true;
  } else if (enemy == 42) {
    if (rand01() >= 0.5) {
      uiConsole_queueMessage(ultimaStrings[1002]);
      player.intelligence = (int)(player.intelligence * 0.6f + 5);
      return true;
    }
  }

  return false;
}

static void sceneDungeon_enemyAttack(int monster) {
  uiConsole_queueMessageFormat("%s%s", ultimaStrings[991], enemyDefinitions[monstersIndex + monster].name);
  float attack = rand01() * 10.0f + 3 * player.dungeonDepth;
  float dodge = player.stamina / 3.0f * rand01() + player.armor * 3;
  if (dodge > 20 || attack < dodge) {
    uiConsole_queueMessage(ultimaStrings[992]);
    return;
  }
  int q = monstersIndex + monster;
  if (q == 23 || q == 37 || q == 30 || q == 42) {
    if (sceneDungeon_enemySpecialAttack(monster)) {
      return;
    }
  }
  int damage = (int)(player.dungeonDepth + player.dungeonDepth * monster * player.dungeonDepth * rand01() + player.dungeonDepth);
  uiConsole_queueMessageFormat("%s%d", ultimaStrings[993], damage);
  player.health -= damage;
  uiConsole_updateStats();
}

static void sceneDungeon_moveEnemies() {
  for (int i=1;i<=5;i++) {
    if (monsters[monstersIndex + i][0] == 1) { continue; }

    int xx = player.px - monsters[monstersIndex + i][1];
    int yy = player.py - monsters[monstersIndex + i][2];
    int dx = (xx > 0) - (xx < 0);
    int dy = (yy > 0) - (yy < 0);
    float ra = sqrtf(xx*xx + yy*yy);

    if (ra < 1.4) { 
      sceneDungeon_enemyAttack(i);
      return; 
    }
    if (ra > 3 && player.px != monsters[monstersIndex + i][1] && player.py != monsters[monstersIndex + i][2]) { continue; }

    if (xx != 0) {
      int zz = dungeonMap[monsters[monstersIndex + i][1] + dx][monsters[monstersIndex + i][2]];
      if (zz != 1 && zz != 12 && zz != 2 && zz < 21) {
        dungeonMap[monsters[monstersIndex + i][1]][monsters[monstersIndex + i][2]] -= 100 * i;
        monsters[monstersIndex + i][1] += dx;
        dungeonMap[monsters[monstersIndex + i][1]][monsters[monstersIndex + i][2]] += 100 * i;
      }
    } else {
      int zz = dungeonMap[monsters[monstersIndex + i][1]][monsters[monstersIndex + i][2] + dy];
      if (zz != 1 && zz != 12 && zz != 2 && zz < 21) {
        dungeonMap[monsters[monstersIndex + i][1]][monsters[monstersIndex + i][2]] -= 100 * i;
        monsters[monstersIndex + i][2] += dy;
        dungeonMap[monsters[monstersIndex + i][1]][monsters[monstersIndex + i][2]] += 100 * i;
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
  if (lagTime > 0) {
    lagTime -= deltaTime;
    if (lagTime < 0) { lagTime = 0; }
  }
  
  if (!queuedMessagesCount && lagTime <= 0 && !vmExecuter_update(deltaTime)) {
    if (respawnPlayer) {
      scene_load(&sceneOverworld);
      return;
    }

    if (ztatsActive) {
      uiZtats_update(deltaTime);
      return;
    }

    if (playerActed) {
      playerActed = false;
      
      sceneDungeon_moveEnemies();
      dungeonRenderer_update();

      if (player_isAlive()){
        uiConsole_addMessage(ultimaStrings[98]);
      } else {
        sceneOverworld_attemptResurrection();
      }
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