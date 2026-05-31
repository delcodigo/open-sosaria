#ifndef OS_SPACE_3D_H
#define OS_SPACE_3D_H

void space3D_init();
void space3D_render(float *viewMatrix);
void space3D_update(float delta);
void space3D_free();

#endif