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
size_t contour, click;

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
    if (ABS(a) < 0.0001f) {
        if (b != 0) {
            result.x = -c / b;
        }
    } else {
        float sqrtPortion = (b * b) - (4.0f * a * c);
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

int isInsideGlyf(SimpleGlyfChar *glyf, Point ray, float scale) {
    size_t i, j;
    Point current, outside, next;
    Point intersect0, intersect1, previousIntersect0 = {0}, previousIntersect1 = {0};
    size_t collisionCount = 0, testCounter = 0;
    for (i = 0; i < glyf->contourNum; i++) {
        for (j = 0; j < glyf->contours[i].length; j += 2, testCounter++) {
            current = getAbsoluteXY(glyf, i, j, scale);
            outside = getAbsoluteXY(glyf, i, j + 1, scale);
            next = getAbsoluteXY(glyf, i, j + 2, scale);

            if (current.y > ray.y && outside.y >= ray.y && next.y > ray.y) continue;
            if (current.y < ray.y && outside.y <= ray.y && next.y < ray.y) continue;

            float a = current.y + next.y - (2 * outside.y);
            float b = 2 * (outside.y - current.y);
            float c = current.y;
            Point quadResult = quadraticRoot(a, b, c - ray.y);

            // in case of a max/min? don't include that edge
            if (quadResult.x == 0 && ABS(b) < 1e-5f) continue;
            // in case of a (n*x - n*1)^2 that equals 1
            if (ABS(a - (c - ray.y)) < 1e-4f && ABS(a * 2 - -b) < 1e-4f) continue;

            intersect0 = bezierInterpolation(current, outside, next, quadResult.x);
            intersect1 = bezierInterpolation(current, outside, next, quadResult.y);

            int valid0 =
                (ALMOST_NUMBER(quadResult.x, 0) >= 0 && ALMOST_NUMBER(quadResult.x, 1) < 1) && intersect0.x > ray.x;
            int valid1 =
                (ALMOST_NUMBER(quadResult.y, 0) >= 0 && ALMOST_NUMBER(quadResult.y, 1) < 1) && intersect1.x > ray.x;

            if (valid0) {
                float distance = MIN(deltaD(intersect0, previousIntersect0), deltaD(intersect0, previousIntersect1));
                if (distance > 1e-5f) collisionCount++;
            }
            if (valid1) {
                float distance = MIN(deltaD(intersect1, previousIntersect0), deltaD(intersect1, previousIntersect1));
                if (distance > 1e-5f) collisionCount++;
            }

            if (valid0 && ray.x == mouse.x - 50 && ray.y == mouse.y - 50) {
                DrawCircle(intersect0.x + 50, intersect0.y + 50, 5, PURPLE);
                float distance = MIN(deltaD(intersect0, previousIntersect0), deltaD(intersect0, previousIntersect1));
                if (click && distance > 1e-5f) {
                    printf(
                        "0curve %zd intersection point (%f, %f) with ray (%f, %f) in curve: (%f, %f) (%f, %f) (%f, %f) "
                        "quadResult %.9g a/b/c %f/%f/%f, distance: %.9g\n",
                        testCounter, intersect0.x, intersect0.y, ray.x, ray.y, current.x, current.y, outside.x,
                        outside.y, next.x, next.y, quadResult.x, a, b, c, distance);
                }
            }
            if (valid1 && ray.x == mouse.x - 50 && ray.y == mouse.y - 50) {
                DrawCircle(intersect1.x + 50, intersect1.y + 50, 5, PURPLE);
                float distance = MIN(deltaD(intersect1, previousIntersect0), deltaD(intersect1, previousIntersect1));
                if (click && distance > 1e-5f) {
                    printf(
                        "1curve %zd intersection point (%f, %f) with ray (%f, %f) in curve: (%f, %f) (%f, %f) (%f, %f) "
                        "quadResult %.9g a/b/c %f/%f/%f, distance: %.9g\n",
                        testCounter, intersect1.x, intersect1.y, ray.x, ray.y, current.x, current.y, outside.x,
                        outside.y, next.x, next.y, quadResult.y, a, b, c, distance);
                }
            }
            if (testCounter == contour && ray.x == mouse.x - 50 && ray.y == mouse.y - 50 && click) {
                float distance1 = MIN(deltaD(intersect0, previousIntersect0), deltaD(intersect0, previousIntersect1));
                float distance2 = MIN(deltaD(intersect1, previousIntersect0), deltaD(intersect1, previousIntersect1));
                float distance = MIN(distance1, distance2);
                printf("curve %zd intersection points (%f, %f) (%f, %f) with ray (%f, %f) "
                       "in curve: (%f, %f) (%f, %f) (%f, %f) "
                       "quadResult %.9g/%.9g a/b/c %f/%f/%f valid %d/%d, distance: %.9g\n",
                       testCounter, intersect0.x, intersect0.y, intersect1.x, intersect1.y, ray.x, ray.y, current.x,
                       current.y, outside.x, outside.y, next.x, next.y, quadResult.x, quadResult.y, a, b, c, valid0,
                       valid1, distance);
            }

            if (valid0) previousIntersect0 = intersect0;
            if (valid1) previousIntersect1 = intersect1;
        }
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

    size_t testCounter;
    for (i = 0, testCounter = 0; i < glyf->contourNum; i++) {
        for (j = 0; j < glyf->contours[i].length; j += 2, testCounter++) {
            Point current = getAbsoluteXY(glyf, i, j, scale);
            Point outside = getAbsoluteXY(glyf, i, j + 1, scale);
            Point next = getAbsoluteXY(glyf, i, j + 2, scale);

            DrawCircle(current.x + 50, current.y + 50, 3, RED);
            DrawCircle(outside.x + 50, outside.y + 50, 2, PINK);

            if (testCounter == contour) {
                drawCurve(current, outside, next, 10, GREEN);
            } else {
                drawCurve(current, outside, next, 1, SKYBLUE);
            }
            DrawLine(mouse.x, mouse.y, 2000, mouse.y, GREEN);
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
