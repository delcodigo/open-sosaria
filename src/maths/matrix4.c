#include <stdlib.h>
#include "matrix4.h"

void matrix4_set(float *matrix, 
    float a1, float a2, float a3, float a4, 
    float b1, float b2, float b3, float b4, 
    float c1, float c2, float c3, float c4,
    float d1, float d2, float d3, float d4) {
  matrix[ 0] = a1; matrix[ 1] = a2; matrix[ 2] = a3; matrix[ 3] = a4;
  matrix[ 4] = b1; matrix[ 5] = b2; matrix[ 6] = b3; matrix[ 7] = b4;
  matrix[ 8] = c1; matrix[ 9] = c2; matrix[10] = c3; matrix[11] = c4;
  matrix[12] = d1; matrix[13] = d2; matrix[14] = d3; matrix[15] = d4;
}

void matrix4_setIdentity(float *matrix) {
  matrix4_set(matrix,
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1
  );
}

void matrix4_setPosition(float *matrix, float x, float y, float z) {
  matrix[12] = x;
  matrix[13] = y;
  matrix[14] = z;
}

void matrix4_multiply(float *matrixA, float *matrixB) {
  float *C = matrixA;
  float *R = matrixB;

  matrix4_set(matrixA,
    R[0] * C[0] + R[1] * C[4] + R[2] * C[8] + R[3] * C[12],
    R[0] * C[1] + R[1] * C[5] + R[2] * C[9] + R[3] * C[13],
    R[0] * C[2] + R[1] * C[6] + R[2] * C[10] + R[3] * C[14],
    R[0] * C[3] + R[1] * C[7] + R[2] * C[11] + R[3] * C[15],

    R[4] * C[0] + R[5] * C[4] + R[6] * C[8] + R[7] * C[12],
    R[4] * C[1] + R[5] * C[5] + R[6] * C[9] + R[7] * C[13],
    R[4] * C[2] + R[5] * C[6] + R[6] * C[10] + R[7] * C[14],
    R[4] * C[3] + R[5] * C[7] + R[6] * C[11] + R[7] * C[15],

    R[8] * C[0] + R[9] * C[4] + R[10] * C[8] + R[11] * C[12],
    R[8] * C[1] + R[9] * C[5] + R[10] * C[9] + R[11] * C[13],
    R[8] * C[2] + R[9] * C[6] + R[10] * C[10] + R[11] * C[14],
    R[8] * C[3] + R[9] * C[7] + R[10] * C[11] + R[11] * C[15],

    R[12] * C[0] + R[13] * C[4] + R[14] * C[8] + R[15] * C[12],
    R[12] * C[1] + R[13] * C[5] + R[14] * C[9] + R[15] * C[13],
    R[12] * C[2] + R[13] * C[6] + R[14] * C[10] + R[15] * C[14],
    R[12] * C[3] + R[13] * C[7] + R[14] * C[11] + R[15] * C[15]
  );
}

void matrix4_setOrthogonalProjection(float *matrix, float width, float height, float znear, float zfar) {
  float l = 0.0f,
        r = width,
        b = height,
        t = 0.0f,
        
        A = 2.0 / (r - l),
        B = 2.0 / (t - b),
        C = -2 / (zfar - znear),
        
        X = -(r + l) / (r - l),
        Y = -(t + b) / (t - b),
        Z = -(zfar + znear) / (zfar - znear);

  matrix4_set(matrix,
    A, 0, 0, 0,
    0, B, 0, 0,
    0, 0, C, 0,
    X, Y, Z, 1
  );
}