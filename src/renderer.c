
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
        .position = createVec3(0, -10, 0),
        .direction = createVec3(0, 1, 0),
        .up = createVec3(0, 0, 1),
    };
    ret.buffer = (Color*)malloc(sizeof(Color) * width * height);
    return ret;
}

void freeRenderer(Renderer renderer) {
    free(renderer.buffer);
}

static Color computeRayColor(Vec3 start, Vec3 direction, Scene* scene) {
    float min_dist = FLT_MAX;
    int min_o = -1;
    int min_t;
    for (int o = 0; o < scene->object_count; o++) {
        Mesh mesh = scene->objects[o].mesh;
        for (int t = 0; t < mesh.triangle_count; t++) {
            Vec3 vert0 = scene->vertecies[mesh.vertex_indices[t][0]];
            Vec3 vert1 = scene->vertecies[mesh.vertex_indices[t][1]];
            Vec3 vert2 = scene->vertecies[mesh.vertex_indices[t][2]];
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
                                min_t = t;
                            }
                        }
                    }
                }
            }
        }
    }
    if (min_o == -1) {
        return createVec3(1, 1, 1);
    } else {
        return createVec3((min_t % 2) / 1.0, (min_t % 3) / 2.0, (min_t % 5) / 4.0);
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
            Color color = computeRayColor(renderer.position, ray, &scene);
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
