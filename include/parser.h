#ifndef PARSER_H
#define PARSER_H

#include <stdint.h>
#include <stdio.h>

#define big16ToLittle16(x) ((x & 0xFF00) >> 8 | (x & 0x00FF) << 8)
#define big32ToLittle32(x) (big16ToLittle16(x & 0xFFFF) << 16 | big16ToLittle16((x & 0xFFFF0000) >> 16))

typedef struct {
    char *name;
} Font;

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
// 'name'	naming
// 'post'	PostScript

typedef uint16_t shortFrac; // 16-bit signed fraction
typedef uint32_t Fixed;     // 16.16-bit signed fixed-point number
typedef int16_t FWord;      // 16-bit signed integer that describes a quantity in FUnits, the smallest measurable distance in em space.
typedef uint16_t uFWord;    // 16-bit unsigned integer that describes a quantity in FUnits, the smallest measurable distance in em space.
typedef uint16_t F2Dot14;   // 16-bit signed fixed number with the low 14 bits representing fraction.
typedef uint16_t longDateTime;

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

uint32_t calcTableChecksum(uint32_t *table, uint32_t numberOfBytesInTable);

Font *parseFont(FILE *fontFile);

#endif