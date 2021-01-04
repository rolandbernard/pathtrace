
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <png.h>

#include "scene.h"

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
            int dx = j - width / 2;
            int dy = i - heigth / 2;
            if (dy*dy < heigth / 100) {
                row[j * 3]     = 0;
                row[j * 3 + 1] = 0;
                row[j * 3 + 2] = 0;
            } else if (dx*dx < heigth / 100) {
                row[j * 3]     = 0;
                row[j * 3 + 1] = 0;
                row[j * 3 + 2] = 0;
            } else if (abs(dx*dx + dy*dy - heigth*heigth / 16) <= heigth*heigth / 100) {
                row[j * 3]     = 255;
                row[j * 3 + 1] = 0;
                row[j * 3 + 2] = 0;
            } else if (dx*dx + dy*dy < heigth*heigth / 16) {
                row[j * 3]     = 0;
                row[j * 3 + 1] = 255;
                row[j * 3 + 2] = 255;
            } else if (dx*dx + dy*dy < heigth*heigth / 6) {
                row[j * 3]     = (i + width - j) * 10 % 255;
                row[j * 3 + 1] = 255;
                row[j * 3 + 2] = (i + j) * 10 % 255;
            } else {
                row[j * 3]     = 255;
                row[j * 3 + 1] = (i + j) * 5 % 255;
                row[j * 3 + 2] = 255;
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

int main(int argc, char** argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s OBJ-FILE OUT-FILE\n", argv[0]);
        return EXIT_FAILURE;
    } else {
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
            Scene scene = loadFromObj(data);
            free(data);

            freeScene(scene); 
            if (!writePNGFile(argv[2], NULL, 500, 500)) {
                fprintf(stderr, "failed to write '%s': %s\n", argv[2], strerror(errno));
            }
            return EXIT_SUCCESS;
        }
    }
}
