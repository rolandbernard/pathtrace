#ifndef _MESH_H_
#define _MESH_H_

#include "vec.h"

typedef struct {
    int (*vertex_indices)[3];
    int (*normal_indices)[3];
    int triangle_count;
} Mesh;

void freeMesh(Mesh mesh);

typedef struct {
    Color emission_color;
    Color specular_color;
    Color diffuse_color;
    Color transmition_color;
    float specular_sharpness;
    float reflective_index;
} MaterialProperties;

MaterialProperties createDefaultMaterial();

typedef struct {
    Mesh mesh;
    MaterialProperties material;
} Object;

void freeObject(Object obj);

typedef struct {
    Vec3* vertecies;
    Vec3* normals;
    int (*vertex_indices)[3];
    int (*normal_indices)[3];
    Object* objects;
    int object_count;
} Scene;

void freeScene(Scene scene);

Scene loadFromObj(const char* obj_content);

#endif