
#include <stdlib.h>
#include <math.h>

#include "bvh.h"

static BvhNode* createBVHLeaf(Vec3 verts[3], int triangle_id) {
    BvhNodeTriangle* ret = (BvhNodeTriangle*)malloc(sizeof(BvhNodeTriangle));
    ret->kind = BVH_NODE_TRIANGLE;
    ret->triangle_id = triangle_id;
    for (int k = 0; k < 3; k++) {
        ret->verts[k] = verts[k];
    }
    return (BvhNode*)ret;
}

static BvhNode* createBVHNode(BoundingBox bounds, BvhNode* children[2], int axis) {
    BvhNodeInternal* ret = (BvhNodeInternal*)malloc(sizeof(BvhNodeInternal));
    ret->kind = BVH_NODE_INTERNAL;
    ret->bounds = bounds;
    for (int k = 0; k < 2; k++) {
        ret->children[k] = children[k];
    }
    ret->split_axis = axis;
    return (BvhNode*)ret;
}

static void surroundTriangles(BoundingBox* bbox, int* ordering, int (*vert_indices)[3], Vec3* verts, int start, int end) {
    for (int i = start; i < end; i++) {
        for (int k = 0; k < 3; k++) {
            Vec3 v = verts[vert_indices[ordering[i]][k]];
            bbox->bound[0] = minVec3(bbox->bound[0], v);
            bbox->bound[1] = maxVec3(bbox->bound[1], v);
        }
    }
}

static float triangleCenter(int* ordering, int (*vert_indices)[3], Vec3* verts, int i, int axis) {
    float min = INFINITY;
    float max = -INFINITY;
    for (int k = 0; k < 3; k++) {
        min = fmin(min, verts[vert_indices[ordering[i]][k]].v[axis]);
        max = fmax(max, verts[vert_indices[ordering[i]][k]].v[axis]);
    }
    return (min + max) / 2;
}

static int qsplit(int* ordering, int (*vert_indices)[3], Vec3* verts, int start, int end, float pivot, int axis) {
    int insert_beg = start;
    int insert_end = end - 1;
    for (int i = start; i < end; i++) {
        float tri_center = triangleCenter(ordering, vert_indices, verts, i, axis);
        if (tri_center < pivot) {
            int tmp = ordering[i];
            ordering[i] = ordering[insert_beg];
            ordering[insert_beg] = tmp;
            insert_beg++;
        } else {
            int tmp = ordering[i];
            ordering[i] = ordering[insert_end];
            ordering[insert_end] = tmp;
            insert_end--;
        }
    }
    return insert_beg;
}

static void quicksort(int* ordering, int (*vert_indices)[3], Vec3* verts, int start, int end, int axis) {
    if (start + 2 == end) {
        float center0 = triangleCenter(ordering, vert_indices, verts, start, axis);
        float center1 = triangleCenter(ordering, vert_indices, verts, start + 1, axis);
        if (center1 < center0) {
            int tmp = ordering[start];
            ordering[start] = ordering[start + 1];
            ordering[start + 1] = tmp;
        }
    } else if (start < end - 1) {
        int pivot_index = (start + end) / 2;
        float pivot = triangleCenter(ordering, vert_indices, verts, pivot_index, axis);
        int tmp = ordering[start];
        ordering[start] = ordering[pivot_index];
        ordering[pivot_index] = tmp;
        int mid_point = qsplit(ordering, vert_indices, verts, start + 1, end, pivot, axis);
        tmp = ordering[start];
        ordering[start] = ordering[mid_point - 1];
        ordering[mid_point - 1] = tmp;
        quicksort(ordering, vert_indices, verts, start, mid_point - 1, axis);
        quicksort(ordering, vert_indices, verts, mid_point, end, axis);
    }
}

static BvhNode* buildBvhAlong(int* ordering, int (*vert_indices)[3], Vec3* verts, int start, int end, int axis) {
    if (start + 1 == end) {
        Vec3 vert[3];
        for (int k = 0; k < 3; k++) {
            vert[k] = verts[vert_indices[ordering[start]][k]];
        }
        return createBVHLeaf(vert, ordering[start]);
    } else {
        BoundingBox bbox = {
            .bound = { createVec3(INFINITY, INFINITY, INFINITY), createVec3(-INFINITY, -INFINITY, -INFINITY) },
        };
        surroundTriangles(&bbox, ordering, vert_indices, verts, start, end);
        quicksort(ordering, vert_indices, verts, start, end, axis);
        BvhNode* childs[2];
        int next_axis = (axis + 1) % 3;
        childs[0] = buildBvhAlong(ordering, vert_indices, verts, start, (start + end) / 2, next_axis);
        childs[1] = buildBvhAlong(ordering, vert_indices, verts, (start + end) / 2, end, next_axis);
        return createBVHNode(bbox, childs, axis);
    }
}

BvhNode* buildBvh(int (*vert_indices)[3], Vec3* verts, int triangle_count) {
    int* order = (int*)malloc(sizeof(int) * triangle_count);
    for (int i = 0; i < triangle_count; i++) {
        order[i] = i;
    }
    BvhNode* ret = buildBvhAlong(order, vert_indices, verts, 0, triangle_count, 0);
    free(order);
    return ret;
}

void freeBvh(BvhNode* bvh) {
    if (bvh != NULL) {
        switch (bvh->kind) {
        case BVH_NODE_INTERNAL: {
            BvhNodeInternal* inter = (BvhNodeInternal*)bvh;
            freeBvh(inter->children[0]);
            freeBvh(inter->children[1]);
        } break;
        default:
            break;
        }
        free(bvh);
    }
}
