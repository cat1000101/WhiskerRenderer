#include "characterMap.h"
#include "parser.h"
#include "utils.h"

uint16_t getGlyphIndex(W_Parser *parser, uint16_t c) {
    size_t format = ((IdentifyFormat *)parser->tables.cmap.cmapFormat)->format;
    size_t i = 0;

    if (format == 4) {
        CmapFormat4 *formatSubtable = parser->tables.cmap.cmapFormat;
        for (i = 0; i < formatSubtable->segCount; i++) {
            if (formatSubtable->endCode[i] < c) continue;
            if (formatSubtable->startCode[i] > c) return 0;
            if (formatSubtable->idRangeOffset[i]) {
                return *(&formatSubtable->idRangeOffset[i] + formatSubtable->idRangeOffset[i] / 2
                         + (c - formatSubtable->startCode[i]));
            } else {
                return formatSubtable->idDelta[i] + c;
            }
        }
    } else if (format == 12) {
        TODO("implument format 12 get glyph index");
    } else {
        UNREACHABLE("used format that is not implumented %zd", format);
    }
    return 0;
}

void parseCmapFormat4(W_Parser *parser, size_t formatOffset) {
    uint8_t *tempView = &parser->fontFile.data[formatOffset];
    CmapFormat4 *result = SAFE_MALLOC(sizeof(CmapFormat4));
    result->format = read_uint16_t_endian(&tempView[OFFSET_OF(CmapFormat4, format)]);
    result->length = read_uint16_t_endian(&tempView[OFFSET_OF(CmapFormat4, length)]);
    result->language = read_uint16_t_endian(&tempView[OFFSET_OF(CmapFormat4, language)]);
    result->segCountX2 = read_uint16_t_endian(&tempView[OFFSET_OF(CmapFormat4, segCountX2)]);
    result->searchRange = read_uint16_t_endian(&tempView[OFFSET_OF(CmapFormat4, searchRange)]);
    result->entrySelector = read_uint16_t_endian(&tempView[OFFSET_OF(CmapFormat4, entrySelector)]);
    result->rangeShift = read_uint16_t_endian(&tempView[OFFSET_OF(CmapFormat4, rangeShift)]);

    size_t i = 0;
    result->segCount = result->segCountX2 / 2;
    result->segmentsLength = result->length - 14 - 8;
    result->segments = SAFE_MALLOC(result->segmentsLength);
    result->endCode = result->segments;
    result->startCode = &result->segments[result->segCount + 1];
    result->idDelta = &result->segments[result->segCount * 2 + 1];
    result->idRangeOffset = &result->segments[result->segCount * 3 + 1];
    result->glyphIndexArray = &result->segments[result->segCount * 4 + 1];
    tempView = &tempView[14];
    for (i = 0; i < result->segmentsLength / 2; i++) {
        result->segments[i] = read_uint16_t_endian(&tempView[i * 2]);
    }

    parser->tables.cmap.cmapFormat = result;
}

int cmapFromTD(W_Parser *parser, TableDirectory cmapTD) {
    uint8_t *tempView = &parser->fontFile.data[cmapTD.offset];
    Cmap *cmapView = &parser->tables.cmap;
    CmapSubtable subtable = (CmapSubtable){0};
    size_t i = 0;
    uint16_t id = 0;
    uint16_t sid = 0;

    cmapView->version = read_uint16_t_endian(tempView);
    cmapView->numberSubtables = read_uint16_t_endian(&tempView[OFFSET_OF(Cmap, numberSubtables)]);

    for (i = 0; i < cmapView->numberSubtables; i++) {
        id = read_uint16_t_endian(&tempView[4 + sizeof(CmapSubtable) * i + OFFSET_OF(CmapSubtable, platformID)]);
        switch (id) {
        case PLATFORMS_UNICODE:
            break;
        default:
            continue;
        }

        sid =
            read_uint16_t_endian(&tempView[4 + sizeof(CmapSubtable) * i + OFFSET_OF(CmapSubtable, platformSpecificID)]);
        switch (sid) {
        case UNICODEI_DENTIFIERS_VERSION10:
        case UNICODEI_DENTIFIERS_VERSION11:
        case UNICODEI_DENTIFIERS_UNICODE_EXTENDED:
            subtable.platformID = id;
            subtable.platformSpecificID = sid;
            subtable.offset =
                read_uint32_t_endian(&tempView[4 + sizeof(CmapSubtable) * i + OFFSET_OF(CmapSubtable, offset)]);
            break;

        case UNICODEI_DENTIFIERS_UNICODE_BMP:
            subtable.platformID = id;
            subtable.platformSpecificID = sid;
            subtable.offset =
                read_uint32_t_endian(&tempView[4 + sizeof(CmapSubtable) * i + OFFSET_OF(CmapSubtable, offset)]);
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
    // subtable.platformID,
    // subtable.platformSpecificID,
    // subtable.offset);

    switch (read_uint16_t_endian(&tempView[subtable.offset])) {
    case 4:
        parseCmapFormat4(parser, cmapTD.offset + subtable.offset);
        break;
    case 12:
        TODO("make parse format 12");
        break;
    default:
        ERROR_OUT("format table not supported");
    }
    if (!cmapView->cmapFormat) {
        return 1;
    }

    // printf("the char 'a' to glyph index %04X\n", getGlyphIndex(parser, 'a'));

    return 0;
}
