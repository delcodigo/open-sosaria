#ifndef OS_SCENE_H
#define OS_SCENE_H

typedef struct {
  void (*scene_init)();
  void (*scene_update)(float deltaTime);
  void (*scene_free)();
} Scene;

extern Scene currentScene;

void scene_load(Scene *newScene);

#endif