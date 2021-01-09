
#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>

#include "intersection.h"

#define EPSILON 1e-4

bool testRayTriangleIntersection(const Ray* ray, const Vec3 vert[3], Intersection* out) {
    Vec3 edge1 = subVec3(vert[1], vert[0]);
    Vec3 edge2 = subVec3(vert[2], vert[0]);
    Vec3 h = crossVec3(ray->direction, edge2);
    float a = dotVec3(edge1, h);
    if (a < -EPSILON || a > EPSILON) {
        float f = 1.0 / a;
        Vec3 s = subVec3(ray->start, vert[0]);
        float u = f * dotVec3(s, h);
        if (u >= 0 && u <= 1.0) {
            Vec3 q = crossVec3(s, edge1);
            float v = f * dotVec3(ray->direction, q);
            if (v >= 0 && u + v <= 1.0) {
                float t = f * dotVec3(edge2, q);
                if (t > EPSILON && t < out->dist) {
                    if (out != NULL) {
                        out->dist = t;
                        out->u = u;
                        out->v = v;
                    }
                    return true;
                }
            }
        }
    }
    return false;
}

bool testRayBoundingBoxIntersection(const Ray* ray, const BoundingBox* bb, float t0, float t1) {
    float tmin, tmax, tymin, tymax, tzmin, tzmax;
    tmin = (bb->bound[ray->sign[0]].x - ray->start.x) * ray->inv_direction.x;
    tmax = (bb->bound[1 - ray->sign[0]].x - ray->start.x) * ray->inv_direction.x;
    tymin = (bb->bound[ray->sign[1]].y - ray->start.y) * ray->inv_direction.y;
    tymax = (bb->bound[1 - ray->sign[1]].y - ray->start.y) * ray->inv_direction.y;
    if ((tmin > tymax) || (tymin > tmax)) {
        return false;
    }
    if (tymin > tmin) {
        tmin = tymin;
    }
    if (tymax < tmax) {
        tmax = tymax;
    }
    tzmin = (bb->bound[ray->sign[2]].z - ray->start.z) * ray->inv_direction.z;
    tzmax = (bb->bound[1 - ray->sign[2]].z - ray->start.z) * ray->inv_direction.z;
    if ((tmin > tzmax) || (tzmin > tmax)) {
        return false;
    }
    if (tzmin > tmin) {
        tmin = tzmin;
    }
    if (tzmax < tmax) {
        tmax = tzmax;
    }
    return ((tmin < t1) && (tmax > t0));
}

bool testRayBvhIntersection(const Ray* ray, const BvhNode* bvh, Intersection* out) {
    switch (bvh->kind) {
    case BVH_NODE_INTERNAL: {
        BvhNodeInternal* inter = (BvhNodeInternal*)bvh;
        if (testRayBoundingBoxIntersection(ray, &inter->bounds, EPSILON, out->dist)) {
            bool intersec0 = testRayBvhIntersection(ray, inter->children[ray->sign[inter->split_axis]], out);
            bool intersec1 = testRayBvhIntersection(ray, inter->children[1 - ray->sign[inter->split_axis]], out);
            return intersec0 || intersec1;
        } else {
            return false;
        }
    } break;
    case BVH_NODE_TRIANGLE: {
        BvhNodeTriangle* tri = (BvhNodeTriangle*)bvh;
        if (testRayTriangleIntersection(ray, tri->verts, out)) {
            out->triangle_id = tri->triangle_id;
            return true;
        } else {
            return false;
        }
    } break;
    default:
        return false;
        break;
    }
}

Ray createRay(Vec3 start, Vec3 direction) {
    Ray ret = {
        .start = start,
        .direction = direction,
        .inv_direction = createVec3(1 / direction.x, 1 / direction.y, 1 / direction.z),
    };
    ret.sign[0] = ret.inv_direction.x < 0;
    ret.sign[1] = ret.inv_direction.y < 0;
    ret.sign[2] = ret.inv_direction.z < 0;
    return ret;
}
