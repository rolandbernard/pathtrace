
#include <stdlib.h>
#include <math.h>

#include "renderer.h"

Renderer createRenderer(int width, int height, float hview, float vview) {
    Renderer ret = {
        .width = width,
        .height = height,
        .horizontal_view = hview,
        .vertical_view = vview,
        .position = createVec3(0, 0, 0),
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

}

void renderScene(Renderer renderer, Scene scene) {
    Vec3 right = normalizeVec3(crossVec3(renderer.direction, renderer.up));
    Vec3 down = normalizeVec3(crossVec3(renderer.direction, right));
    Vec3 forward = normalizeVec3(renderer.direction);
    float horizontal_scale = tanf(renderer.horizontal_view);
    float vertical_scale = tanf(renderer.vertical_view);
    for (int y = 0; y < renderer.height; y++) {
        for (int x = 0; x < renderer.width; x++) {
            
        }
    }
}

void scaleBuffer(Renderer renderer, float scale) {
    for (int i = 0; i < renderer.height; i++) {
        for (int j = 0; j < renderer.width; j++) {
            Color* pixel = renderer.buffer + (i * renderer.width + j);
            for (int k = 0; k < 3; k++) {
                pixel->v[k] *= scale;
            }
        }
    }
}