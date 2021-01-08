
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <float.h>

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
    renderer->diffuse_samples = 50;
    renderer->buffer = (Color*)malloc(sizeof(Color) * width * height);
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
            // MaterialProperties material = scene->objects[min_o].material;
            // Color c = material.emission_color;
            return scaleVec3(addVec3(normal, createVec3(1, 1, 1)), 0.5);
        } else {
            return renderer->void_color;
        }
    }
}

void renderScene(Renderer* renderer, Scene scene) {
    Vec3 right = normalizeVec3(crossVec3(renderer->direction, renderer->up));
    Vec3 down = normalizeVec3(crossVec3(renderer->direction, right));
    Vec3 forward = normalizeVec3(renderer->direction);
    float horizontal_scale = tanf(renderer->horizontal_view);
    float vertical_scale = tanf(renderer->vertical_view);
    for (int y = 0; y < renderer->height; y++) {
        for (int x = 0; x < renderer->width; x++) {
            float scale_x = (x / (float)renderer->width - 0.5) * horizontal_scale; 
            float scale_y = (y / (float)renderer->height - 0.5) * vertical_scale; 
            Ray ray = createRay(renderer->position, addVec3(forward, addVec3(scaleVec3(right, scale_x), scaleVec3(down, scale_y))));
            Color color = computeRayColor(&ray, &scene, renderer, 1);
            Color* pixel = renderer->buffer + (y * renderer->width + x);
            *pixel = addVec3(*pixel, color);
        }
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
