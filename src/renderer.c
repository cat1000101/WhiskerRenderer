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

Vector2 mouse;
size_t contour, contourNum, click;

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
            if (result.x == result.y) result.y = NAN;
        }
    }
    return result;
}
float deltaD(Point p0, Point p1) {
    float dx = SQR(p0.x - p1.x);
    float dy = SQR(p0.y - p1.y);
    return dx + dy;
}
// intersection point (186.253220, 176.000015) with ray (14.000000, 176.000000) in curve: (88.800003, 184.800003) (131.199997, 176.000000) (186.400009, 176.000000)
// intersection point (231.199997, 176.000000) with ray (14.000000, 176.000000) in curve: (231.199997, 176.000000) (231.199997, 164.800003) (231.199997, 153.600006)
// intersection point (259.199982, 175.999985) with ray (14.000000, 176.000000) in curve: (259.200012, 153.600006) (259.200012, 260.800018) (259.200012, 368.000000)
// curve 4 (88.800003, 184.800003) (131.199997, 176.000000) (186.400009, 176.000000) quadResult 0.998670 a/b/c 8.799988/-17.600006/184.800003
// curve 6 (231.199997, 176.000000) (231.199997, 164.800003) (231.199997, 153.600006) quadResult 0.000000 a/b/c 0.000000/-22.399994/176.000000
// curve 21 (259.200012, 153.600006) (259.200012, 260.800018) (259.200012, 368.000000) valid a/b/c -0.000061/214.400024/153.

// curve 13 (42.400002, 49.600002) (36.000000, 49.600002) (32.000000, 44.000000)
// curve 14 (32.000000, 44.000000) (28.000000, 37.600002) (28.000000, 33.600002)
// y = 44   \* a -1

// curve 9 (0.000000, 144.000000) (0.000000, 104.000000) (19.200001, 72.000000)
// curve 10 (19.200001, 72.000000) (39.200001, 39.200001) (72.000000, 20.000000)
// y = 72   /* q -1

// curve 11 (72.000000, 20.000000) (104.800003, 0.000000) (144.000000, 0.000000)
// curve 12 (144.000000, 0.000000) (184.000000, 0.000000) (211.199997, 19.200001) *
// y = 0 _* +1
// t = 0.5 t' = -,+ t = 0 t' = 0 quadResult = 0: continue
// 2at + b

int isInsideGlyf(SimpleGlyfChar *glyf, Point ray, float scale) {
    size_t i, j, contourCounter = 0;
    Point current, outside, next, intersect0, intersect1, previousIntersect0 = {0}, previousIntersect1 = {0};
    size_t collisionCount = 0;
    for (i = 0; i < glyf->contourNum; i++) {
        for (j = 0; j < glyf->contours[i].length; j += 2, contourCounter++) {
            current = getAbsoluteXY(glyf, i, j, scale);
            outside = getAbsoluteXY(glyf, i, j + 1, scale);
            next = getAbsoluteXY(glyf, i, j + 2, scale);

            if (current.y > ray.y && outside.y >= ray.y && next.y > ray.y) continue;
            if (current.y < ray.y && outside.y <= ray.y && next.y < ray.y) continue;

            float a = current.y + next.y - (2 * outside.y);
            float b = 2 * (outside.y - current.y);
            float c = current.y;
            Point quadResult = quadraticRoot(a, b, c - ray.y);

            intersect0 = bezierInterpolation(current, outside, next, quadResult.x);
            intersect1 = bezierInterpolation(current, outside, next, quadResult.y);

            int valid0 = (quadResult.x >= 0 && quadResult.x < 1) && intersect0.x > ray.x;
            int valid1 = (quadResult.y >= 0 && quadResult.y < 1) && intersect1.x > ray.x;

            if (valid0) {
                float distance = MIN(deltaD(intersect0, previousIntersect0), deltaD(intersect0, previousIntersect1));
                if (click && ray.x == mouse.x - 50 && ray.y == mouse.y - 50) {
                    printf("0distance: %f\n", distance);
                }
                if (distance > 1e-5) collisionCount++;
            }
            if (valid1) {
                float distance = MIN(deltaD(intersect1, previousIntersect0), deltaD(intersect1, previousIntersect1));
                if (click && ray.x == mouse.x - 50 && ray.y == mouse.y - 50) {
                    printf("1distance: %f\n", distance);
                }
                if (distance > 1e-5) collisionCount++;
            }

            if (valid0 && ray.x == mouse.x - 50 && ray.y == mouse.y - 50) {
                DrawCircle(intersect0.x + 50, intersect0.y + 50, 5, PURPLE);
                if (click && MIN(deltaD(intersect0, previousIntersect0), deltaD(intersect0, previousIntersect1)) > 1e-5) {
                    printf(
                        "0curve %zd intersection point (%f, %f) with ray (%f, %f) in curve: (%f, %f) (%f, %f) (%f, %f) "
                        "quadResult %f a/b/c %f/%f/%f\n",
                        contourCounter, intersect0.x, intersect0.y, ray.x, ray.y, current.x, current.y, outside.x,
                        outside.y, next.x, next.y, quadResult.x, a, b, c);
                }
            }
            if (valid1 && ray.x == mouse.x - 50 && ray.y == mouse.y - 50) {
                DrawCircle(intersect1.x + 50, intersect1.y + 50, 5, PURPLE);
                if (click && MIN(deltaD(intersect1, previousIntersect0), deltaD(intersect1, previousIntersect1)) > 1e-5) {
                    printf(
                        "1curve %zd intersection point (%f, %f) with ray (%f, %f) in curve: (%f, %f) (%f, %f) (%f, %f) "
                        "quadResult %f a/b/c %f/%f/%f\n",
                        contourCounter, intersect1.x, intersect1.y, ray.x, ray.y, current.x, current.y, outside.x,
                        outside.y, next.x, next.y, quadResult.y, a, b, c);
                }
            }

            if (valid0) previousIntersect0 = intersect0;
            if (valid1) previousIntersect1 = intersect1;
        }
    }
    if (ray.x == mouse.x - 50 && ray.y == mouse.y - 50 && click) {
        printf("collision amount: %zd\n", collisionCount);
    }
    // when the amount of intersection is odd the point is inside the glyf
    return (collisionCount % 2) == 1;
}

void drawCurve(Point p0, Point p1, Point p2, float thickness, Color color) {
    Point previous = p0;
    Point next = {0};
    float t = 0;
    for (size_t i = 0; i < RESOLUTION; i++) {
        t = (i + 1.0f) / RESOLUTION;
        next = bezierInterpolation(p0, p1, p2, t);
        // DrawLine(previous.x + 50, previous.y + 50, next.x + 50, next.y + 50, color);
        DrawLineEx((Vector2){previous.x + 50, previous.y + 50}, (Vector2){next.x + 50, next.y + 50}, thickness, color);
        previous = next;
    }
    if (click && thickness == 10) {
        printf("curve %zd (%f, %f) (%f, %f) (%f, %f)\n", contour, p0.x, p0.y, p1.x, p1.y, p2.x, p2.y);
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

            if (contourNum == contour) {
                drawCurve(current, outside, next, 10, GREEN);
            } else {
                drawCurve(current, outside, next, 1, SKYBLUE);
            }
            DrawLine(mouse.x, mouse.y, 2000, mouse.y, GREEN);
            contourNum++;
        }
    }
    contourNum = 0;
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
        if (IsKeyPressed(KEY_N)) {
            contour++;
        } else {
            tmp = GetCharPressed();
            key = (tmp && tmp != 'n') ? tmp : key;
        }
        mouse = GetMousePosition();
        click = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

        BeginDrawing();
        ClearBackground(RAYWHITE);
        // drawBitmap(bitmaps[(uint8_t)key], 300, 2);
        drawChar(font, (uint8_t)key, 800);
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
