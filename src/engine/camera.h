#ifndef OS_CAMERA_HEADER
#define OS_CAMERA_HEADER

#include <stdbool.h>

typedef struct {
  float priv_viewProjectionMatrix[16];
  float projectionMatrix[16];
  float viewMatrix[16];
  bool isDirty;
  float width;
  float height;
} Camera;

void camera_createOrthogonal(Camera *camera, float width, float height, float znear, float zfar);
void camera_free(Camera *camera);
void camera_setPosition3f(Camera *camera, float x, float y, float z);
float camera_getX(Camera *camera);
float camera_getY(Camera *camera);
float *camera_getViewProjectionMatrix(Camera *camera);

#endif