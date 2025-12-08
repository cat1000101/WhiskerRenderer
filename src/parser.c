#include <stdio.h>
#include <string.h>

#include "raylib.h"

#include "parser.h"
#include "utils.h"

Head headFromTD(W_Parser parser, TableDirectory headTD) {
    Head returnTable = {0};
    Head *table = (Head *)(&parser.fontFile.data[headTD.offset]);

    returnTable.version = SWAP_ENDIAN_32(table->version);
    returnTable.fontRevision = SWAP_ENDIAN_32(table->fontRevision);
    returnTable.checkSumAdjustment = SWAP_ENDIAN_32(table->checkSumAdjustment);
    returnTable.magicNumber = SWAP_ENDIAN_32(table->magicNumber);
    returnTable.flags = SWAP_ENDIAN_16(table->flags);
    returnTable.unitsPerEm = SWAP_ENDIAN_16(table->unitsPerEm);
    returnTable.created = table->created;
    returnTable.modified = table->modified;
    returnTable.xMin = (int16_t)SWAP_ENDIAN_16(table->xMin);
    returnTable.yMin = (int16_t)SWAP_ENDIAN_16(table->yMin);
    returnTable.xMax = (int16_t)SWAP_ENDIAN_16(table->xMax);
    returnTable.yMax = (int16_t)SWAP_ENDIAN_16(table->yMax);
    returnTable.macStyle = SWAP_ENDIAN_16(table->macStyle);
    returnTable.lowestRecPPEM = SWAP_ENDIAN_16(table->lowestRecPPEM);
    returnTable.fontDirectionHint = (int16_t)SWAP_ENDIAN_16(table->fontDirectionHint);
    returnTable.indexToLocFormat = (int16_t)SWAP_ENDIAN_16(table->indexToLocFormat);
    returnTable.glyphDataFormat = (int16_t)SWAP_ENDIAN_16(table->glyphDataFormat);

    return returnTable;
}
Maxp maxpFromTD(W_Parser parser, TableDirectory maxpTD) {
    Maxp returnTable = {0};
    Maxp *table = (Maxp *)(&parser.fontFile.data[maxpTD.offset]);

    returnTable.version = SWAP_ENDIAN_32(table->version);
    returnTable.numGlyphs = SWAP_ENDIAN_16(table->numGlyphs);
    returnTable.maxPoints = SWAP_ENDIAN_16(table->maxPoints);
    returnTable.maxContours = SWAP_ENDIAN_16(table->maxContours);
    returnTable.maxComponentPoints = SWAP_ENDIAN_16(table->maxComponentPoints);
    returnTable.maxComponentContours = SWAP_ENDIAN_16(table->maxComponentContours);
    returnTable.maxZones = SWAP_ENDIAN_16(table->maxZones);
    returnTable.maxTwilightPoints = SWAP_ENDIAN_16(table->maxTwilightPoints);
    returnTable.maxStorage = SWAP_ENDIAN_16(table->maxStorage);
    returnTable.maxFunctionDefs = SWAP_ENDIAN_16(table->maxFunctionDefs);
    returnTable.maxInstructionDefs = SWAP_ENDIAN_16(table->maxInstructionDefs);
    returnTable.maxStackElements = SWAP_ENDIAN_16(table->maxStackElements);
    returnTable.maxSizeOfInstructions = SWAP_ENDIAN_16(table->maxSizeOfInstructions);
    returnTable.maxComponentElements = SWAP_ENDIAN_16(table->maxComponentElements);
    returnTable.maxComponentDepth = SWAP_ENDIAN_16(table->maxComponentDepth);

    return returnTable;
}
Hhea hheaFromTD(W_Parser parser, TableDirectory hheaTD) {
    Hhea returnTable = {0};
    Hhea *table = (Hhea *)(&parser.fontFile.data[hheaTD.offset]);

    returnTable.version = SWAP_ENDIAN_32(table->version);
    returnTable.ascent = (int16_t)SWAP_ENDIAN_16(table->ascent);
    returnTable.descent = (int16_t)SWAP_ENDIAN_16(table->descent);
    returnTable.lineGap = (int16_t)SWAP_ENDIAN_16(table->lineGap);
    returnTable.advanceWidthMax = SWAP_ENDIAN_16(table->advanceWidthMax);
    returnTable.minLeftSideBearing = (int16_t)SWAP_ENDIAN_16(table->minLeftSideBearing);
    returnTable.minRightSideBearing = (int16_t)SWAP_ENDIAN_16(table->minRightSideBearing);
    returnTable.xMaxExtent = (int16_t)SWAP_ENDIAN_16(table->xMaxExtent);
    returnTable.caretSlopeRise = (int16_t)SWAP_ENDIAN_16(table->caretSlopeRise);
    returnTable.caretSlopeRun = (int16_t)SWAP_ENDIAN_16(table->caretSlopeRun);
    returnTable.caretOffset = (int16_t)SWAP_ENDIAN_16(table->caretOffset);
    returnTable.reserved1 = (int16_t)SWAP_ENDIAN_16(table->reserved1);
    returnTable.reserved2 = (int16_t)SWAP_ENDIAN_16(table->reserved2);
    returnTable.reserved3 = (int16_t)SWAP_ENDIAN_16(table->reserved3);
    returnTable.reserved4 = (int16_t)SWAP_ENDIAN_16(table->reserved4);
    returnTable.metricDataFormat = (int16_t)SWAP_ENDIAN_16(table->metricDataFormat);
    returnTable.numOfLongHorMetrics = SWAP_ENDIAN_16(table->numOfLongHorMetrics);

    return returnTable;
}
Loca locaFromTD(W_Parser parser, TableDirectory locaTD) {
    Loca returnTable = {0};
    size_t i = 0;

    returnTable.len = parser.tables.maxp.numGlyphs;
    uint8_t *tempView = &parser.fontFile.data[locaTD.offset];
    if (parser.tables.head.indexToLocFormat == 0) {
        returnTable.offsets.shortOffsets = malloc(sizeof(returnTable.offsets.shortOffsets) * returnTable.len);
        for (i = 0; i <= returnTable.len; i++) {
            returnTable.offsets.shortOffsets[i] = read_uint16_t_endian(tempView + i * sizeof(returnTable.offsets.shortOffsets));
        }
    } else {
        returnTable.offsets.longOffsets = malloc(sizeof(returnTable.offsets.longOffsets) * returnTable.len);
        for (i = 0; i <= returnTable.len; i++) {
            returnTable.offsets.longOffsets[i] = read_uint32_t_endian(tempView + i * sizeof(returnTable.offsets.longOffsets));
        }
    }

    return returnTable;
}
Hmtx hmtxFromTD(W_Parser parser, TableDirectory hmtxTD) {
    Hmtx returnTable = {0};
    size_t i = 0;
    uint8_t *tempView = &parser.fontFile.data[hmtxTD.offset];
    returnTable.hMetricsLen = parser.tables.Hhea.numOfLongHorMetrics;
    returnTable.leftSideBearingLen = parser.tables.maxp.numGlyphs - parser.tables.Hhea.numOfLongHorMetrics;

    returnTable.hMetrics = malloc(returnTable.hMetricsLen * sizeof(returnTable.hMetrics));
    if (!returnTable.hMetrics) {
        perror("hmtxFromBe");
        exit(1);
    }
    for (i = 0; i < returnTable.hMetricsLen; i++) {
        returnTable.hMetrics[i].advanceWidth = read_uint16_t_endian(tempView + i * 4);
        returnTable.hMetrics[i].leftSideBearing = read_int16_t_endian(tempView + i * 4 + 2);
    }
    for (i = returnTable.hMetricsLen; i < parser.tables.maxp.numGlyphs; i++) {
        returnTable.hMetrics[i].advanceWidth = returnTable.hMetrics[returnTable.hMetricsLen - 1].advanceWidth;
        returnTable.hMetrics[i].leftSideBearing = read_int16_t_endian(tempView + i * 4 + 2);
    }
    return returnTable;
}

uint32_t calcTableChecksum(uint32_t *table, uint32_t numberOfBytesInTable) {
    uint32_t sum = 0;
    uint32_t temp = 0;
    uint32_t nLongs = (numberOfBytesInTable + 3) / 4;
    // check for unalignment and add them seperatly with big endian in mind
    if (numberOfBytesInTable % 4 != 0) {
        size_t extra = numberOfBytesInTable % 4;
        nLongs--;
        for (size_t i = 0; i < extra; i++) {
            sum += (uint32_t)(((uint8_t *)table)[numberOfBytesInTable - extra + i]) << (i * 8);
        }
    }
    while (nLongs-- > 0) {
        temp = *table++;
        sum += SWAP_ENDIAN_32(temp);
    }
    return sum;
}

TableDirectory getTableDirectory(W_Parser fontFile, char *tag) {
    TableDirectory table = {0};
    TableDirectory *tables = (TableDirectory *)(&fontFile.fontFile.data[sizeof(OffsetSubTable)]);
    for (size_t i = 0; i < fontFile.numTables; i++) {
        if (memcmp(&tables[i], tag, 4)) {
            continue;
        }
        table.tag = tables[i].tag;
        table.checkSum = SWAP_ENDIAN_32(tables[i].checkSum);
        table.offset = SWAP_ENDIAN_32(tables[i].offset);
        table.length = SWAP_ENDIAN_32(tables[i].length);
        if (table.offset + table.length > fontFile.fontFile.size) {
            fprintf(stderr, "end of table is after the end of sfnt\n");
            return (TableDirectory){0};
        }

        // check the checksum of table, special case is 'head' table where we need to set a value in it to zero(subtracting also works)
        uint32_t calculatedChecksum = calcTableChecksum((uint32_t *)(&fontFile.fontFile.data[table.offset]), table.length);
        if (!memcmp("head", tag, 4)) {
            uint32_t headChecksum = calculatedChecksum - read_uint32_t_endian(&fontFile.fontFile.data[table.offset + 8]);
            if (headChecksum != table.checkSum) {
                fprintf(stderr, "table head invalid checksum 0x%08X when expected 0x%08X\n", headChecksum, table.checkSum);
                return (TableDirectory){0};
            }
        } else if (calculatedChecksum != table.checkSum) {
            fprintf(stderr, "table '%s' invalid checksum 0x%08X when expected 0x%08X\n", tag, calculatedChecksum, table.checkSum);
            return (TableDirectory){0};
        }
        return table;
    }
    fprintf(stderr, "table %s not found\n", tag);
    return (TableDirectory){0};
}

W_Parser checkFont(mappedFile fontFile) {
    uint32_t scaler = read_uint32_t_endian(fontFile.data);
    uint16_t numTables = read_uint32_t_endian(&fontFile.data[4]);
    W_Parser returnStruct = (W_Parser){0};
    if (scaler != 0x74727565 && scaler != 0x00010000) {
        char *incorrectTag = (scaler == 0x74797031) ? "typ1" : (scaler == 0x4F54544F) ? "OTTO"
                                                                                      : "unknown";
        fprintf(stderr, "incompatable font format %s value 0x%08X\n", incorrectTag, scaler);
        return returnStruct;
    }
    if (fontFile.size < sizeof(OffsetSubTable) + (sizeof(TableDirectory) * numTables)) {
        fprintf(stderr, "size of sfnt smaller then size of the font directory\n");
        return returnStruct;
    }

    uint32_t fontChecksum = calcTableChecksum((uint32_t *)fontFile.data, fontFile.size);
    if (fontChecksum != 0xB1B0AFBA) {
        fprintf(stderr, "font checksum incorrect 0x%08X when expecting 0x%08X\n", fontChecksum, 0xB1B0AFBA);
        return returnStruct;
    }
    returnStruct.fontFile = fontFile;
    returnStruct.numTables = numTables;
    return returnStruct;
}

W_Font *parseFont(mappedFile fontFile) {
    // W_Font font = {0};
    W_Parser parser = checkFont(fontFile);
    if (!parser.numTables) return NULL;

    TableDirectory head = getTableDirectory(parser, "head");
    if (!head.tag) return NULL;

    const int screenWidth = 800;
    const int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "WhiskerRenderer");

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("Welcome to Raylib", 190, 200, 20, LIGHTGRAY);
        EndDrawing();
    }

    CloseWindow();
    return NULL;
}