#ifndef _RENDERER_H_
#define _RENDERER_H_

#include "vec.h"
#include "scene.h"

typedef struct {
    Color* buffer;
    int width;
    int height;
    float horizontal_view;
    float vertical_view;
    Vec3 position;
    Vec3 direction;
    Vec3 up;
    Vec3 void_color;
    int specular_samples;
    int diffuse_samples;
    int transmition_samples;
    int pixel_samples;
    int depth;
    int specular_depth_cost;
    int diffuse_depth_cost;
    int transmition_depth_cost;
    int threads;
} Renderer;

void initRenderer(Renderer* renderer, int width, int height, float hview, float vview);

void freeRenderer(Renderer* renderer);

void renderScene(Renderer* renderer, Scene* scene);

void scaleBuffer(Renderer* renderer, float scale);

void clearBuffer(Renderer* renderer);

#endif