
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
    renderer->pixel_samples = 128;
    renderer->depth = 100;
    renderer->specular_depth_cost = 20;
    renderer->diffuse_depth_cost = 50;
    renderer->transmition_depth_cost = 20;
    renderer->buffer = (Color*)malloc(sizeof(Color) * width * height);
    renderer->threads = 32;
}

void freeRenderer(Renderer* renderer) {
    free(renderer->buffer);
}

#include <assert.h>

static Color computeRayColor(Ray* ray, Scene* scene, Renderer* renderer, int depth) {
    if (depth <= 0) {
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
            bool outside = true;
            if (dotVec3(normal, ray->direction) > 0) {
                outside = false;
                normal = scaleVec3(normal, -1);
            }
            int object_id = scene->object_ids[intersection.triangle_id];
            MaterialProperties* material = &scene->objects[object_id].material;
            Color c = material->emission_color;
            if (rand() > (RAND_MAX / 2)) {
                if (depth - renderer->diffuse_depth_cost > 0) {
                    if (!isVec3Null(material->diffuse_color)) {
                        Ray new_ray = createRay(vert, randomVec3InDirection(normal, 1, 1));
                        Color color = computeRayColor(&new_ray, scene, renderer, depth - renderer->diffuse_depth_cost);
                        Color diffuse_color = scaleVec3(mulVec3(color, material->diffuse_color), 2.0);
                        c = addVec3(c, diffuse_color);
                    }
                }
            } else {
                if (material->specular_sharpness != 0) {
                    if (material->transmitability > 0.0 && !isVec3Null(material->transmition_color)) {
                        float n1 = outside ? 1.0 : material->index_of_refraction;
                        float n2 = outside ? material->index_of_refraction : 1.0;
                        float cosO = -dotVec3(ray->direction, normal);
                        float r0 = (n1 - n2) / (n1 + n2);
                        r0 *= r0;
                        float refl = r0 + (1 - r0) * powf(1 - cosO, 5);
                        if (refl * (float)RAND_MAX > rand()) {
                            if (depth - renderer->specular_depth_cost > 0) {
                                Vec3 reflection = subVec3(ray->direction, scaleVec3(normal, 2 * dotVec3(ray->direction, normal)));
                                Vec3 direction = randomVec3InDirection(reflection, 1, material->specular_sharpness);
                                Ray new_ray = createRay(vert, direction);
                                Color color = computeRayColor(&new_ray, scene, renderer, depth - renderer->specular_depth_cost);
                                Color reflection_color = mulVec3(color, material->specular_color);
                                c = addVec3(c, reflection_color);
                            }
                        } else {
                            if (depth - renderer->transmition_depth_cost > 0) {
                                float angle = acosf(cosO);
                                float sinO = sinf(angle);
                                Vec3 transmition = addVec3(scaleVec3(ray->direction, n1 / n2), scaleVec3(normal, (cosO * n1 / n2 - sqrtf(1 - sinO * sinO))));
                                Vec3 direction = randomVec3InDirection(transmition, 1, material->specular_sharpness);
                                Ray new_ray = createRay(vert, direction);
                                Color color = computeRayColor(&new_ray, scene, renderer, depth - renderer->transmition_depth_cost);
                                Color reflection_color = scaleVec3(color, material->transmitability);
                                reflection_color = mulVec3(reflection_color, material->transmition_color);
                                c = addVec3(c, reflection_color);
                            }
                        }
                    } else if (depth - renderer->specular_depth_cost > 0) {
                        if (!isVec3Null(material->specular_color)) {
                            Vec3 reflection = subVec3(ray->direction, scaleVec3(normal, 2 * dotVec3(ray->direction, normal)));
                            Vec3 direction = randomVec3InDirection(reflection, 1, material->specular_sharpness);
                            Ray new_ray = createRay(vert, direction);
                            Color color = computeRayColor(&new_ray, scene, renderer, depth - renderer->specular_depth_cost);
                            Color specular_color = scaleVec3(mulVec3(color, material->specular_color), 2.0);
                            c = addVec3(c, specular_color);
                        }
                    }
                }
            }
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
            Color pixel_color = createVec3(0, 0, 0);
            for (int s = 0; s < data->renderer->pixel_samples; s++) {
                Vec3 actual_direction = randomVec3InDirection(direction, 1e-5, 100);
                Ray ray = createRay(data->renderer->position, actual_direction);
                Color color = computeRayColor(&ray, data->scene, data->renderer, data->renderer->depth);
                pixel_color = addVec3(pixel_color, color);
            }
            pixel_color = scaleVec3(pixel_color, 1.0 / data->renderer->pixel_samples);
            Color* pixel = data->renderer->buffer + (y * data->renderer->width + x);
            *pixel = addVec3(*pixel, pixel_color);
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
