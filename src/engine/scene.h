#ifndef OS_SCENE_H
#define OS_SCENE_H

typedef struct {
  void (*scene_init)();
  void (*scene_update)();
  void (*scene_free)();
} Scene;

#endif