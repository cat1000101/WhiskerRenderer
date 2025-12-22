#include "glyf.h"

int parseGlyf(W_Parser *parser);

int glyfFromTD(W_Parser *parser, TableDirectory glyfTD) {
    parser->tables.glyf.glyfStartOffset = glyfTD.offset;
    uint8_t *tempView = &parser->fontFile.data[glyfTD.offset];
    (void)tempView;

    return 0;
}
