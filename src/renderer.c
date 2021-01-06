
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <float.h>

#include "vec.h"
#include "renderer.h"

Renderer createRenderer(int width, int height, float hview, float vview) {
    Renderer ret = {
        .width = width,
        .height = height,
        .horizontal_view = hview,
        .vertical_view = vview,
        .position = createVec3(0, 0, -2),
        .direction = createVec3(0, 0, -1),
        .up = createVec3(0, 1, 0),
        .void_color = createVec3(0, 0, 0),
    };
    ret.buffer = (Color*)malloc(sizeof(Color) * width * height);
    return ret;
}

void freeRenderer(Renderer renderer) {
    free(renderer.buffer);
}

static Color computeRayColor(Vec3 start, Vec3 direction, Scene* scene, Renderer* renderer, int depth) {
    if (depth == 0) {
        return createVec3(0, 0, 0);
    } else {
        float min_dist = FLT_MAX;
        int min_o = -1;
        int min_t;
        float min_u;
        float min_v;
        for (int o = 0; o < scene->object_count; o++) {
            Mesh mesh = scene->objects[o].mesh;
            for (int tr = 0; tr < mesh.triangle_count; tr++) {
                Vec3 vert0 = scene->vertecies[mesh.vertex_indices[tr][0]];
                Vec3 vert1 = scene->vertecies[mesh.vertex_indices[tr][1]];
                Vec3 vert2 = scene->vertecies[mesh.vertex_indices[tr][2]];
                Vec3 edge1 = subVec3(vert1, vert0);
                Vec3 edge2 = subVec3(vert2, vert0);
                Vec3 h = crossVec3(direction, edge2);
                float a = dotVec3(edge1, h);
                if (a < -FLT_EPSILON || a > FLT_EPSILON) {
                    float f = 1.0 / a;
                    Vec3 s = subVec3(start, vert0);
                    float u = f * dotVec3(s, h);
                    if (u >= 0.0 && u <= 1.0) {
                        Vec3 q = crossVec3(s, edge1);
                        float v = f * dotVec3(direction, q);
                        if (v >= 0.0 && u + v <= 1.0) {
                            float t = f * dotVec3(edge2, q);
                            if (t > FLT_EPSILON) {
                                if (t < min_dist) {
                                    min_dist = t;
                                    min_o = 0;
                                    min_u = u;
                                    min_v = v;
                                    min_t = tr;
                                }
                            }
                        }
                    }
                }
            }
        }
        if (min_o == -1) {
            return renderer->void_color;
        } else {
            Mesh mesh = scene->objects[min_o].mesh;
            Vec3 vert0 = scene->vertecies[mesh.vertex_indices[min_t][0]];
            Vec3 vert1 = scene->vertecies[mesh.vertex_indices[min_t][1]];
            Vec3 vert2 = scene->vertecies[mesh.vertex_indices[min_t][2]];
            Vec3 point = addVec3(scaleVec3(vert0, 1 - min_u - min_v), addVec3(scaleVec3(vert1, min_u), scaleVec3(vert2, min_v)));
            Vec3 norm0 = scene->normals[mesh.normal_indices[min_t][0]];
            Vec3 norm1 = scene->normals[mesh.normal_indices[min_t][1]];
            Vec3 norm2 = scene->normals[mesh.normal_indices[min_t][2]];
            Vec3 normal = addVec3(scaleVec3(norm0, 1 - min_u - min_v), addVec3(scaleVec3(norm1, min_u), scaleVec3(norm2, min_v)));
            MaterialProperties material = scene->objects[min_o].material;
            Color c = material.emission_color;
            return normal;
        }
    }
}

void renderScene(Renderer renderer, Scene scene) {
    Vec3 right = normalizeVec3(crossVec3(renderer.direction, renderer.up));
    Vec3 down = normalizeVec3(crossVec3(renderer.direction, right));
    Vec3 forward = normalizeVec3(renderer.direction);
    float horizontal_scale = tanf(renderer.horizontal_view);
    float vertical_scale = tanf(renderer.vertical_view);
    for (int y = 0; y < renderer.height; y++) {
        for (int x = 0; x < renderer.width; x++) {
            float scale_x = (x / (float)renderer.width - 0.5) * horizontal_scale; 
            float scale_y = (y / (float)renderer.height - 0.5) * vertical_scale; 
            Vec3 ray = normalizeVec3(addVec3(forward, addVec3(scaleVec3(right, scale_x), scaleVec3(down, scale_y))));
            Color color = computeRayColor(renderer.position, ray, &scene, &renderer, 1);
            Color* pixel = renderer.buffer + (y * renderer.width + x);
            *pixel = addVec3(*pixel, color);
        }
    }
}

void scaleBuffer(Renderer renderer, float scale) {
    for (int i = 0; i < renderer.height; i++) {
        for (int j = 0; j < renderer.width; j++) {
            Color* pixel = renderer.buffer + (i * renderer.width + j);
            *pixel = scaleVec3(*pixel, scale);
        }
    }
}

void clearBuffer(Renderer renderer) {
    for (int i = 0; i < renderer.height; i++) {
        for (int j = 0; j < renderer.width; j++) {
            renderer.buffer[i * renderer.width + j] = createVec3(0, 0, 0);
        }
    }
}
