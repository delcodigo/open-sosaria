#include <stdlib.h>
#include "camera.h"
#include "maths/matrix4.h"

void camera_createOrthogonal(Camera *camera, float width, float height, float znear, float zfar) {
  matrix4_setIdentity(camera->projectionMatrix);
  matrix4_setIdentity(camera->viewMatrix);
  matrix4_setIdentity(camera->priv_viewProjectionMatrix);
  camera->isDirty = true;
  camera->width = width;
  camera->height = height;

  matrix4_setOrthogonalProjection(camera->projectionMatrix, width, height, znear, zfar);
}

void camera_setPosition3f(Camera *camera, float x, float y, float z) {
  matrix4_setPosition(camera->viewMatrix, -x, -y, -z);
  camera->isDirty = true;
}

float camera_getX(Camera *camera) {
  return -camera->viewMatrix[12];
}

float camera_getY(Camera *camera) {
  return -camera->viewMatrix[13];
}

float *camera_getViewProjectionMatrix(Camera *camera) {
  if (!camera->isDirty) {
    return camera->priv_viewProjectionMatrix;
  }

  matrix4_setIdentity(camera->priv_viewProjectionMatrix);
  matrix4_multiply(camera->priv_viewProjectionMatrix, camera->projectionMatrix);
  matrix4_multiply(camera->priv_viewProjectionMatrix, camera->viewMatrix);

  camera->isDirty = false;

  return camera->priv_viewProjectionMatrix;
}