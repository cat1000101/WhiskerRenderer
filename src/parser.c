#include <stdio.h>
#include <string.h>

#include "raylib.h"

#include "parser.h"
#include "utils.h"

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
        sum += big32ToLittle32(temp);
    }
    return sum;
}

TableDirectory getTableDirectory(W_Parser fontFile, char *tag) {
    TableDirectory table = {0};
    TableDirectory *tables = (TableDirectory *)(&fontFile.fontFile.data[sizeof(OffsetSubTable)]);
    for (size_t i = 0; i < fontFile.numTables; i++) {
        if (!memcmp(&tables[i], tag, 4)) {
            table.tag = tables[i].tag;
            table.checkSum = big32ToLittle32(tables[i].checkSum);
            table.offset = big32ToLittle32(tables[i].offset);
            table.length = big32ToLittle32(tables[i].length);
            if (table.offset + table.length > fontFile.fontFile.size) {
                fprintf(stderr, "end of table is after the end of sfnt\n");
                return (TableDirectory){0};
            }

            // check the checksum of table, special case is 'head' table where we need to set a value in it to zero(subtracting also works)
            uint32_t calculatedChecksum = calcTableChecksum((uint32_t *)(&fontFile.fontFile.data[table.offset]), table.length);
            if (!memcmp("head", tag, 4)) {
                uint32_t headChecksum = calculatedChecksum - big32ToLittle32(*(uint32_t *)(&fontFile.fontFile.data[table.offset + 8]));
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
    }
    fprintf(stderr, "table %s not found\n", tag);
    return (TableDirectory){0};
}

W_Parser checkFont(mappedFile fontFile) {
    uint32_t scaler = big32ToLittle32(*(uint32_t *)fontFile.data);
    uint16_t numTables = big16ToLittle16(*(uint16_t *)(&fontFile.data[4]));
    if (scaler != 0x74727565 && scaler != 0x00010000) {
        char *incorrectTag = (scaler == 0x74797031) ? "typ1" : (scaler == 0x4F54544F) ? "OTTO"
                                                                                      : "unknown";
        fprintf(stderr, "incompatable font format %s value 0x%08X\n", incorrectTag, scaler);
        return (W_Parser){0};
    }
    if (fontFile.size < sizeof(OffsetSubTable) + (sizeof(TableDirectory) * numTables)) {
        fprintf(stderr, "size of sfnt smaller then size of the font directory\n");
        return (W_Parser){0};
    }

    uint32_t fontChecksum = calcTableChecksum((uint32_t *)fontFile.data, fontFile.size);
    if (fontChecksum != 0xB1B0AFBA) {
        fprintf(stderr, "font checksum incorrect 0x%08X when expecting 0x%08X\n", fontChecksum, 0xB1B0AFBA);
        return (W_Parser){0};
    }
    return (W_Parser){fontFile, numTables};
}

W_Font *parseFont(mappedFile fontFile) {
    // W_Font font = {0};
    W_Parser parser = checkFont(fontFile);
    if (!parser.numTables) {
        return NULL;
    }
    TableDirectory head = getTableDirectory(parser, "head");
    if (!head.tag) {
        return NULL;
    }

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