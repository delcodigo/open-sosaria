#ifndef OS_MATRIX4_H
#define OS_MATRIX4_H

void matrix4_set(float *matrix, float a1, float a2, float a3, float a4, float b1, float b2, float b3, float b4, float c1, float c2, float c3, float c4, float d1, float d2, float d3, float d4);
void matrix4_setIdentity(float *matrix);
void matrix4_setPosition(float *matrix, float x, float y, float z);
void matrix4_multiply(float *matrixA, float *matrixB);
void matrix4_setOrthogonalProjection(float *matrix, float width, float height, float znear, float zfar);

#endif