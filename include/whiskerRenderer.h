#ifndef WHISKER_RENDERER_H
#define WHISKER_RENDERER_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint8_t *data;
    size_t size;
} MappedFile;

typedef struct {
    uint8_t *bitmap;
    size_t width;
    size_t height;
    uint16_t advanceWidth;
    int16_t leftSideBearing;
    int16_t verticalAdjustment;
} charBitmap;

void *parseFont(MappedFile fontFile);
charBitmap getBitmapForChar(void *font, char c, size_t px);
void raylibDrawString(void *font, char *s, size_t px, int posX, int posY);

int mapFile(int fd, MappedFile *mappedFile);
void unmapFile(MappedFile mf);

#endif
