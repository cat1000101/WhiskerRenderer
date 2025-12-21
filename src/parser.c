#include <stdio.h>
#include <string.h>

#include "raylib.h"

#include "characterMap.h"
#include "parser.h"
#include "utils.h"

void headFromTD(W_Parser *parser, TableDirectory headTD) {
    Head *table = (Head *)(&parser->fontFile.data[headTD.offset]);
    Head *headView = &parser->tables.head;

    headView->version = SWAP_ENDIAN_32(table->version);
    headView->fontRevision = SWAP_ENDIAN_32(table->fontRevision);
    headView->checkSumAdjustment = SWAP_ENDIAN_32(table->checkSumAdjustment);
    headView->magicNumber = SWAP_ENDIAN_32(table->magicNumber);
    headView->flags = SWAP_ENDIAN_16(table->flags);
    headView->unitsPerEm = SWAP_ENDIAN_16(table->unitsPerEm);
    headView->created = table->created;
    headView->modified = table->modified;
    headView->xMin = (int16_t)SWAP_ENDIAN_16(table->xMin);
    headView->yMin = (int16_t)SWAP_ENDIAN_16(table->yMin);
    headView->xMax = (int16_t)SWAP_ENDIAN_16(table->xMax);
    headView->yMax = (int16_t)SWAP_ENDIAN_16(table->yMax);
    headView->macStyle = SWAP_ENDIAN_16(table->macStyle);
    headView->lowestRecPPEM = SWAP_ENDIAN_16(table->lowestRecPPEM);
    headView->fontDirectionHint = (int16_t)SWAP_ENDIAN_16(table->fontDirectionHint);
    headView->indexToLocFormat = (int16_t)SWAP_ENDIAN_16(table->indexToLocFormat);
    headView->glyphDataFormat = (int16_t)SWAP_ENDIAN_16(table->glyphDataFormat);
}
void maxpFromTD(W_Parser *parser, TableDirectory maxpTD) {
    Maxp *table = (Maxp *)(&parser->fontFile.data[maxpTD.offset]);
    Maxp *maxpView = &parser->tables.maxp;

    maxpView->version = SWAP_ENDIAN_32(table->version);
    maxpView->numGlyphs = SWAP_ENDIAN_16(table->numGlyphs);
    maxpView->maxPoints = SWAP_ENDIAN_16(table->maxPoints);
    maxpView->maxContours = SWAP_ENDIAN_16(table->maxContours);
    maxpView->maxComponentPoints = SWAP_ENDIAN_16(table->maxComponentPoints);
    maxpView->maxComponentContours = SWAP_ENDIAN_16(table->maxComponentContours);
    maxpView->maxZones = SWAP_ENDIAN_16(table->maxZones);
    maxpView->maxTwilightPoints = SWAP_ENDIAN_16(table->maxTwilightPoints);
    maxpView->maxStorage = SWAP_ENDIAN_16(table->maxStorage);
    maxpView->maxFunctionDefs = SWAP_ENDIAN_16(table->maxFunctionDefs);
    maxpView->maxInstructionDefs = SWAP_ENDIAN_16(table->maxInstructionDefs);
    maxpView->maxStackElements = SWAP_ENDIAN_16(table->maxStackElements);
    maxpView->maxSizeOfInstructions = SWAP_ENDIAN_16(table->maxSizeOfInstructions);
    maxpView->maxComponentElements = SWAP_ENDIAN_16(table->maxComponentElements);
    maxpView->maxComponentDepth = SWAP_ENDIAN_16(table->maxComponentDepth);
}
void hheaFromTD(W_Parser *parser, TableDirectory hheaTD) {
    Hhea *table = (Hhea *)(&parser->fontFile.data[hheaTD.offset]);
    Hhea *hheaView = &parser->tables.hhea;

    hheaView->version = SWAP_ENDIAN_32(table->version);
    hheaView->ascent = (int16_t)SWAP_ENDIAN_16(table->ascent);
    hheaView->descent = (int16_t)SWAP_ENDIAN_16(table->descent);
    hheaView->lineGap = (int16_t)SWAP_ENDIAN_16(table->lineGap);
    hheaView->advanceWidthMax = SWAP_ENDIAN_16(table->advanceWidthMax);
    hheaView->minLeftSideBearing = (int16_t)SWAP_ENDIAN_16(table->minLeftSideBearing);
    hheaView->minRightSideBearing = (int16_t)SWAP_ENDIAN_16(table->minRightSideBearing);
    hheaView->xMaxExtent = (int16_t)SWAP_ENDIAN_16(table->xMaxExtent);
    hheaView->caretSlopeRise = (int16_t)SWAP_ENDIAN_16(table->caretSlopeRise);
    hheaView->caretSlopeRun = (int16_t)SWAP_ENDIAN_16(table->caretSlopeRun);
    hheaView->caretOffset = (int16_t)SWAP_ENDIAN_16(table->caretOffset);
    hheaView->reserved1 = (int16_t)SWAP_ENDIAN_16(table->reserved1);
    hheaView->reserved2 = (int16_t)SWAP_ENDIAN_16(table->reserved2);
    hheaView->reserved3 = (int16_t)SWAP_ENDIAN_16(table->reserved3);
    hheaView->reserved4 = (int16_t)SWAP_ENDIAN_16(table->reserved4);
    hheaView->metricDataFormat = (int16_t)SWAP_ENDIAN_16(table->metricDataFormat);
    hheaView->numOfLongHorMetrics = SWAP_ENDIAN_16(table->numOfLongHorMetrics);
}
int locaFromTD(W_Parser *parser, TableDirectory locaTD) {
    uint8_t *tempView = &parser->fontFile.data[locaTD.offset];
    Loca *locaView = &parser->tables.loca;
    size_t i = 0;

    locaView->len = parser->tables.maxp.numGlyphs;
    if (parser->tables.head.indexToLocFormat == 0) {
        locaView->offsets.shortOffsets = SAFE_MALLOC(sizeof(*locaView->offsets.shortOffsets) * locaView->len);
        for (i = 0; i <= locaView->len; i++) {
            locaView->offsets.shortOffsets[i] =
                read_uint16_t_endian(tempView + i * sizeof(*locaView->offsets.shortOffsets));
        }
    } else {
        locaView->offsets.longOffsets = SAFE_MALLOC(sizeof(*locaView->offsets.longOffsets) * locaView->len);
        for (i = 0; i <= locaView->len; i++) {
            locaView->offsets.longOffsets[i] =
                read_uint32_t_endian(tempView + i * sizeof(*locaView->offsets.longOffsets));
        }
    }
    return 0;
}
int hmtxFromTD(W_Parser *parser, TableDirectory hmtxTD) {
    uint8_t *tempView = &parser->fontFile.data[hmtxTD.offset];
    Hmtx *htmxView = &parser->tables.hmtx;
    size_t i = 0;

    htmxView->hMetricsLen = parser->tables.hhea.numOfLongHorMetrics;
    htmxView->leftSideBearingLen = parser->tables.maxp.numGlyphs - parser->tables.hhea.numOfLongHorMetrics;
    htmxView->hMetrics = SAFE_MALLOC(htmxView->hMetricsLen * sizeof(*htmxView->hMetrics));
    for (i = 0; i < htmxView->hMetricsLen; i++) {
        htmxView->hMetrics[i].advanceWidth = read_uint16_t_endian(tempView + i * 4);
        htmxView->hMetrics[i].leftSideBearing = read_int16_t_endian(tempView + i * 4 + 2);
    }
    for (i = htmxView->hMetricsLen; i < parser->tables.maxp.numGlyphs; i++) {
        htmxView->hMetrics[i].advanceWidth = htmxView->hMetrics[htmxView->hMetricsLen - 1].advanceWidth;
        htmxView->hMetrics[i].leftSideBearing = read_int16_t_endian(tempView + i * 4 + 2);
    }
    return 0;
}

uint32_t calcTableChecksum(uint32_t *table, uint32_t numberOfBytesInTable) {
    uint32_t nLongs = (numberOfBytesInTable + 3) / 4;
    uint32_t sum = 0;
    uint32_t temp = 0;
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

int getTableDirectoryAt(W_Parser *parser, size_t offset, TableDirectory *result) {
    uint8_t *rawTable = &parser->fontFile.data[sizeof(OffsetSubTable) + offset];

    result->tag = *(uint32_t *)(&rawTable[0]);
    result->checkSum = read_uint32_t_endian(&rawTable[OFFSET_OF(TableDirectory, checkSum)]);
    result->offset = read_uint32_t_endian(&rawTable[OFFSET_OF(TableDirectory, offset)]);
    result->length = read_uint32_t_endian(&rawTable[OFFSET_OF(TableDirectory, length)]);
    if (result->offset + result->length > parser->fontFile.size) {
        fprintf(stderr, "end of table '%.4s' is after the end of sfnt\n", (char *)(&result->tag));
        return 1;
    }

    // check the checksum of table, special case is 'head' table where we need to set a value in it to zero(subtracting
    // also works)
    uint32_t calculatedChecksum =
        calcTableChecksum((uint32_t *)(&parser->fontFile.data[result->offset]), result->length);
    if (!memcmp("head", (char *)(&result->tag), 4)) {
        uint32_t headChecksum = calculatedChecksum - read_uint32_t_endian(&parser->fontFile.data[result->offset + 8]);
        if (headChecksum != result->checkSum) {
            fprintf(stderr, "table head invalid checksum 0x%08X when expected 0x%08X\n", headChecksum,
                    result->checkSum);
            return 1;
        }
    } else if (calculatedChecksum != result->checkSum) {
        fprintf(stderr, "table '%.4s' invalid checksum 0x%08X when expected 0x%08X\n", (char *)(&result->tag),
                calculatedChecksum, result->checkSum);
        return 1;
    }
    return 0;
}

int getTableDirectory(W_Parser *parser, char *tag, TableDirectory *result) {
    size_t offset = 0;
    TableDirectory *tables = (TableDirectory *)(&parser->fontFile.data[sizeof(OffsetSubTable)]);

    for (size_t i = 0; i < parser->numTables; i++) {
        if (memcmp(&tables[i], tag, 4)) {
            continue;
        }
        offset = (size_t)(&tables[i]) - (size_t)(tables);
        if (getTableDirectoryAt(parser, offset, result)) return 1;
        if (!result->tag) {
            fprintf(stderr, "invalid searched table directory '%.4s' at offset %08zX\n", tag, offset);
            return 1;
        }
        return 0;
    }
    fprintf(stderr, "table %s not found\n", tag);
    return 1;
}

int setTables(W_Parser *parser) {
    TableDirectory table = {0};

    if (getTableDirectory(parser, "head", &table)) ERROR_OUT("unable to get required table head");
    headFromTD(parser, table);

    if (getTableDirectory(parser, "maxp", &table)) ERROR_OUT("unable to get required table maxp");
    maxpFromTD(parser, table);

    if (getTableDirectory(parser, "hhea", &table)) ERROR_OUT("unable to get required table hhea");
    hheaFromTD(parser, table);

    if (getTableDirectory(parser, "loca", &table)) ERROR_OUT("unable to get required table loca");
    if (locaFromTD(parser, table)) ERROR_OUT("failed to parse loca");

    if (getTableDirectory(parser, "hmtx", &table)) ERROR_OUT("unable to get required table hmtx");
    if (hmtxFromTD(parser, table)) ERROR_OUT("failed to parse hmtx");

    if (getTableDirectory(parser, "cmap", &table)) ERROR_OUT("unable to get required table cmap");
    if (cmapFromTD(parser, table)) ERROR_OUT("failed to parse hmtx");

    return 0;
}

int checkFont(MappedFile fontFile, W_Parser *result) {
    uint32_t scaler = read_uint32_t_endian(fontFile.data);
    uint16_t numTables = read_uint32_t_endian(&fontFile.data[4]);

    if (scaler != 0x74727565 && scaler != 0x00010000) {
        char *incorrectTag = (scaler == 0x74797031) ? "typ1" : (scaler == 0x4F54544F) ? "OTTO" : "unknown";
        fprintf(stderr, "incompatable font format %s value 0x%08X\n", incorrectTag, scaler);
        return 1;
    }
    if (fontFile.size < sizeof(OffsetSubTable) + (sizeof(TableDirectory) * numTables)) {
        fprintf(stderr, "size of sfnt smaller then size of the font directory\n");
        return 1;
    }

    uint32_t fontChecksum = calcTableChecksum((uint32_t *)fontFile.data, fontFile.size);
    if (fontChecksum != 0xB1B0AFBA) {
        fprintf(stderr, "font checksum incorrect 0x%08X when expecting 0x%08X\n", fontChecksum, 0xB1B0AFBA);
        return 1;
    }
    result->fontFile = fontFile;
    result->numTables = numTables;
    return 0;
}

W_Font *parseFont(MappedFile fontFile) {
    W_Parser parser = (W_Parser){0};
    if (checkFont(fontFile, &parser)) return NULL;
    if (setTables(&parser)) return NULL;

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
