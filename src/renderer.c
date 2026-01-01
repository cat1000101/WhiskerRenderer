#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "renderer.h"
#include "utils.h"
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
    result.y = (glyf->boundingBox.yMax - glyf->contours[contourNum].yFontUnit[index]) * scale;
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
// t = (-b +- sqrt(b^2 - 4ac)) / 2a
Point quadraticRoot(float a, float b, float c) {
    Point result = {NAN, NAN};
    if (ABS(a) < 0.0001) {
        if (b != 0) {
            result.x = -c / b;
        }
    } else {
        float sqrtPortion = (b * b) - (4 * a * c);
        if (sqrtPortion >= 0) {
            result.x = (-b + sqrt(sqrtPortion)) / (2 * a);
            result.y = (-b - sqrt(sqrtPortion)) / (2 * a);
        }
    }
    return result;
}

int isIntersecting(Point p0, Point p1, Point p2, Point ray) {
    float a = p0.y + p2.y - (2 * p1.y);
    float b = 2 * (p1.y - p0.y);
    float c = p0.y;
    Point quadResult = quadraticRoot(a, b, c - ray.y);
    int collisionCount = 0;
    int valid1 = (quadResult.x >= 0 && quadResult.x < 1) && bezierInterpolation(p0, p1, p2, quadResult.x).x > ray.x;
    int valid2 = (quadResult.y >= 0 && quadResult.y < 1) && bezierInterpolation(p0, p1, p2, quadResult.y).x > ray.x;

    if (valid1) collisionCount++;
    if (valid2) collisionCount++;

    return collisionCount;
}
int isInsideGlyf(SimpleGlyfChar *glyf, Point ray, float scale) {
    size_t i, j;
    Point current, outside, next;
    size_t collisionCount = 0;
    for (i = 0; i < glyf->contourNum; i++) {
        for (j = 0; j < glyf->contours[i].length; j += 2) {
            current = getAbsoluteXY(glyf, i, j, scale);
            outside = getAbsoluteXY(glyf, i, j + 1, scale);
            next = getAbsoluteXY(glyf, i, j + 2, scale);

            if (current.y > ray.y && outside.y >= ray.y && next.y > ray.y) continue;
            if (current.y < ray.y && outside.y <= ray.y && next.y < ray.y) continue;

            collisionCount += isIntersecting(current, outside, next, ray);
        }
    }
    // when the amount of intersection is odd the point is inside the glyf
    return (collisionCount % 2) == 1;
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

charBitmap rasterizeCharBitmap(W_Font *font, uint8_t c, size_t px) {
    SimpleGlyfChar *glyf = &font->parser.tables.glyf.chars[c];
    float scale = (float)px / (float)font->parser.tables.head.unitsPerEm;
    uint16_t width_f32 = (glyf->boundingBox.xMax - glyf->boundingBox.xMin) * scale;
    size_t width = (uint16_t)(width_f32 + 0.5f);
    uint16_t hight_f32 = (glyf->boundingBox.yMax - glyf->boundingBox.yMin) * scale;
    size_t hight = (uint16_t)(hight_f32 + 0.5f);
    // printf("rendering '%c': scale %f width/hight %f/%f min(%f, %f) max(%f, %f)\n", c, scale, width_f32, hight_f32,
    // (float)glyf->boundingBox.xMin, (float)glyf->boundingBox.yMin, (float)glyf->boundingBox.xMax,
    // (float)glyf->boundingBox.yMax);
    size_t i, j;
    uint8_t *bitmap = SAFE_MALLOC(width * hight);
    memset(bitmap, 0, width * hight);

    for (i = 0; i < hight; i++) {
        for (j = 0; j < width; j++) {
            if (isInsideGlyf(glyf, (Point){(float)j, (float)i}, scale)) {
                bitmap[i * width + j] = 0xFF;
            }
        }
    }

    return (charBitmap){.bitmap = bitmap, .hight = hight, .width = width};
}

void drawBitmap(charBitmap bitmap, size_t px, size_t scale) {
    size_t i, j, dx, dy;
    for (i = 0; i < bitmap.hight; i++) {
        for (j = 0; j < bitmap.width; j++) {
            Color color;
            switch (bitmap.bitmap[i * bitmap.width + j]) {
            case 0xFF:
                color = BLACK;
                break;
            case 0x80:
                color = RED;
                break;
            case 0x40:
                color = PINK;
                break;
            default:
                color = RAYWHITE;
            }
            dx = j * scale + 50;
            dy = i * scale + 50;
            DrawRectangle(dx, dy, scale, scale, color);
        }
    }
}

void drawChar(W_Font *font, uint8_t c, size_t px) {
    SimpleGlyfChar *glyf = &font->parser.tables.glyf.chars[c];
    float scale = (float)px / (float)font->parser.tables.head.unitsPerEm;
    uint16_t width_f32 = (glyf->boundingBox.xMax - glyf->boundingBox.xMin) * scale;
    size_t width = (uint16_t)(width_f32 + 0.5f);
    uint16_t hight_f32 = (glyf->boundingBox.yMax - glyf->boundingBox.yMin) * scale;
    size_t hight = (uint16_t)(hight_f32 + 0.5f);
    size_t i, j;

    for (i = 0; i < hight; i++) {
        for (j = 0; j < width; j++) {
            if (isInsideGlyf(glyf, (Point){(float)j, (float)i}, scale)) {
                DrawPixel(j + 50, i + 50, BLACK);
            }
        }
    }

    for (i = 0; i < glyf->contourNum; i++) {
        for (j = 0; j < glyf->contours[i].length; j += 2) {
            Point current = getAbsoluteXY(glyf, i, j, scale);
            Point outside = getAbsoluteXY(glyf, i, j + 1, scale);
            Point next = getAbsoluteXY(glyf, i, j + 2, scale);

            DrawCircle(current.x + 50, current.y + 50, 3, RED);
            DrawCircle(outside.x + 50, outside.y + 50, 2, PINK);

            drawCurve(current, outside, next);
        }
    }
}

int drawString_i(W_Font *font, char *character) {
    // charBitmap *bitmaps = SAFE_MALLOC(sizeof(charBitmap) * 256);
    // for (size_t i = 0; i < 256; i++) {
    // if (!getPrintChar(i)) continue;
    // bitmaps[i] = rasterizeCharBitmap(font, i, 300);
    // }

    char tmp, key = 'a';

    int width = 1200;
    int hight = 800;
    InitWindow(width, hight, "testing fonts");
    while (!WindowShouldClose()) {
        tmp = GetCharPressed();
        key = tmp ? tmp : key;

        BeginDrawing();
        ClearBackground(RAYWHITE);
        // drawBitmap(bitmaps[(uint8_t)key], 300, 2);
        drawChar(font, (uint8_t)key, 800);
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
