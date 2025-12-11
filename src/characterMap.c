#include "characterMap.h"
#include "parser.h"
#include "utils.h"

Cmap cmapFromTD(W_Parser *parser, TableDirectory cmapTD) {
    Cmap returnTable = (Cmap){0};
    size_t i = 0;
    uint16_t id = 0;
    uint16_t sid = 0;
    uint8_t *tempView = &parser->fontFile.data[cmapTD.offset];

    returnTable.version = read_uint16_t_endian(tempView);
    returnTable.numberSubtables = read_uint16_t_endian(&tempView[OFFSET_OF(Cmap, numberSubtables)]);

    for (i = 0; i < returnTable.numberSubtables; i++) {
        id = read_uint16_t_endian(&tempView[4 + sizeof(CmapSubtable) * i + OFFSET_OF(CmapSubtable, platformID)]);
        switch (id) {
        case PLATFORMS_UNICODE:
            break;
        default:
            continue;
        }

        sid = read_uint16_t_endian(&tempView[4 + sizeof(CmapSubtable) * i + OFFSET_OF(CmapSubtable, platformSpecificID)]);
        switch (sid) {
        case UNICODEI_DENTIFIERS_VERSION10:
        case UNICODEI_DENTIFIERS_VERSION11:
        case UNICODEI_DENTIFIERS_UNICODE_EXTENDED:
            returnTable.subtable.platformID = id;
            returnTable.subtable.platformSpecificID = sid;
            returnTable.subtable.offset = read_uint32_t_endian(
                &tempView[4 + sizeof(CmapSubtable) * i + OFFSET_OF(CmapSubtable, offset)]);
            break;
            ;
        case UNICODEI_DENTIFIERS_UNICODE_BMP:
            returnTable.subtable.platformID = id;
            returnTable.subtable.platformSpecificID = sid;
            returnTable.subtable.offset = read_uint32_t_endian(
                &tempView[4 + sizeof(CmapSubtable) * i + OFFSET_OF(CmapSubtable, offset)]);
            continue;
        default:
            continue;
        }
        break;
    }
    if (IS_ZERO(returnTable.subtable)) {
        fprintf(stderr, "wasn't able to find sutable encoding\n");
        return (Cmap){0};
    }
    // printf("platform id: %d platform specific id: %d offset: %08X\n",
    //        returnTable.subtable.platformID,
    //        returnTable.subtable.platformSpecificID,
    //        returnTable.subtable.offset);
    return returnTable;
}