#include <stddef.h>
#include <stdio.h>

#include "renderer.h"
#include "whiskerRenderer.h"
#include "whiskerRendererTypes.h"

#include "raylib.h"

typedef struct {
    float x;
    float y;
} Point;
Point getAbsoluteXY(SimpleGlyfChar *glyf, size_t contourNum, size_t index, float scale) {
    Point result = {0};
    index = index % glyf->contours[contourNum].length;
    result.x = (glyf->contours[contourNum].xFontUnit[index] - glyf->boundingBox.xMin) * scale;
    result.y =
        (glyf->boundingBox.yMax - (glyf->contours[contourNum].yFontUnit[index] - glyf->boundingBox.yMin)) * scale;
    // printf("contour/index: %zd/%zd orig: (%d, %d) transformed: (%f, %f) onContour: %d\n", contourNum, index,
    // glyf->contours[contourNum].xFontUnit[index], glyf->contours[contourNum].yFontUnit[index], result.x, result.y,
    // glyf->contours[contourNum].flags[index] & 1);
    return result;
}

// p(t) = (1-t)^2 * p0 + 2t(1-t)p1 + t^2 * p2
Point bezierInterpolation(Point p0, Point p1, Point p2, float t) {
    Point result = {0};
    result.x = (p0.x * (1 - t) * (1 - t)) + (2 * t * p1.x * (1 - t)) + (t * t * p2.x);
    result.y = (p0.y * (1 - t) * (1 - t)) + (2 * t * p1.y * (1 - t)) + (t * t * p2.y);
    return result;
}
void drawCurve(Point p0, Point p1, Point p2) {
    Point previous = p0;
    Point next = {0};
    float t = 0;
    for (size_t i = 0; i < RESOLUTION; i++) {
        t = (i + 1.0f) / RESOLUTION;
        next = bezierInterpolation(p0, p1, p2, t);
        DrawLine(previous.x + 50, previous.y + 50, next.x + 50, next.y + 50, SKYBLUE);
        previous = next;
    }
}

int renderCharBitmap(W_Font *font, uint8_t c, size_t px) {
    SimpleGlyfChar glyf = font->parser.tables.glyf.chars[c];
    // float scale = (float)px / (float)font->parser.tables.head.unitsPerEm;
    float scale = 500.0f / (glyf.boundingBox.yMax - glyf.boundingBox.yMin);
    // float scale = 1;
    float width = (glyf.boundingBox.xMax - glyf.boundingBox.xMin) * scale;
    float hight = (glyf.boundingBox.yMax - glyf.boundingBox.yMin) * scale;
    printf("rendering '%c': scale %f width/hight %f/%f min(%f, %f) max(%f, %f)\n", c, scale, width, hight,
           (float)glyf.boundingBox.xMin, (float)glyf.boundingBox.yMin, (float)glyf.boundingBox.xMax,
           (float)glyf.boundingBox.yMax);

    for (size_t i = 0; i < glyf.contourNum; i++) {
        for (size_t j = 0; j < glyf.contours[i].length; j += 2) {
            Point current = getAbsoluteXY(&glyf, i, j, scale);
            Point outside = getAbsoluteXY(&glyf, i, j + 1, scale);
            Point next = getAbsoluteXY(&glyf, i, j + 2, scale);

            DrawCircle(current.x + 50, current.y + 50, 3, RED);
            DrawCircle(outside.x + 50, outside.y + 50, 2, PINK);

            drawCurve(current, outside, next);
        }
    }

    return 0;
}

int drawString_i(W_Font *font, char *character) {
    int width = 900;
    int hight = 600;
    InitWindow(width, hight, "testing fonts");
    // while (!WindowShouldClose()) {}
    BeginDrawing();
    ClearBackground(RAYWHITE);
    renderCharBitmap(font, 'B', 42);
    EndDrawing();

    while (1) {
    }
    CloseWindow();

    return 0;
}
