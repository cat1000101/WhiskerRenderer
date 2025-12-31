#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "characterMap.h"
#include "glyf.h"
#include "utils.h"
#include "whiskerRendererTypes.h"

// X_Y = 0 x coordinates, X_Y = 1 y coordinates
size_t parseCordinateWithFlag(uint8_t *view, int16_t *coordinate, uint8_t *flags, size_t pointCount, uint8_t X_Y) {
    uint8_t ifShort = X_Y ? OUTLINE_FLAGS_Y_SHORT_VECTOR : OUTLINE_FLAGS_X_SHORT_VECTOR;
    uint8_t ifSame = X_Y ? OUTLINE_FLAGS_THIS_Y_IS_SAME : OUTLINE_FLAGS_THIS_X_IS_SAME;
    uint8_t *tempView = view;
    int16_t absolute = 0;
    for (size_t i = 0; i < pointCount; i++) {
        if (flags[i] & ifShort) {
            if (flags[i] & ifSame) {
                absolute += *tempView++;
            } else {
                absolute -= *tempView++;
            }
        } else {
            if (!(flags[i] & ifSame)) {
                absolute += read_int16_t_endian(tempView);
                tempView += sizeof(int16_t);
            }
        }
        coordinate[i] = absolute;
        // printf("%c %zd: %d\n",X_Y ? 'Y' : 'X', i, coordinate[i]);
    }
    return (size_t)(tempView - view);
}

int parseGlyf(W_Parser *parser, size_t charValue, SimpleGlyfChar *glyfResult) {
    size_t index = getGlyphIndex(parser, (uint16_t)charValue);
    size_t glyfOffset = getGlyfOffset(parser, index);
    size_t glyfSize = getGlyfOffset(parser, index + 1) - glyfOffset;
    uint8_t *tempView = &parser->fontFile.data[parser->tables.glyf.glyfStartOffset + glyfOffset];
    size_t i = 0;

    if (!glyfSize) {
        // ERROR_OUT("empty glyf, need to implement empty length thingy");
        return 1;
    }

    int16_t tempContourNum = read_int16_t_endian(tempView);
    glyfResult->boundingBox.xMin = read_int16_t_endian(&tempView[OFFSET_OF(GlyphDescription, xMin)]);
    glyfResult->boundingBox.yMin = read_int16_t_endian(&tempView[OFFSET_OF(GlyphDescription, yMin)]);
    glyfResult->boundingBox.xMax = read_int16_t_endian(&tempView[OFFSET_OF(GlyphDescription, xMax)]);
    glyfResult->boundingBox.yMax = read_int16_t_endian(&tempView[OFFSET_OF(GlyphDescription, yMax)]);
    glyfResult->charValue = charValue;
    tempView += sizeof(GlyphDescription);

    // printf("glyf %c:contour number: %d, bounding box: (%d, %d)min (%d, %d)max sizeof glyf: %zd, index: %zd\n",
    // (char)glyfResult->charValue, tempContourNum, glyfResult->boundingBox.xMin,
    // glyfResult->boundingBox.yMin, glyfResult->boundingBox.xMax, glyfResult->boundingBox.yMax, glyfSize, index);

    if (tempContourNum < 0) {
        // ERROR_OUT("Component glyfs not implemented");
        return 1;
    }
    glyfResult->contourNum = tempContourNum;

    uint16_t *endPtsOfContours = SAFE_MALLOC(sizeof(uint16_t) * glyfResult->contourNum);
    for (i = 0; i < glyfResult->contourNum; i++) {
        endPtsOfContours[i] = read_uint16_t_endian(&tempView[i * sizeof(uint16_t)]);
    }
    tempView += sizeof(uint16_t) * glyfResult->contourNum;

    // instructions, i am not implementing that for now
    tempView += sizeof(uint16_t) + read_uint16_t_endian(tempView);

    size_t maxPointIndex = endPtsOfContours[glyfResult->contourNum - 1];
    size_t pointCount = maxPointIndex + 1;
    uint8_t *flags = SAFE_MALLOC(sizeof(uint8_t) * pointCount);
    int16_t *xPoints = SAFE_MALLOC(sizeof(int16_t) * pointCount);
    int16_t *yPoints = SAFE_MALLOC(sizeof(int16_t) * pointCount);

    // if (getPrintChar((char)charValue))
    //     printf("%c\n", (char)charValue);
    // else
    //     printf("not printable\n");

    size_t flagsToParse = pointCount;
    uint8_t flag = 0;
    uint8_t repeat = 0;
    i = 0;
    while (flagsToParse > 0) {
        flag = tempView[i++];
        if (isFlagBitSet(flag, 3)) {
            // flag = flag & (~OUTLINE_FLAGS_REPEAT);
            for (repeat = tempView[i++] + 1; repeat > 0; repeat--) {
                flags[pointCount - flagsToParse--] = flag;
            }
        } else {
            flags[pointCount - flagsToParse--] = flag;
        }
    }
    tempView += sizeof(uint8_t) * i;
    tempView += parseCordinateWithFlag(tempView, xPoints, flags, pointCount, 0);
    tempView += parseCordinateWithFlag(tempView, yPoints, flags, pointCount, 1);

    glyfResult->contours = SAFE_MALLOC(sizeof(struct Contours) * glyfResult->contourNum);
    uint16_t startPoint = 0;
    size_t j = 0;
    i16Vec xVec = {0};
    i16Vec yVec = {0};
    u8Vec flagVec = {0};
    uint8_t nextFlag;
    int16_t currX, currY, nextX, nextY, contourLength;
    for (i = 0; i < glyfResult->contourNum; i++) {
        i16Vec_init(&xVec);
        i16Vec_init(&yVec);
        u8Vec_init(&flagVec);
        contourLength = endPtsOfContours[i] - startPoint + 1;
        for (j = 0; j < contourLength; j++) {
            currX = xPoints[startPoint + j];
            currY = yPoints[startPoint + j];
            nextFlag = flags[startPoint + ((j + 1) % contourLength)];
            i16Vec_push(&xVec, currX);
            i16Vec_push(&yVec, currY);
            u8Vec_push(&flagVec, flags[startPoint + j]);

            if ((flagVec.data[flagVec.len - 1] & 1) == (nextFlag & 1)) {
                nextX = xPoints[startPoint + ((j + 1) % contourLength)];
                nextY = yPoints[startPoint + ((j + 1) % contourLength)];
                i16Vec_push(&xVec, (currX + nextX) / 2);
                i16Vec_push(&yVec, (currY + nextY) / 2);
                u8Vec_push(&flagVec, 1 - (nextFlag & 1));
            }
        }

        glyfResult->contours[i].flags = flagVec.data;
        glyfResult->contours[i].xFontUnit = xVec.data;
        glyfResult->contours[i].yFontUnit = yVec.data;
        glyfResult->contours[i].length = yVec.len;
        // printf("contour %zd: startPts %d, endPts %d, length %d\n", i, startPoint, endPtsOfContours[i], endPtsOfContours[i] - startPoint + 1);
        startPoint = endPtsOfContours[i] + 1;
    }
    free(flags);
    free(xPoints);
    free(yPoints);
    free(endPtsOfContours);
    return 0;
}

int glyfFromTD(W_Parser *parser, TableDirectory glyfTD) {
    parser->tables.glyf.glyfStartOffset = glyfTD.offset;
    parser->tables.glyf.chars = SAFE_MALLOC(sizeof(SimpleGlyfChar) * 256); // 256 ascii chars
    parser->tables.glyf.charNum = 256;
    for (size_t i = 0; i < parser->tables.glyf.charNum; i++) {
        if (parseGlyf(parser, i, &parser->tables.glyf.chars[i])) {
            // printf("glyf for '%zd' not found\n", i);
            memcpy(&parser->tables.glyf.chars[i], &parser->tables.glyf.chars[0], sizeof(SimpleGlyfChar));
        }
    }

    return 0;
}
