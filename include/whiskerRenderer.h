#ifndef WHISKER_RENDERER_H
#define WHISKER_RENDERER_H

#include "whiskerRendererTypes.h"

typedef struct {
    char *name;
    W_Parser parser;
} W_Font;

W_Font *parseFont(MappedFile fontFile);
int drawString(W_Font *font, char *characters);

int parseArgs(int argc, char *argv[]);
int mapFile(int fd, MappedFile *mappedFile);
void unmapFile(MappedFile mf);

#endif
