#ifndef RENDERER_H
#define RENDERER_H

#include <stdint.h>

#include "whiskerRenderer.h"

typedef struct {
    uint8_t *bitmap;
    size_t width;
    size_t hight;
} charBitmap;

int drawString_i(W_Font *font, char *character);

#endif
