
#include <math.h>
#include <stdlib.h>

#include "vec.h"

Vec3 createVec3(float x, float y, float z) {
    Vec3 ret = { .x = x, .y = y, .z = z };
    return ret;
}

Vec3 scaleVec3(Vec3 v, float s) {
    return createVec3(v.x * s, v.y * s, v.z * s);
}

Vec3 addVec3(Vec3 u, Vec3 v) {
    return createVec3(u.x + v.x, u.y + v.y, u.z + v.z);
}

Vec3 subVec3(Vec3 u, Vec3 v) {
    return createVec3(u.x - v.x, u.y - v.y, u.z - v.z);
}

Vec3 mulVec3(Vec3 u, Vec3 v) {
    return createVec3(u.x * v.x, u.y * v.y, u.z * v.z);
}

float dotVec3(Vec3 u, Vec3 v) {
    return (u.x * v.x) + (u.y * v.y) + (u.z * v.z);
}

Vec3 crossVec3(Vec3 u, Vec3 v) {
    return createVec3(
        (u.y * v.z) - (u.z * v.y),
        (u.z * v.x) - (u.x * v.z),
        (u.x * v.y) - (u.y * v.x)
    );
}

float magnitudeVec3(Vec3 v) {
    float sum = 0;
    for (int k = 0; k < 3; k++) {
        sum += v.v[k] * v.v[k];
    }
    return sqrtf(sum);
}

Vec3 normalizeVec3(Vec3 v) {
    return scaleVec3(v, 1 / magnitudeVec3(v));
}

Vec3 minVec3(Vec3 u, Vec3 v) {
    return createVec3(
        fmin(u.x, v.x),
        fmin(u.y, v.y),
        fmin(u.z, v.z)
    );
}

Vec3 maxVec3(Vec3 u, Vec3 v) {
    return createVec3(
        fmax(u.x, v.x),
        fmax(u.y, v.y),
        fmax(u.z, v.z)
    );
}

bool isVec3Null(Vec3 u) {
    return u.x == 0 && u.y == 0 && u.z == 0;
}

// up and zero should be orthogonal and normalized
Vec3 fromInclineAndAzimuthal(Vec3 up, Vec3 zero, float incline, float azimuthal) {
    Vec3 right = crossVec3(zero, up);
    Vec3 zero_inc = addVec3(scaleVec3(zero, cosf(azimuthal)), scaleVec3(right, sinf(azimuthal)));
    Vec3 ret = addVec3(scaleVec3(zero_inc, cosf(incline)), scaleVec3(up, sinf(incline)));
    return ret;
}

Vec3 randomVec3() {
    float r0 = rand() / (float)RAND_MAX; // two uniformly random values from 0 to 1
    float r1 = rand() / (float)RAND_MAX;
    float O = 2 * PI * r0;
    float z = 2 * r1 - 1;
    return createVec3(
        sqrtf(1 - z*z) * cosf(O),
        sqrtf(1 - z*z) * sinf(O),
        z
    );
}

// v should be normalized
Vec3 randomVec3InDirection(Vec3 v, float off, float pow) {
    float r0 = rand() / (float)RAND_MAX; // two uniformly random values from 0 to 1
    float r1 = rand() / (float)RAND_MAX;
    float O = 2 * PI * r0;
    float z = 1 - powf(r1, pow) * off;
    Vec3 any_up = normalizeVec3(crossVec3(v, addVec3(v, createVec3(1, 1, 1)))); // any vector othogonal to v
    Vec3 right = crossVec3(v, any_up);
    Vec3 zero_inc = addVec3(scaleVec3(any_up, cosf(O)), scaleVec3(right, sinf(O)));
    Vec3 ret = addVec3(scaleVec3(zero_inc, sqrtf(1 - z*z)), scaleVec3(v, z));
    return ret;
}

Mat3x3 createNullMat3x3() {
    Mat3x3 ret = { .v = {
        { 0, 0, 0, },
        { 0, 0, 0, },
        { 0, 0, 0, },
    }};
    return ret;
}

Mat3x3 createIdentMat3x3() {
    Mat3x3 ret = { .v = {
        { 1, 0, 0, },
        { 0, 1, 0, },
        { 0, 0, 1, },
    }};
    return ret;
}

Mat3x3 createRotationXMat3x3(float a) {
    Mat3x3 ret = { .v = {
        { 1,      0,       0, },
        { 0, cos(a), -sin(a), },
        { 0, sin(a),  cos(a), },
    }};
    return ret;
}

Mat3x3 createRotationYMat3x3(float a) {
    Mat3x3 ret = { .v = {
        {  cos(a), 0, sin(a), },
        {       0, 1,      0, },
        { -sin(a), 0, cos(a), },
    }};
    return ret;
}

Mat3x3 createRotationZMat3x3(float a) {
    Mat3x3 ret = { .v = {
        { cos(a), -sin(a), 0, },
        { sin(a),  cos(a), 0, },
        {      0,       0, 1, },
    }};
    return ret;
}

Mat3x3 createScaleMat3x3(float x, float y, float z) {
    Mat3x3 ret = { .v = {
        { x, 0, 0, },
        { 0, y, 0, },
        { 0, 0, z, },
    }};
    return ret;
}

Mat3x3 scaleMat3x3(Mat3x3 A, float s) {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            A.v[i][j] *= s;
        }
    }
    return A;
}

Mat3x3 multMat3x3(Mat3x3 A, Mat3x3 B) {
    Mat3x3 ret;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            ret.v[i][j] = 0;
            for (int k = 0; k < 3; k++) {
                ret.v[i][j] += A.v[i][k] * B.v[k][j];
            }
        }
    }
    return ret;
}

Vec3 multMat3x3Vec3(Mat3x3 A, Vec3 v) {
    Vec3 ret;
    for (int i = 0; i < 3; i++) {
        ret.v[i] = 0;
        for (int k = 0; k < 3; k++) {
            ret.v[i] += A.v[i][k] * v.v[k];
        }
    }
    return ret;
}
