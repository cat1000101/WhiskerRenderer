#ifndef GLYF_H
#define GLYF_H

#include <stdint.h>

#include "parser.h"

typedef struct {
    // If the number of contours is positive or zero, it is a single glyph;
    // If the number of contours less than zero, the glyph is compound
    int16_t numberOfContours;
    int16_t xMin; //	Minimum x for coordinate data
    int16_t yMin; //	Minimum y for coordinate data
    int16_t xMax; //	Maximum x for coordinate data
    int16_t yMax; //	Maximum y for coordinate data
} GlyphDescription;

typedef struct {
    uint8_t bits;
} OutlineFlags;

typedef enum {
    // If set, the point is on the curve;
    // Otherwise, it is off the curve.
    OUTLINE_FLAGS_ON_CURVE = 1 << 0,
    // If set, the corresponding x-coordinate is 1 byte long;
    // Otherwise, the corresponding x-coordinate is 2 bytes long
    OUTLINE_FLAGS_X_SHORT_VECTOR = 1 << 1,
    // If set, the corresponding y-coordinate is 1 byte long;
    // Otherwise, the corresponding y-coordinate is 2 bytes long
    OUTLINE_FLAGS_Y_SHORT_VECTOR = 1 << 2,
    // If set, the next byte specifies the number of additional times this set of flags is to be repeated. In this way, the number of flags listed can be smaller than the number of points in a character.
    OUTLINE_FLAGS_REPEAT = 1 << 3,
    // (Positive x-Short vector)
    // This flag has one of two meanings, depending on how the x-Short Vector flag is set.
    // If the x-Short Vector bit is set, this bit describes the sign of the value, with a value of 1 equalling positive and a zero value negative.
    // If the x-short Vector bit is not set, and this bit is set, then the current x-coordinate is the same as the previous x-coordinate.
    // If the x-short Vector bit is not set, and this bit is not set, the current x-coordinate is a signed 16-bit delta vector. In this case, the delta vector is the change in x
    OUTLINE_FLAGS_THIS_X_IS_SAME = 1 << 4,
    // (Positive y-Short vector)
    // This flag has one of two meanings, depending on how the y-Short Vector flag is set.
    // If the y-Short Vector bit is set, this bit describes the sign of the value, with a value of 1 equalling positive and a zero value negative.
    // If the y-short Vector bit is not set, and this bit is set, then the current y-coordinate is the same as the previous y-coordinate.
    // If the y-short Vector bit is not set, and this bit is not set, the current y-coordinate is a signed 16-bit delta vector. In this case, the delta vector is the change in y
    OUTLINE_FLAGS_THIS_Y_IS_SAME = 1 << 5,
    // Set to zero
    OUTLINE_FLAGS_RESERVED = 3 << 6,
} OutlineFlagsBits;

typedef struct {
    uint16_t bits;
} ComponentFlags;

typedef enum {
    // If set, the arguments are words;
    // If not set, they are bytes.
    COMPONENT_FLAGS_ARG_1_AND_2_ARE_WORDS = 1 << 0,
    // If set, the arguments are xy values;
    // If not set, they are points.
    COMPONENT_FLAGS_ARGS_ARE_XY_VALUES = 1 << 1,
    // If set, round the xy values to grid;
    // if not set do not round xy values to grid (relevant only to bit 1 is set)
    COMPONENT_FLAGS_ROUND_XY_TO_GRID = 1 << 2,
    // If set, there is a simple scale for the component.
    // If not set, scale is 1.0.
    COMPONENT_FLAGS_WE_HAVE_A_SCALE = 1 << 3,
    COMPONENT_FLAGS_OBSOLETE = 1 << 4,        // (obsolete; set to zero)
    COMPONENT_FLAGS_MORE_COMPONENTS = 1 << 5, // If set, at least one additional glyph follows this one.
    COMPONENT_FLAGS_WE_HAVE_AN_X_AND_Y_SCALE =
        1 << 6, // If set the x direction will use a different scale than the y direction.
    COMPONENT_FLAGS_WE_HAVE_A_TWO_BY_TWO =
        1 << 7, // If set there is a 2-by-2 transformation that will be used to scale the component.
    COMPONENT_FLAGS_WE_HAVE_INSTRUCTIONS =
        1 << 8, // If set, instructions for the component character follow the last component.
    COMPONENT_FLAGS_USE_MY_METRICS = 1 << 9,    // Use metrics from this component for the compound glyph.
    COMPONENT_FLAGS_OVERLAP_COMPOUND = 1 << 10, // If set, the components of this compound glyph overlap.
} ComponentFlagsBits;

typedef struct {
    uint16_t
        *endPtsOfContours; //	Array of last points of each contour; n is the number of contours; array entries are point indices
    uint16_t instructionLength; // Total number of bytes needed for instructions
    uint8_t *instructions;      //	Array of instructions for this glyph, length = instructionLength
    OutlineFlags *flags;             //	Array of flags
    uint8_t *
        xCoordinates; // or int16_t // Array of x-coordinates; the first is relative to (0,0), others are relative to previous point
    uint8_t *
        yCoordinates; // or int16_t // Array of y-coordinates; the first is relative to (0,0), others are relative to previous point
} SimpleGlyfData;

typedef struct {
    ComponentFlags flags;      // 	Component flag
    uint16_t glyphIndex; // 	Glyph index of component
    // int16, int8 or uint8,	X-offset for component or point number; type depends on bits 0 and 1 in component flags
    uint16_t argument1;
    // int16, int8 or uint8,	Y-offset for component or point number type depends on bits 0 and 1 in component flags
    uint16_t argument2;
} ComponentGlyfData;

int glyfFromTD(W_Parser *parser, TableDirectory cmapTD);

#endif
