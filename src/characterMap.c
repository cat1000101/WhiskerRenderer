#include "characterMap.h"
#include "parser.h"
#include "utils.h"

Cmap cmapFromTD(W_Parser *parser, TableDirectory cmapTD) {
    Cmap returnTable = {0};
    size_t i = 0;
    uint8_t *tempView = &parser->fontFile.data[cmapTD.offset];

    returnTable.version = read_uint16_t_endian(tempView);
    returnTable.numberSubtables = read_uint16_t_endian(&tempView[OFFSET_OF(Cmap, numberSubtables)]);
    returnTable.subtables = SAFE_MALLOC(sizeof(CmapSubtable) * returnTable.numberSubtables);

    for (i = 0; i < returnTable.numberSubtables; i++) {
        returnTable.subtables[i].platformID = read_uint16_t_endian(&tempView[sizeof(Cmap) + sizeof(CmapSubtable) * i + OFFSET_OF(CmapSubtable, platformID)]);
        returnTable.subtables[i].platformSpecificID = read_uint16_t_endian(&tempView[sizeof(Cmap) + sizeof(CmapSubtable) * i + OFFSET_OF(CmapSubtable, platformSpecificID)]);
        returnTable.subtables[i].offset = read_uint32_t_endian(&tempView[sizeof(Cmap) + sizeof(CmapSubtable) * i + OFFSET_OF(CmapSubtable, offset)]);
    }

    return returnTable;
}