#ifndef PARSER_H
#define PARSER_H

#include <stdint.h>
#include <stdio.h>

#include "whiskerRendererTypes.h"

typedef struct {
    uint32_t scalerType;
    uint16_t numTables;
    uint16_t searchRange;
    uint16_t entrySelector;
    uint16_t rangeShift;
} OffsetSubTable;

size_t getGlyfOffset(W_Parser *parser, size_t index);
int parseFont_i(MappedFile fontFile, W_Parser *parser);

#endif
