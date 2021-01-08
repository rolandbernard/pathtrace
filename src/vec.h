#ifndef _VEC_H_
#define _VEC_H_

typedef union {
    struct {
        float x;
        float y;
        float z;
    };
    float v[3];
} Vec3;

typedef Vec3 Color;

Vec3 createVec3(float x, float y, float z);

Vec3 scaleVec3(Vec3 v, float s);

Vec3 addVec3(Vec3 u, Vec3 v);

Vec3 subVec3(Vec3 u, Vec3 v);

Vec3 mulVec3(Vec3 u, Vec3 v);

float dotVec3(Vec3 u, Vec3 v);

Vec3 crossVec3(Vec3 u, Vec3 v);

float magnitudeVec3(Vec3 v);

Vec3 normalizeVec3(Vec3 v);

Vec3 minVec3(Vec3 u, Vec3 v);

Vec3 maxVec3(Vec3 u, Vec3 v);

typedef struct {
    float v[3][3];
} Mat3x3;

Mat3x3 createNullMat3x3();

Mat3x3 createIdentMat3x3();

Mat3x3 createRotationXMat3x3(float a);

Mat3x3 createRotationYMat3x3(float a);

Mat3x3 createRotationZMat3x3(float a);

Mat3x3 createScaleMat3x3(float x, float y, float z);

Mat3x3 scaleMat3x3(Mat3x3 A, float s);

Mat3x3 multMat3x3(Mat3x3 A, Mat3x3 B);

Vec3 multMat3x3Vec3(Mat3x3 A, Vec3 v);

#endif