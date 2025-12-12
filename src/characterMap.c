#include "characterMap.h"
#include "parser.h"
#include "utils.h"

int parseCmapFormat4(W_Parser *parser, size_t formatOffset, CmapFormat4 *result) {
    uint8_t *tempView = &parser->fontFile.data[formatOffset];
    result->format = read_uint16_t_endian(&tempView[OFFSET_OF(CmapFormat4, format)]);
    result->length = read_uint16_t_endian(&tempView[OFFSET_OF(CmapFormat4, length)]);
    result->language = read_uint16_t_endian(&tempView[OFFSET_OF(CmapFormat4, language)]);
    result->segCountX2 = read_uint16_t_endian(&tempView[OFFSET_OF(CmapFormat4, segCountX2)]);
    result->searchRange = read_uint16_t_endian(&tempView[OFFSET_OF(CmapFormat4, searchRange)]);
    result->entrySelector = read_uint16_t_endian(&tempView[OFFSET_OF(CmapFormat4, entrySelector)]);
    result->rangeShift = read_uint16_t_endian(&tempView[OFFSET_OF(CmapFormat4, rangeShift)]);
    result->segmentsLength = read_uint16_t_endian(&tempView[OFFSET_OF(CmapFormat4, segmentsLength)]);

    return 0;
}

int cmapFromTD(W_Parser *parser, TableDirectory cmapTD, Cmap *result) {
    uint8_t *tempView = &parser->fontFile.data[cmapTD.offset];
    CmapSubtable subtable = (CmapSubtable){0};
    size_t i = 0;
    uint16_t id = 0;
    uint16_t sid = 0;

    result->version = read_uint16_t_endian(tempView);
    result->numberSubtables = read_uint16_t_endian(&tempView[OFFSET_OF(Cmap, numberSubtables)]);

    for (i = 0; i < result->numberSubtables; i++) {
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
        ERROR_OUT("wasn't able to find suitable encoding");
    }
    // printf("platform id: %d platform specific id: %d offset: %08X\n",
    //        subtable.platformID,
    //        subtable.platformSpecificID,
    //        subtable.offset);

    switch (read_uint16_t_endian(&tempView[subtable.offset])) {
    case 4:
        parseCmapFormat4(parser, cmapTD.offset + subtable.offset, result->cmapFormat);
        break;
    case 12:
        TODO("make parse format 12");
        break;
    default:
        ERROR_OUT("format table not supported");
    }

    return 0;
}
