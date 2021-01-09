
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "scene.h"

MaterialProperties createDefaultMaterial() {
    MaterialProperties ret = {
        .emission_color = createVec3(0.0, 0.0, 0.0),
        .diffuse_color = createVec3(0.0, 0.0, 0.0),
        .specular_color = createVec3(0.0, 0.0, 0.0),
        .transmition_color = createVec3(1.0, 1.0, 1.0),
        .specular_sharpness = 1,
        .transmitability = 0.0,
        .index_of_refraction = 1.5,
    };
    return ret;
}

void freeScene(Scene* scene) {
    free(scene->vertecies);
    free(scene->normals);
    free(scene->vertex_indices);
    free(scene->normal_indices);
    free(scene->object_ids);
    free(scene->objects);
    freeBvh(scene->bvh);
}

typedef struct {
    int count;
    int capacity;
    char** names;
    MaterialProperties* props;
} MaterialList;

static void initMaterialList(MaterialList* list) {
    list->count = 0;
    list->capacity = 4;
    list->names = (char**)malloc(sizeof(char*) * list->capacity);
    list->props = (MaterialProperties*)malloc(sizeof(MaterialProperties) * list->capacity);
}

static void freeMaterialList(MaterialList* list)  {
    for (int i = 0; i < list->count; i++) {
        free(list->names[i]);
    }
    free(list->names);
    free(list->props);
}

static void addMaterial(MaterialList* list, char* name, MaterialProperties mat) {
    if (list->count == list->capacity) {
        list->capacity *= 2;
        list->names = (char**)realloc(list->names, sizeof(char*) * list->capacity);
        list->props = (MaterialProperties*)realloc(list->props, sizeof(MaterialProperties) * list->capacity);
    }
    list->names[list->count] = name;
    list->props[list->count] = mat;
    list->count++;
}

static MaterialProperties getMaterial(MaterialList* list, const char* name) {
    for (int i = 0; i < list->count; i++) {
        if (strcmp(list->names[i], name) == 0) {
            return list->props[i];
        }
    }
    return createDefaultMaterial();
}

static void loadMaterials(MaterialList* list, const char* mtl_content) {
    char tmp[128];
    MaterialProperties props = createDefaultMaterial();
    char* last_name = NULL;
    int offset = 0;
    while (mtl_content[offset] != 0) {
        if (mtl_content[offset] != '#') {
            if (mtl_content[offset] == 'N') {
                if (mtl_content[offset + 1] == 's') {
                    offset += 2;
                    while (mtl_content[offset] == ' ') {
                        offset++;
                    }
                    int num_start = offset;
                    while (isdigit(mtl_content[offset]) || mtl_content[offset] == '.' || mtl_content[offset] == '-' || mtl_content[offset] == '+') {
                        offset++;
                    }
                    memcpy(tmp, mtl_content + num_start, offset - num_start);
                    tmp[offset - num_start] = 0;
                    props.specular_sharpness = atof(tmp);
                } else if (mtl_content[offset + 1] == 'i') {
                    offset += 2;
                    while (mtl_content[offset] == ' ') {
                        offset++;
                    }
                    int num_start = offset;
                    while (isdigit(mtl_content[offset]) || mtl_content[offset] == '.' || mtl_content[offset] == '-' || mtl_content[offset] == '+') {
                        offset++;
                    }
                    memcpy(tmp, mtl_content + num_start, offset - num_start);
                    tmp[offset - num_start] = 0;
                    props.index_of_refraction = atof(tmp);
                }
            } else if (mtl_content[offset] == 'K') {
                if (mtl_content[offset + 1] == 'd') {
                    offset += 2;
                    for (int k = 0; k < 3; k++) {
                        while (mtl_content[offset] == ' ') {
                            offset++;
                        }
                        int num_start = offset;
                        while (isdigit(mtl_content[offset]) || mtl_content[offset] == '.' || mtl_content[offset] == '-' || mtl_content[offset] == '+') {
                            offset++;
                        }
                        memcpy(tmp, mtl_content + num_start, offset - num_start);
                        tmp[offset - num_start] = 0;
                        props.diffuse_color.v[k] = atof(tmp);
                    }
                } else if (mtl_content[offset + 1] == 's') {
                    offset += 2;
                    for (int k = 0; k < 3; k++) {
                        while (mtl_content[offset] == ' ') {
                            offset++;
                        }
                        int num_start = offset;
                        while (isdigit(mtl_content[offset]) || mtl_content[offset] == '.' || mtl_content[offset] == '-' || mtl_content[offset] == '+') {
                            offset++;
                        }
                        memcpy(tmp, mtl_content + num_start, offset - num_start);
                        tmp[offset - num_start] = 0;
                        props.specular_color.v[k] = atof(tmp);
                    }
                } else if (mtl_content[offset + 1] == 'e') {
                    offset += 2;
                    for (int k = 0; k < 3; k++) {
                        while (mtl_content[offset] == ' ') {
                            offset++;
                        }
                        int num_start = offset;
                        while (isdigit(mtl_content[offset]) || mtl_content[offset] == '.' || mtl_content[offset] == '-' || mtl_content[offset] == '+') {
                            offset++;
                        }
                        memcpy(tmp, mtl_content + num_start, offset - num_start);
                        tmp[offset - num_start] = 0;
                        props.emission_color.v[k] = atof(tmp);
                    }
                }
            } else if (mtl_content[offset] == 'T') {
                if (mtl_content[offset + 1] == 'r') {
                    offset += 2;
                    while (mtl_content[offset] == ' ') {
                        offset++;
                    }
                    int num_start = offset;
                    while (isdigit(mtl_content[offset]) || mtl_content[offset] == '.' || mtl_content[offset] == '-' || mtl_content[offset] == '+') {
                        offset++;
                    }
                    memcpy(tmp, mtl_content + num_start, offset - num_start);
                    tmp[offset - num_start] = 0;
                    props.transmitability = atof(tmp);
                } else if (mtl_content[offset + 1] == 'f') {
                    offset += 2;
                    for (int k = 0; k < 3; k++) {
                        while (mtl_content[offset] == ' ') {
                            offset++;
                        }
                        int num_start = offset;
                        while (isdigit(mtl_content[offset]) || mtl_content[offset] == '.' || mtl_content[offset] == '-' || mtl_content[offset] == '+') {
                            offset++;
                        }
                        memcpy(tmp, mtl_content + num_start, offset - num_start);
                        tmp[offset - num_start] = 0;
                        props.transmition_color.v[k] = atof(tmp);
                    }
                }
            } else if (strncmp(mtl_content + offset, "newmtl ", 7) == 0) {
                if (last_name != NULL) {
                    addMaterial(list, last_name, props);
                    props = createDefaultMaterial();
                }
                offset += 7;
                int name_start = offset;
                while (mtl_content[offset] != 0 && mtl_content[offset] != '\n') {
                    offset++;
                }
                int len = offset - name_start;
                last_name = (char*)malloc(sizeof(char) * (len + 1));
                memcpy(last_name, mtl_content + name_start, len);
                last_name[len] = 0;
            }
        }
        while (mtl_content[offset] != '\n' && mtl_content[offset] != 0) {
            offset++;
        }
        if (mtl_content[offset] == '\n') {
            offset++;
        }
    }
    if (last_name != NULL) {
        addMaterial(list, last_name, props);
    }
}

void loadFromObj(Scene* scene, const char* obj_content, const char* mtl_content) {
    MaterialList mtl_list;
    initMaterialList(&mtl_list);
    loadMaterials(&mtl_list, mtl_content);
    char tmp[128];
    int vertex_count = 0;
    int normal_count = 0;
    int triangle_count = 0;
    int object_count = 0;
    int offset = 0;
    // Count the number of vertecies, normals and triangles
    while (obj_content[offset] != 0) {
        if (obj_content[offset] != '#') {
            if (obj_content[offset] == 'v') {
                if (obj_content[offset + 1] == ' ') {
                    vertex_count++;
                } else if (obj_content[offset + 1] == 'n') {
                    normal_count++;
                }
            } else if (obj_content[offset] == 'f' && obj_content[offset + 1] == ' ') {
                int face_vert_count = 0;
                for (int i = offset; obj_content[i] != '\n' && obj_content[i] != 0; i++) {
                    if (obj_content[i] == ' ' && obj_content[i + 1] >= '0' && obj_content[i + 1] <= '9') {
                        face_vert_count++;
                    }
                }
                triangle_count += face_vert_count - 2;
            } else if (obj_content[offset] == 'o' && obj_content[offset + 1] == ' ') {
                object_count++;
            }
        }
        while (obj_content[offset] != '\n' && obj_content[offset] != 0) {
            offset++;
        }
        if (obj_content[offset] == '\n') {
            offset++;
        }
    }
    Vec3* vertecies = (Vec3*)malloc(sizeof(Vec3) * vertex_count);
    Vec3* normals = (Vec3*)malloc(sizeof(Vec3) * normal_count);
    int (*vertex_indices)[3] = (int(*)[3])malloc(sizeof(int[3]) * triangle_count);
    int (*normal_indices)[3] = (int(*)[3])malloc(sizeof(int[3]) * triangle_count);
    int* object_ids = (int*)malloc(sizeof(int) * triangle_count);
    Object* objects = (Object*)malloc(sizeof(Object) * object_count);
    int vertex_id = 0;
    int normal_id = 0;
    int triangle_id = 0;
    int object_id = 0;
    offset = 0;
    while (obj_content[offset] != 0) {
        if (obj_content[offset] != '#') {
            if (obj_content[offset] == 'v') {
                if (obj_content[offset + 1] == ' ') {
                    Vec3* vertex = vertecies + vertex_id;
                    offset += 2;
                    for (int k = 0; k < 3; k++) {
                        while (obj_content[offset] == ' ') {
                            offset++;
                        }
                        int num_start = offset;
                        while (isdigit(obj_content[offset]) || obj_content[offset] == '.' || obj_content[offset] == '-' || obj_content[offset] == '+') {
                            offset++;
                        }
                        memcpy(tmp, obj_content + num_start, offset - num_start);
                        tmp[offset - num_start] = 0;
                        vertex->v[k] = atof(tmp);
                    }
                    vertex_id++;
                } else if (obj_content[offset + 1] == 'n') {
                    Vec3* normal = normals + normal_id;
                    offset += 3;
                    for (int k = 0; k < 3; k++) {
                        while (obj_content[offset] == ' ') {
                            offset++;
                        }
                        int num_start = offset;
                        while (isdigit(obj_content[offset]) || obj_content[offset] == '.' || obj_content[offset] == '-' || obj_content[offset] == '+') {
                            offset++;
                        }
                        memcpy(tmp, obj_content + num_start, offset - num_start);
                        tmp[offset - num_start] = 0;
                        normal->v[k] = atof(tmp);
                    }
                    normal_id++;
                }
            } else if (obj_content[offset] == 'f' && obj_content[offset + 1] == ' ') {
                int face_vert_count = 0;
                int face_verts[3];
                int face_norms[3];
                offset += 2;
                while (obj_content[offset] != 0 && obj_content[offset] != '\n') {
                    while (obj_content[offset] == ' ') {
                        offset++;
                    }
                    int num_start = offset;
                    while (isdigit(obj_content[offset])) {
                        offset++;
                    }
                    memcpy(tmp, obj_content + num_start, offset - num_start);
                    tmp[offset - num_start] = 0;
                    face_verts[face_vert_count] = atoi(tmp);
                    if (obj_content[offset] == '/') {
                        offset++;
                        while (isdigit(obj_content[offset])) {
                            offset++;
                        }
                        if (obj_content[offset] == '/') {
                            offset++;
                            int num_start = offset;
                            while (isdigit(obj_content[offset])) {
                                offset++;
                            }
                            memcpy(tmp, obj_content + num_start, offset - num_start);
                            tmp[offset - num_start] = 0;
                            face_norms[face_vert_count] = atoi(tmp);
                        }
                    }
                    face_vert_count++;
                    if (face_vert_count == 3) {
                        object_ids[triangle_id] = object_id - 1;
                        for (int k = 0; k < 3; k++) {
                            vertex_indices[triangle_id][k] = face_verts[k] + (face_verts[k] < 0 ? vertex_count : -1);
                            normal_indices[triangle_id][k] = face_norms[k] + (face_norms[k] < 0 ? normal_count : -1);
                        }
                        face_vert_count--;
                        face_verts[1] = face_verts[2];
                        face_norms[1] = face_norms[2];
                        triangle_id++;
                    }
                }
            } else if (obj_content[offset] == 'o' && obj_content[offset + 1] == ' ') {
                objects[object_id].material = createDefaultMaterial();
                objects[object_id].starting_triangle = triangle_id;
                object_id++;
            } else if (strncmp(obj_content + offset, "usemtl ", 7) == 0) {
                if (object_id > 0) {
                    offset += 7;
                    int name_start = offset;
                    while (obj_content[offset] != 0 && obj_content[offset] != '\n') {
                        offset++;
                    }
                    memcpy(tmp, obj_content + name_start, offset - name_start);
                    tmp[offset - name_start] = 0;
                    objects[object_id - 1].material = getMaterial(&mtl_list, tmp);
                }
            }
        }
        while (obj_content[offset] != '\n' && obj_content[offset] != 0) {
            offset++;
        }
        if (obj_content[offset] == '\n') {
            offset++;
        }
    }
    freeMaterialList(&mtl_list);
    scene->vertecies = vertecies;
    scene->normals = normals;
    scene->vertex_indices = vertex_indices;
    scene->normal_indices = normal_indices;
    scene->object_ids = object_ids;
    scene->triangle_count = triangle_count;
    scene->objects = objects;
    scene->object_count = object_count;
    scene->bvh = buildBvh(vertex_indices, vertecies, triangle_count);
}

