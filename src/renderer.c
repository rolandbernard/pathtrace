
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <float.h>
#include <pthread.h>

#include "vec.h"
#include "renderer.h"
#include "intersection.h"

void initRenderer(Renderer* renderer, int width, int height, float hview, float vview) {
    renderer->width = width;
    renderer->height = height;
    renderer->horizontal_view = hview;
    renderer->vertical_view = vview;
    renderer->position = createVec3(0, 0, 0);
    renderer->direction = createVec3(0, 0, -1);
    renderer->up = createVec3(0, 1, 0);
    renderer->void_color = createVec3(0, 0, 0);
    renderer->specular_samples = 5;
    renderer->diffuse_samples = 10;
    renderer->depth = 100;
    renderer->specular_depth_cost = 20;
    renderer->diffuse_depth_cost = 30;
    renderer->transmition_depth_cost = 10;
    renderer->buffer = (Color*)malloc(sizeof(Color) * width * height);
    renderer->threads = 32;
}

void freeRenderer(Renderer* renderer) {
    free(renderer->buffer);
}

static Color computeRayColor(Ray* ray, Scene* scene, Renderer* renderer, int depth) {
    if (depth == 0) {
        return renderer->void_color;
    } else {
        Intersection intersection = {
            .dist = INFINITY, // Maximum distance
        }; 
        if (testRayBvhIntersection(ray, scene->bvh, &intersection)) {
            Vec3 vert0 = scene->vertecies[scene->vertex_indices[intersection.triangle_id][0]];
            Vec3 vert1 = scene->vertecies[scene->vertex_indices[intersection.triangle_id][1]];
            Vec3 vert2 = scene->vertecies[scene->vertex_indices[intersection.triangle_id][2]];
            Vec3 vert = addVec3(
                scaleVec3(vert0, 1 - intersection.u - intersection.v),
                addVec3(
                    scaleVec3(vert1, intersection.u),
                    scaleVec3(vert2, intersection.v)
                )
            );
            Vec3 norm0 = scene->normals[scene->normal_indices[intersection.triangle_id][0]];
            Vec3 norm1 = scene->normals[scene->normal_indices[intersection.triangle_id][1]];
            Vec3 norm2 = scene->normals[scene->normal_indices[intersection.triangle_id][2]];
            Vec3 normal = addVec3(
                scaleVec3(norm0, 1 - intersection.u - intersection.v),
                addVec3(
                    scaleVec3(norm1, intersection.u),
                    scaleVec3(norm2, intersection.v)
                )
            );
            if (dotVec3(normal, ray->direction) > 0) {
                normal = scaleVec3(normal, -1);
            }
            int object_id = scene->object_ids[intersection.triangle_id];
            MaterialProperties* material = &scene->objects[object_id].material;
            Color c = material->emission_color;
            if (depth - renderer->diffuse_depth_cost > 0) {
                if (!isVec3Null(material->diffuse_color)) {
                    Color diffuse_color = createVec3(0, 0, 0);
                    for (int s = 0; s < renderer->diffuse_samples; s++) {
                        Ray new_ray = createRay(vert, randomVec3InDirection(normal));
                        Color color = computeRayColor(&new_ray, scene, renderer, depth - renderer->diffuse_depth_cost);
                        diffuse_color = addVec3(diffuse_color, color);
                    }
                    diffuse_color = scaleVec3(diffuse_color, 1.0 / renderer->diffuse_samples);
                    diffuse_color = mulVec3(diffuse_color, material->diffuse_color);
                    c = addVec3(c, diffuse_color);
                }
            }
            if (material->specular_sharpness != 0) {
                if (!isVec3Null(material->specular_color)) {
                }
                if (material->transmitability != 0 && !isVec3Null(material->transmition_color)) {
                }
            }
            // c = addVec3(c, scaleVec3(material->diffuse_color, -dotVec3(normal, ray->direction)));
            return c;
        } else {
            return renderer->void_color;
        }
    }
}

typedef struct {
    int line_from;
    int line_to;
    Vec3 right;
    Vec3 down;
    Vec3 forward;
    float horizontal_scale;
    float vertical_scale;
    Renderer* renderer;
    Scene* scene;
} RenderFunctionData;

static void* renderThreadFunction(void* udata) {
    RenderFunctionData* data = (RenderFunctionData*)udata;
    for (int y = data->line_from; y < data->line_to; y++) {
        for (int x = 0; x < data->renderer->width; x++) {
            float scale_x = (x / (float)data->renderer->width - 0.5) * data->horizontal_scale;
            float scale_y = (y / (float)data->renderer->height - 0.5) * data->vertical_scale;
            Vec3 direction = normalizeVec3(addVec3(data->forward, addVec3(scaleVec3(data->right, scale_x), scaleVec3(data->down, scale_y))));
            Ray ray = createRay(data->renderer->position, direction);
            Color color = computeRayColor(&ray, data->scene, data->renderer, data->renderer->depth);
            Color* pixel = data->renderer->buffer + (y * data->renderer->width + x);
            *pixel = addVec3(*pixel, color);
        }
    }
    return NULL;
}

void renderScene(Renderer* renderer, Scene* scene) {
    Vec3 right = normalizeVec3(crossVec3(renderer->direction, renderer->up));
    Vec3 down = normalizeVec3(crossVec3(renderer->direction, right));
    Vec3 forward = normalizeVec3(renderer->direction);
    float horizontal_scale = tanf(renderer->horizontal_view);
    float vertical_scale = tanf(renderer->vertical_view);
    RenderFunctionData data[renderer->threads];
    pthread_t threads[renderer->threads];
    for (int t = 0; t < renderer->threads; t++) {
        data[t].line_from = renderer->height * t / renderer->threads;
        data[t].line_to = renderer->height * (t + 1) / renderer->threads;
        data[t].forward = forward;
        data[t].down = down;
        data[t].right = right;
        data[t].horizontal_scale = horizontal_scale;
        data[t].vertical_scale = vertical_scale;
        data[t].renderer = renderer;
        data[t].scene = scene;
        pthread_create(threads + t, NULL, renderThreadFunction, (void*)(data + t));
    }
    for (int t = 0; t < renderer->threads; t++) {
        pthread_join(threads[t], NULL);
    }
}

void scaleBuffer(Renderer* renderer, float scale) {
    for (int i = 0; i < renderer->height; i++) {
        for (int j = 0; j < renderer->width; j++) {
            Color* pixel = renderer->buffer + (i * renderer->width + j);
            *pixel = scaleVec3(*pixel, scale);
        }
    }
}

void clearBuffer(Renderer* renderer) {
    for (int i = 0; i < renderer->height; i++) {
        for (int j = 0; j < renderer->width; j++) {
            renderer->buffer[i * renderer->width + j] = createVec3(0, 0, 0);
        }
    }
}
