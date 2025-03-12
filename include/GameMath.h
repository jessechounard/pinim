#ifndef PINIM_GAME_MATH_INCLUDED
#define PINIM_GAME_MATH_INCLUDED

#include <assert.h>
#include <SDL3/SDL.h>

typedef float Vector2[2];
typedef float Matrix4[16];

#define MATRIX4_IDENTITY (Matrix4){1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}

void Vector2_Copy(Vector2 source, Vector2 destination);
float Vector2_DotProduct(Vector2 v1, Vector2 v2);
// Matrix * Vector
void Matrix4_Zero(Matrix4 destination);
void Matrix4_Identity(Matrix4 destination);
void Matrix4_Copy(Matrix4 source, Matrix4 destination);
void Matrix4_Multiply(Matrix4 m1, Matrix4 m2, Matrix4 destination);
void Matrix4_OrthoCamera(
    float left, float right, float bottom, float top, float nearZ, float farZ, Matrix4 destination);

#endif // PINIM_GAME_MATH_INCLUDED

#if defined(PINIM_GAME_MATH_IMPLEMENTATION)

void Vector2_Copy(Vector2 source, Vector2 destination) {
    assert(source != NULL);
    assert(destination != NULL);

    destination[0] = source[0];
    destination[1] = source[1];
}

float Vector2_DotProduct(Vector2 v1, Vector2 v2) {
    return v1[0] * v2[0] + v1[1] * v2[1];
}

void Matrix4_Zero(Matrix4 destination) {
    assert(destination != NULL);

    SDL_memset(destination, 0, sizeof(float) * 16);
}
void Matrix4_Identity(Matrix4 destination) {
    assert(destination != NULL);

    Matrix4_Zero(destination);
    destination[0] = destination[5] = destination[10] = destination[15] = 1;
}

void Matrix4_Copy(Matrix4 source, Matrix4 destination) {
    assert(source != NULL);
    assert(destination != NULL);

    SDL_memcpy(destination, source, sizeof(float) * 16);
}

void Matrix4_Multiply(Matrix4 m1, Matrix4 m2, Matrix4 destination) {
    assert(m1 != NULL);
    assert(m2 != NULL);
    assert(destination != NULL);

    float a00 = m1[0], a01 = m1[1], a02 = m1[2], a03 = m1[3];
    float a10 = m1[4], a11 = m1[5], a12 = m1[6], a13 = m1[7];
    float a20 = m1[8], a21 = m1[9], a22 = m1[10], a23 = m1[11];
    float a30 = m1[12], a31 = m1[13], a32 = m1[14], a33 = m1[15];

    float b00 = m2[0], b01 = m2[1], b02 = m2[2], b03 = m2[3];
    float b10 = m2[4], b11 = m2[5], b12 = m2[6], b13 = m2[7];
    float b20 = m2[8], b21 = m2[9], b22 = m2[10], b23 = m2[11];
    float b30 = m2[12], b31 = m2[13], b32 = m2[14], b33 = m2[15];

    destination[0] = a00 * b00 + a10 * b01 + a20 * b02 + a30 * b03;
    destination[1] = a01 * b00 + a11 * b01 + a21 * b02 + a31 * b03;
    destination[2] = a02 * b00 + a12 * b01 + a22 * b02 + a32 * b03;
    destination[3] = a03 * b00 + a13 * b01 + a23 * b02 + a33 * b03;
    destination[4] = a00 * b10 + a10 * b11 + a20 * b12 + a30 * b13;
    destination[5] = a01 * b10 + a11 * b11 + a21 * b12 + a31 * b13;
    destination[6] = a02 * b10 + a12 * b11 + a22 * b12 + a32 * b13;
    destination[7] = a03 * b10 + a13 * b11 + a23 * b12 + a33 * b13;
    destination[8] = a00 * b20 + a10 * b21 + a20 * b22 + a30 * b23;
    destination[9] = a01 * b20 + a11 * b21 + a21 * b22 + a31 * b23;
    destination[10] = a02 * b20 + a12 * b21 + a22 * b22 + a32 * b23;
    destination[11] = a03 * b20 + a13 * b21 + a23 * b22 + a33 * b23;
    destination[12] = a00 * b30 + a10 * b31 + a20 * b32 + a30 * b33;
    destination[13] = a01 * b30 + a11 * b31 + a21 * b32 + a31 * b33;
    destination[14] = a02 * b30 + a12 * b31 + a22 * b32 + a32 * b33;
    destination[15] = a03 * b30 + a13 * b31 + a23 * b32 + a33 * b33;
}

void Matrix4_OrthoCamera(float left, float right, float bottom, float top, float nearZ, float farZ,
    Matrix4 destination) {
    float rl, tb, fn;

    Matrix4_Zero(destination);

    rl = 1.0f / (right - left);
    tb = 1.0f / (top - bottom);
    fn = -1.0f / (farZ - nearZ);

    destination[0] = 2.0f * rl;
    destination[5] = 2.0f * tb;
    destination[10] = 2.0f * fn;
    destination[12] = -(right + left) * rl;
    destination[13] = -(top + bottom) * tb;
    destination[14] = (farZ + nearZ) * fn;
    destination[15] = 1.0f;
}

#endif