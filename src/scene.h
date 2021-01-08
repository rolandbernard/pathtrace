#ifndef _MESH_H_
#define _MESH_H_

#include <stdbool.h>

#include "vec.h"
#include "bvh.h"

typedef struct {
    Color emission_color;
    Color specular_color;
    Color diffuse_color;
    Color transmition_color;
    float transmitability;
    float specular_sharpness;
    float index_of_refraction;
} MaterialProperties;

MaterialProperties createDefaultMaterial();

typedef struct {
    int starting_triangle;
    MaterialProperties material;
} Object;

typedef struct {
    Vec3* vertecies;
    Vec3* normals;
    int (*vertex_indices)[3];
    int (*normal_indices)[3];
    int* object_ids;
    int triangle_count;
    Object* objects;
    int object_count;
    BvhNode* bvh;
} Scene;

void freeScene(Scene* scene);

void loadFromObj(Scene* scene, const char* obj_content);

#endif