#include <float.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "utils.h"
#include "whiskerRenderer.h"
#include "whiskerRendererTypes.h"

#include "raylib.h"

#define DEBUG_THINGY

#ifdef DEBUG_THINGY
Vector2 mouse;
size_t contour, click;
#endif

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
            result.x = (-b + sqrtf(sqrtPortion)) / (2 * a);
            result.y = (-b - sqrtf(sqrtPortion)) / (2 * a);
            if (result.x == result.y) result.y = NAN;
        }
    }
    return result;
}

int isInsideGlyf(SimpleGlyfChar *glyf, Point ray, float scale) {
    size_t i, j, testCounter = 0;
    Point current, outside, next;
    float closestDst = FLT_MAX;
    int insideGlyf = 0;
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

            Point intersect0 = bezierInterpolation(current, outside, next, quadResult.x);
            Point intersect1 = bezierInterpolation(current, outside, next, quadResult.y);

            int valid0 = ALMOST_NUMBER(quadResult.x, 0) >= 0 && quadResult.x <= 1 && intersect0.x > ray.x;
            int valid1 = ALMOST_NUMBER(quadResult.y, 0) >= 0 && quadResult.y <= 1 && intersect1.x > ray.x;

            if (valid0 || valid1) {
                int use0 = (valid0 && valid1) ? intersect0.x < intersect1.x : valid0;
                float distance = (use0 ? intersect0.x : intersect1.x) - ray.x;

                if (distance < closestDst) {
                    float rootUsed = use0 ? quadResult.x : quadResult.y;
                    float angle = 2 * a * rootUsed + b;
                    insideGlyf = angle > 0;
                    closestDst = distance;
                }
            }

#ifdef DEBUG_THINGY
            if ((valid0 || valid1) && ray.x == mouse.x - 50 && ray.y == mouse.y - 50) {
                int use0 = (valid0 && valid1) ? intersect0.x < intersect1.x : valid0;
                Point intersectSelected = use0 ? intersect0 : intersect1;
                DrawCircle(intersectSelected.x + 50, intersectSelected.y + 50, 5, PURPLE);
                if (click) {
                    float distance = intersectSelected.x - ray.x;
                    float rootUsed = use0 ? quadResult.x : quadResult.y;
                    float angle = 2 * a * rootUsed + b;
                    printf("curve %zd intersection points (%f, %f) with ray (%f, %f)\n"
                           "in curve: (%f, %f) (%f, %f) (%f, %f) angle %.9g\n"
                           "quadResult %.9g/%.9g a/b/c %f/%f/%f valid %d/%d, distance: %.9g\n\n",
                           testCounter, intersectSelected.x, intersectSelected.y, ray.x, ray.y, current.x, current.y,
                           outside.x, outside.y, next.x, next.y, angle, quadResult.x, quadResult.y, a, b, c, valid0,
                           valid1, distance);
                }
            }
            if (testCounter == contour && ray.x == mouse.x - 50 && ray.y == mouse.y - 50 && click) {
                int use0 = intersect0.x < intersect1.x;
                Point intersectSelected = use0 ? intersect0 : intersect1;
                float distance = intersectSelected.x - ray.x;
                float rootUsed = use0 ? quadResult.x : quadResult.y;
                float angle = 2 * a * rootUsed + b;
                printf("meow curve %zd intersection points (%f, %f) with ray (%f, %f)\n"
                       "in curve: (%f, %f) (%f, %f) (%f, %f) angle %.9g\n"
                       "quadResult %.9g/%.9g a/b/c %f/%f/%f valid %d/%d, distance: %.9g\n\n",
                       testCounter, intersectSelected.x, intersectSelected.y, ray.x, ray.y, current.x, current.y,
                       outside.x, outside.y, next.x, next.y, angle, quadResult.x, quadResult.y, a, b, c, valid0, valid1,
                       distance);
            }
#endif
        }
    }
    return insideGlyf;
}

void rasterizeBezierCurve(W_Font *font, Point p0, Point p1, Point p2, uint8_t *bitmap, size_t width, size_t height) {
    float length = sqrtf(SQR(p0.x - p1.x) + SQR(p0.y - p1.y)) + sqrtf(SQR(p2.x - p1.x) + SQR(p2.y - p1.y));
    for (size_t i = 0; i < length; i++) {
        float t = i / length;
        Point p = bezierInterpolation(p0, p1, p2, t);
        p.x = p.x + 0.5f;
        if (p.x < 0 || p.x >= width || p.y < 0 || p.y >= height) {
            // printf("out of bound (%f, %f) box (%zd, %zd)\n", p.x, p.y, width, height);
            continue;
        }
        bitmap[(size_t)(p.x) + (size_t)(p.y) * width] = 0xFF;
    }
}

charBitmap rasterizeCharBitmap(W_Font *font, uint8_t c, size_t px) {
    SimpleGlyfChar *glyf = &font->parser->tables.glyf.chars[c];
    float scale = (float)px / (float)font->parser->tables.head.unitsPerEm;
    float width_f32 = (glyf->boundingBox.xMax - glyf->boundingBox.xMin) * scale;
    size_t width = (uint16_t)(width_f32 + 0.5f) + 1;
    float height_f32 = (glyf->boundingBox.yMax - glyf->boundingBox.yMin) * scale;
    size_t height = (uint16_t)(height_f32 + 0.5f) + 1;
    // printf("rendering '%c': scale %f width/height %f/%f min(%f, %f) max(%f, %f)\n", c, scale, width_f32, height_f32,
    // (float)glyf->boundingBox.xMin, (float)glyf->boundingBox.yMin, (float)glyf->boundingBox.xMax,
    // (float)glyf->boundingBox.yMax);
    size_t i, j;
    uint8_t *bitmap = SAFE_MALLOC(width * height);
    memset(bitmap, 0, width * height);

    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            if (isInsideGlyf(glyf, (Point){(float)j, (float)i}, scale)) {
                bitmap[i * width + j] = 0xFF;
            }
        }
    }

    // printf("size %zd %zd advance %f lsb %f\n", width, height, glyf->hMetrics.advanceWidth * scale,
    // glyf->hMetrics.leftSideBearing * scale);

    // i am not sure if this is good or not made it draw to the ceil so better
    for (i = 0; i < glyf->contourNum; i++) {
        for (j = 0; j < glyf->contours[i].length; j += 2) {
            Point current = getAbsoluteXY(glyf, i, j, scale);
            Point outside = getAbsoluteXY(glyf, i, j + 1, scale);
            Point next = getAbsoluteXY(glyf, i, j + 2, scale);
            rasterizeBezierCurve(font, current, outside, next, bitmap, width, height);
        }
    }

    return (charBitmap){.bitmap = bitmap,
                        .height = height,
                        .width = width,
                        .advanceWidth = glyf->hMetrics.advanceWidth * scale,
                        .leftSideBearing = glyf->hMetrics.leftSideBearing * scale,
                        .verticalAdjustment = glyf->boundingBox.yMin * scale};
}

void drawBitmap(charBitmap bitmap, size_t scale, Point offset) {
    size_t i, j, dx, dy;
    for (i = 0; i < bitmap.height; i++) {
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
            dx = j * scale + offset.x;
            dy = i * scale + offset.y - bitmap.height - bitmap.verticalAdjustment;
            if (scale == 1) {
                DrawPixel(dx, dy, color);
            } else {
                DrawRectangle(dx, dy, scale, scale, color);
            }
        }
    }
}

charBitmap getBitmapForChar(void *font, char c, size_t px) {
    charBitmap result = {0};
    result = rasterizeCharBitmap(font, c, px);
    return result;
}
void raylibDrawString(void *font, char *s, size_t px, int posX, int posY) {
    W_Font *fontT = (W_Font *)font;
    Point pen = {posX, posY};
    char c = '\0';
    while ((c = *s++)) {
        charBitmap bitmap = rasterizeCharBitmap(fontT, c, px);
        pen.x += bitmap.leftSideBearing;
        drawBitmap(bitmap, 1, pen);
        pen.x += bitmap.advanceWidth;
    }
}

#ifdef DEBUG_THINGY
#define RESOLUTION 20
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

void drawChar(W_Font *font, uint8_t c, size_t px) {
    SimpleGlyfChar *glyf = &font->parser->tables.glyf.chars[c];
    float scale = (float)px / (float)font->parser->tables.head.unitsPerEm;
    float width_f32 = (glyf->boundingBox.xMax - glyf->boundingBox.xMin) * scale;
    size_t width = (uint16_t)(width_f32 + 0.5f);
    float height_f32 = (glyf->boundingBox.yMax - glyf->boundingBox.yMin) * scale;
    size_t height = (uint16_t)(height_f32 + 0.5f);
    size_t i, j;

    int upScale = 800 / px;
    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            if (isInsideGlyf(glyf, (Point){(float)j, (float)i}, scale)) {
                if (upScale == 1)
                    DrawPixel(j + 50, i + 50, BLACK);
                else
                    DrawRectangle(j * upScale + 50, i * upScale + 50, upScale, upScale, BLACK);
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

int testRasterize(W_Font *font, char *character) {
    char tmp, key = 'a';

    int width = 1200;
    int height = 800;
    InitWindow(width, height, "testing fonts");
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
        drawChar(font, (uint8_t)key, 800);
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
#endif
