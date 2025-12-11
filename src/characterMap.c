#include "characterMap.h"
#include "parser.h"
#include "utils.h"

CmapFormat4 *parseCmapFormat4(W_Parser *parser, size_t formatOffset) {
    uint8_t *tempView = &parser->fontFile.data[formatOffset];
    CmapFormat4 *formatTable = SAFE_MALLOC(sizeof(CmapFormat4));
    formatTable->format = read_uint16_t_endian(&tempView[OFFSET_OF(CmapFormat4, format)]);
    formatTable->length = read_uint16_t_endian(&tempView[OFFSET_OF(CmapFormat4, length)]);
    formatTable->language = read_uint16_t_endian(&tempView[OFFSET_OF(CmapFormat4, language)]);
    formatTable->segCountX2 = read_uint16_t_endian(&tempView[OFFSET_OF(CmapFormat4, segCountX2)]);
    formatTable->searchRange = read_uint16_t_endian(&tempView[OFFSET_OF(CmapFormat4, searchRange)]);
    formatTable->entrySelector = read_uint16_t_endian(&tempView[OFFSET_OF(CmapFormat4, entrySelector)]);
    formatTable->rangeShift = read_uint16_t_endian(&tempView[OFFSET_OF(CmapFormat4, rangeShift)]);
    formatTable->segmentsLength = read_uint16_t_endian(&tempView[OFFSET_OF(CmapFormat4, segmentsLength)]);

    return formatTable;
}

Cmap cmapFromTD(W_Parser *parser, TableDirectory cmapTD) {
    uint8_t *tempView = &parser->fontFile.data[cmapTD.offset];
    Cmap returnTable = (Cmap){0};
    CmapSubtable subtable = (CmapSubtable){0};
    size_t i = 0;
    uint16_t id = 0;
    uint16_t sid = 0;

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
            subtable.platformID = id;
            subtable.platformSpecificID = sid;
            subtable.offset = read_uint32_t_endian(
                &tempView[4 + sizeof(CmapSubtable) * i + OFFSET_OF(CmapSubtable, offset)]);
            break;

        case UNICODEI_DENTIFIERS_UNICODE_BMP:
            subtable.platformID = id;
            subtable.platformSpecificID = sid;
            subtable.offset = read_uint32_t_endian(
                &tempView[4 + sizeof(CmapSubtable) * i + OFFSET_OF(CmapSubtable, offset)]);
            continue;
        default:
            continue;
        }
        break;
    }
    if (IS_ZERO(subtable)) {
        UNREACHABLE("wasn't able to find suitable encoding\n");
    }
    // printf("platform id: %d platform specific id: %d offset: %08X\n",
    //        subtable.platformID,
    //        subtable.platformSpecificID,
    //        subtable.offset);

    returnTable.cmapFormat = ({
        void *temp = NULL;
        switch (read_uint16_t_endian(&tempView[subtable.offset])) {
        case 4:
            temp = parseCmapFormat4(parser, cmapTD.offset + subtable.offset);
            break;
        case 12:
            TODO("make parse format 12\n");
            break;
        default:
            UNREACHABLE("format table not supported\n");
        }
        temp;
    });

    return returnTable;
}
