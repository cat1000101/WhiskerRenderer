#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>

typedef struct {
    char* name;
} Font;

Font* parseFont(FILE* fontFile);

#endif