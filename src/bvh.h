#ifndef _BVH_H_
#define _BVH_H_

#include "vec.h"

typedef enum {
    BVH_NODE_INTERNAL,
    BVH_NODE_TRIANGLE,
} BvhNodeKind;

#define BVH_NODE_BASE BvhNodeKind kind;

typedef struct {
   Vec3 bound[2];
} BoundingBox;

typedef struct {
    BVH_NODE_BASE
} BvhNode;

typedef struct {
    BVH_NODE_BASE
    BoundingBox bounds;
    BvhNode* children[2];
    int split_axis;
} BvhNodeInternal;

typedef struct {
    BVH_NODE_BASE
    int triangle_id;
    Vec3 verts[3];
} BvhNodeTriangle;

BvhNode* buildBvh(int (*vert_indices)[3], Vec3* verts, int triangle_count);

void freeBvh(BvhNode* bvh);

#endif