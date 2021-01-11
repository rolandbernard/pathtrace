
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <png.h>
#include <time.h>

#include "scene.h"
#include "renderer.h"

bool writePNGFile(const char* filename, Color* pixels, int width, int heigth) {
    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png == NULL) {
        return false;
    }
    png_infop info = png_create_info_struct(png);
    if (!info) {
        png_destroy_write_struct(&png, &info);
        return false;
    }
    FILE* fp = fopen(filename, "wb");
    if (fp == NULL) {
        png_destroy_write_struct(&png, &info);
        return false;
    }
    png_init_io(png, fp);
    png_set_IHDR(png, info, width, heigth, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
    png_colorp palette = (png_colorp)png_malloc(png, sizeof(png_color) * PNG_MAX_PALETTE_LENGTH);
    if (palette == NULL) {
        fclose(fp);
        png_destroy_write_struct(&png, &info);
        return false;
    }
    png_set_PLTE(png, info, palette, PNG_MAX_PALETTE_LENGTH);
    png_write_info(png, info);
    png_set_packing(png);
    png_bytepp rows = (png_bytepp)png_malloc(png, sizeof(png_bytep) * heigth);
    for (int i = 0; i < heigth; i++) {
        png_bytep row = (png_bytep)png_malloc(png, sizeof(png_byte) * width * 3);
        for (int j = 0; j < width; j++) {
            for (int k = 0; k < 3; k++) {
                float value = pixels[i * width + j].v[k];
                row[3 * j + k] = (png_byte)(fminf(fmaxf(256 * sqrtf(value), 0), 255));
            }
        }
        rows[i] = row;
    }
    png_write_image(png, rows);
    png_write_end(png, info);
    fclose(fp);
    for (int i = 0; i < heigth; i++) {
        png_free(png, rows[i]);
    }
    png_free(png, rows);
    png_free(png, palette);
    png_destroy_write_struct(&png, &info);
    return true;
}

#define WIDTH 125//0
#define HEIGHT 125//0

#define HVIEW 0.5
#define VVIEW 0.5

int main(int argc, char** argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s OBJ-FILE OUT-FILE\n", argv[0]);
        return EXIT_FAILURE;
    } else {
        srand(time(NULL));
        FILE* obj_file = fopen(argv[1], "r");
        if (obj_file == NULL) {
            fprintf(stderr, "failed to open '%s': %s\n", argv[1], strerror(errno));
            return EXIT_FAILURE;
        } else {
            fseek(obj_file, 0, SEEK_END);
            int size = ftell(obj_file);
            fseek(obj_file, 0, SEEK_SET);
            char* data = malloc(size + 1);
            fread(data, 1, size, obj_file);
            data[size] = 0;
            fclose(obj_file);
            Scene scene;
            char* mtl_data = NULL;
            int path_len = strlen(argv[1]);
            argv[1][path_len - 3] = 'm';
            argv[1][path_len - 2] = 't';
            argv[1][path_len - 1] = 'l';
            FILE* mtl_file = fopen(argv[1], "r");
            if (mtl_file != NULL) {
                fseek(mtl_file, 0, SEEK_END);
                int size = ftell(mtl_file);
                fseek(mtl_file, 0, SEEK_SET);
                mtl_data = malloc(size + 1);
                fread(mtl_data, 1, size, mtl_file);
                mtl_data[size] = 0;
                fclose(mtl_file);
            }
            loadFromObj(&scene, data, mtl_data);
            free(mtl_data);
            free(data);
            Renderer renderer;
            initRenderer(&renderer, WIDTH, HEIGHT, HVIEW, VVIEW);
            clearBuffer(&renderer);
            for (int i = 0; i < 128; i++) {
                renderScene(&renderer, &scene);
                scaleBuffer(&renderer, 1.0 / (i + 1));
                if (!writePNGFile(argv[2], renderer.buffer, WIDTH, HEIGHT)) {
                    fprintf(stderr, "failed to write '%s': %s\n", argv[2], strerror(errno));
                }
                scaleBuffer(&renderer, (i + 1));
            }
            freeRenderer(&renderer);
            freeScene(&scene); 
            return EXIT_SUCCESS;
        }
    }
}
