#ifndef PARSER_H
#define PARSER_H

#include <stdint.h>
#include <stdio.h>

#include "utils.h"

typedef struct {
    uint8_t x, y;
} ShortVec2;

typedef struct {
    uint16_t x, y;
} Vec2;

typedef struct {
    uint32_t scalerType;
    uint16_t numTables;
    uint16_t searchRange;
    uint16_t entrySelector;
    uint16_t rangeShift;
} OffsetSubTable;

typedef struct {
    uint32_t tag;
    uint32_t checkSum;
    uint32_t offset;
    uint32_t length;
} TableDirectory;

// 'cmap'	character to glyph mapping
// 'glyf'	glyph data
// 'head'	font header
// 'hhea'	horizontal header
// 'hmtx'	horizontal metrics
// 'loca'	index to location
// 'maxp'	maximum profile
// 'name'	naming - not needed
// 'post'	PostScript - not needed

typedef uint16_t shortFrac; // 16-bit signed fraction
typedef uint32_t Fixed;     // 16.16-bit signed fixed-point number
typedef int16_t FWord;      // 16-bit signed integer that describes a quantity in FUnits, the smallest measurable distance in em space.
typedef uint16_t uFWord;    // 16-bit unsigned integer that describes a quantity in FUnits, the smallest measurable distance in em space.
typedef uint16_t F2Dot14;   // 16-bit signed fixed number with the low 14 bits representing fraction.
typedef int64_t longDateTime;

typedef struct {
    Fixed version;               // 0x00010000 if (version 1.0)
    Fixed fontRevision;          // set by font manufacturer
    uint32_t checkSumAdjustment; // https://developer.apple.com/fonts/TrueType-Reference-Manual/RM06/Chap6head.html
    uint32_t magicNumber;        // set to 0x5F0F3CF5
    uint16_t flags;              // https://developer.apple.com/fonts/TrueType-Reference-Manual/RM06/Chap6head.html
    uint16_t unitsPerEm;         // range from 64 to 16384
    longDateTime created;        // international date
    longDateTime modified;       // international date
    FWord xMin;                  // for all glyph bounding boxes
    FWord yMin;                  // for all glyph bounding boxes
    FWord xMax;                  // for all glyph bounding boxes
    FWord yMax;                  // for all glyph bounding boxes
    uint16_t macStyle;           // bit 0 bold bit 1 italic bit 2 underline bit 3 outline bit 4 shadow bit 5 condensed (narrow) bit 6 extended
    uint16_t lowestRecPPEM;      // smallest readable size in pixels
    int16_t fontDirectionHint;   // 0 Mixed directional glyphs 1 Only strongly left to right glyphs 2 Like 1 but also contains neutrals -1 Only strongly right to left glyphs -2 Like -1 but also contains neutrals
    int16_t indexToLocFormat;    // 0 for short offsets, 1 for long
    int16_t glyphDataFormat;     // 0 for current format
} Head;

typedef struct {
    Fixed version;                  // 0x00010000 (1.0)
    uint16_t numGlyphs;             // the number of glyphs in the font
    uint16_t maxPoints;             // points in non-compound glyph
    uint16_t maxContours;           // contours in non-compound glyph
    uint16_t maxComponentPoints;    // points in compound glyph
    uint16_t maxComponentContours;  // contours in compound glyph
    uint16_t maxZones;              // set to 2
    uint16_t maxTwilightPoints;     // points used in Twilight Zone (Z0)
    uint16_t maxStorage;            // number of Storage Area locations
    uint16_t maxFunctionDefs;       // number of FDEFs
    uint16_t maxInstructionDefs;    // number of IDEFs
    uint16_t maxStackElements;      // maximum stack depth
    uint16_t maxSizeOfInstructions; // byte count for glyph instructions
    uint16_t maxComponentElements;  // number of glyphs referenced at top level
    uint16_t maxComponentDepth;     // levels of recursion, set to 0 if font has only simple glyphs
} Maxp;

typedef struct {
    Fixed version;                // 0x00010000 (1.0)
    FWord ascent;                 // Distance from baseline of highest ascender
    FWord descent;                // Distance from baseline of lowest descender
    FWord lineGap;                // typographic line gap
    uFWord advanceWidthMax;       // must be consistent with horizontal metrics
    FWord minLeftSideBearing;     // must be consistent with horizontal metrics
    FWord minRightSideBearing;    // must be consistent with horizontal metrics
    FWord xMaxExtent;             // max(lsb + (xMax-xMin))
    int16_t caretSlopeRise;       // used to calculate the slope of the caret (rise/run) set to 1 for vertical caret
    int16_t caretSlopeRun;        // 0 for vertical
    FWord caretOffset;            // set value to 0 for non-slanted fonts
    int16_t reserved1;            // set value to 0
    int16_t reserved2;            // set value to 0
    int16_t reserved3;            // set value to 0
    int16_t reserved4;            // set value to 0
    int16_t metricDataFormat;     // 0 for current format
    uint16_t numOfLongHorMetrics; // number of advance widths in metrics table
} Hhea;

typedef struct {
    uint16_t platformID;         // Platform identifier
    uint16_t platformSpecificID; // Platform-specific encoding identifier
    uint32_t offset;             // Offset of the mapping table
} CmapSubtable;

typedef struct {
    uint16_t version;         // Version number (Set to zero)
    uint16_t numberSubtables; // Number of encoding subtables
    CmapSubtable subtable;
} Cmap;

typedef struct {
    struct {
        uint16_t advanceWidth;
        int16_t leftSideBearing;
    } *hMetrics;
    size_t hMetricsLen;        // numOfLongHorMetrics
    size_t leftSideBearingLen; // number of entries = total number of glyphs minus numOfLongHorMetrics
} Hmtx;

typedef struct {
    union {
        uint16_t *shortOffsets;
        uint32_t *longOffsets;
    } offsets;
    size_t len;
} Loca;

typedef struct {
    char *name;
} W_Font;

typedef struct {
    mappedFile fontFile;
    uint16_t numTables;
    struct {
        Head head;
        Cmap cmap;
        Maxp maxp;
        Loca loca;
        Hhea hhea;
        Hmtx hmtx;
    } tables;
} W_Parser;

W_Font *parseFont(mappedFile fontFile);

#endif