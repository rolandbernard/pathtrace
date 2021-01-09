#ifndef _INTERSECTION_H_
#define _INTERSECTION_H_

#include "bvh.h"
#include "vec.h"

typedef struct {
    float dist;
    int triangle_id;
    float u;
    float v;
} Intersection;

typedef struct {
    Vec3 start;
    Vec3 direction;
    Vec3 inv_direction;
    int sign[3];
} Ray;

Ray createRay(Vec3 start, Vec3 direction);

bool testRayTriangleIntersection(Ray* ray, Vec3 vert[3], Intersection* out);

bool testRayBoundingBoxIntersection(Ray* ray, BoundingBox* bounds, float t0, float t1);

bool testRayBvhIntersection(Ray* ray, BvhNode* bvh, Intersection* out);

#endif