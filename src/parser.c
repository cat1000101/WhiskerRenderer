#include "parser.h"
#include "raylib.h"

uint32_t calcTableChecksum(uint32_t *table, uint32_t numberOfBytesInTable) {
    uint32_t sum = 0;
    uint32_t nLongs = (numberOfBytesInTable + 3) / 4;
    while (nLongs-- > 0)
        sum += *table++;
    return sum;
}

W_Font *parseFont(FILE *fontFile) {
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