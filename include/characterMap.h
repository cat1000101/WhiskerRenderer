#ifndef CHARACTER_MAP_H
#define CHARACTER_MAP_H

#include <stdint.h>

#include "parser.h"

typedef enum {
    PLATFORMS_UNICODE = 0,   // Indicates Unicode version.
    PLATFORMS_MACINTOSH = 1, // Script Manager code.
    PLATFORMS_RESERVED = 2,  // do not use
    PLATFORMS_MICROSOFT = 3  // Microsoft encoding.
} Platforms;

typedef enum {
    UNICODEI_DENTIFIERS_VERSION10 = 0,                   // 1.0 semantics
    UNICODEI_DENTIFIERS_VERSION11 = 1,                   // 1.1 semantics
    UNICODEI_DENTIFIERS_ISO = 2,                         // 10646 1993 semantics (deprecated)
    UNICODEI_DENTIFIERS_UNICODE_BMP = 3,                 // 2.0 or later semantics (BMP only)
    UNICODEI_DENTIFIERS_UNICODE_EXTENDED = 4,            // 2.0 or later semantics (non-BMP characters allowed)
    UNICODEI_DENTIFIERS_UNICODE_VARIATION_SEQUENCES = 5, // Variation Sequences
    UNICODEI_DENTIFIERS_LASTRESORT = 6,
} UnicodeIdentifiers;

typedef struct {
    uint16_t format;         // Format number is set to 4
    uint16_t length;         // Length of subtable in bytes
    uint16_t language;       // Language code (see above)
    uint16_t segCountX2;     // 2 * segCount
    uint16_t searchRange;    // 2 * (2**FLOOR(log2(segCount)))
    uint16_t entrySelector;  // log2(searchRange/2)
    uint16_t rangeShift;     // (2 * segCount) - searchRange
    uint16_t segmentsLength; // segmentsLength = length - 14
    uint16_t *segments;      // encompassing array for all tables
    uint16_t *endCode;       // [segCount]	Ending character code for each segment, last = 0xFFFF.
    // UInt16	reservedPad	This value should be zero
    uint16_t *startCode;       // [segCount]	Starting character code for each segment
    uint16_t *idDelta;         // [segCount]	Delta for all character codes in segment
    uint16_t *idRangeOffset;   // [segCount]	Offset in bytes to glyph indexArray, or 0
    uint16_t *glyphIndexArray; // [variable]	Glyph index array, variable = (length - 16 - 4 * segCountX2)/2
} CmapFormat4;

typedef struct {
    uint32_t startCharCode;  // First character code in this group
    uint32_t endCharCode;    // Last character code in this group
    uint32_t startGlyphCode; // Glyph index corresponding to the starting character code; subsequent charcters are mapped to sequential glyphs
} CmapFormat12Group;

typedef struct {
    uint16_t format;   // Subtable format; set to 12
    uint16_t reserved; // Set to 0.
    uint32_t length;   // Byte length of this subtable (including the header)
    uint32_t language; // Language code (see above)
    uint32_t nGroups;  // Number of groupings which follow
    CmapFormat12Group *groups;
} CmapFormat12;

typedef struct {
    uint16_t format;
} IdentifyFormat;

typedef struct {
    uint16_t platformID;         // Platform identifier
    uint16_t platformSpecificID; // Platform-specific encoding identifier
    uint32_t offset;             // Offset of the mapping table
} CmapSubtable;

int cmapFromTD(W_Parser *parser, TableDirectory cmapTD);

#endif