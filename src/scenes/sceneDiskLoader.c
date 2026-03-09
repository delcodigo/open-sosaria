#include <string.h>
#include "sceneDiskLoader.h"
#include "engine/text.h"
#include "utils.h"

static Text diskNotFound;

void sceneDiskLoader_init() {
  if (!file_exists("disk1.dsk")) {
    text_create(&diskNotFound, "'disk1.dsk' not found!");
    return;
  }

  if (!file_exists("disk2.dsk")) {
    text_create(&diskNotFound, "'disk2.dsk' not found!");
    return;
  }

  text_create(&diskNotFound, "Loading ULTIMA Data...");
}

void sceneDiskLoader_update() {
  text_render(&diskNotFound, 140 - 11 * 7, 92);
}

void sceneDiskLoader_free() {
  text_free(&diskNotFound);
}

Scene sceneDiskLoader = {
  .scene_init = sceneDiskLoader_init,
  .scene_update = sceneDiskLoader_update,
  .scene_free = sceneDiskLoader_free
};
