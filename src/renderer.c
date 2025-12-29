#include <stddef.h>
#include <stdio.h>

#include "renderer.h"
#include "whiskerRenderer.h"
#include "whiskerRendererTypes.h"

#include "raylib.h"

// p(t) = (1-t)2p0 + 2t(1-t)p1 + t2p2
typedef struct {
    int x;
    int y;
    int onContour;
} Point;
Point getAbsoluteXY(SimpleGlyfChar *glyf, size_t contourNum, size_t index) {
    Point result = {0};
    if (contourNum > glyf->contourNum) {
        printf("wtf contour num too big\n");
        return result;
    }
    if (index > glyf->contours[contourNum].length + 1) {
        printf("wtf index too big\n");
        return result;
    }
    if (index == glyf->contours[contourNum].length) index = 0;
    result.x = glyf->contours[contourNum].xFontUnit[index] - glyf->boundingBox.xMin;
    result.y = glyf->boundingBox.yMax - (glyf->contours[contourNum].yFontUnit[index] - glyf->boundingBox.yMin);
    result.onContour = glyf->contours[contourNum].flags[index] & 0b1;
    return result;
}

int renderCharBitmap(W_Font *font, uint8_t c, size_t px) {
    SimpleGlyfChar glyf = font->parser.tables.glyf.chars[c];
    float scale = (float)px / (float)font->parser.tables.head.unitsPerEm;
    float width = (glyf.boundingBox.xMax - glyf.boundingBox.xMin) * scale;
    float hight = (glyf.boundingBox.yMax - glyf.boundingBox.yMin) * scale;
    printf("rendering '%c': scale %f width/hight %f/%f\n", c, scale, width, hight);

    for (size_t i = 0; i < glyf.contourNum; i++) {
        for (size_t j = 0; j < glyf.contours[i].length; j++) {
            Point current = getAbsoluteXY(&glyf, i, j);
            Point next = getAbsoluteXY(&glyf, i, j + 1);
            int xAbsolute = current.x;
            int yAbsolute = current.y;
            int nextXAbsolute = next.x;
            int nextYAbsolute = next.y;

            if (current.onContour) {
                DrawCircle(xAbsolute + 50, yAbsolute + 50, 5, RED);
            } else {
                DrawCircle(xAbsolute + 50, yAbsolute + 50, 5, YELLOW);
            }
            DrawLine(xAbsolute + 50, yAbsolute + 50, nextXAbsolute + 50, nextYAbsolute + 50, SKYBLUE);
        }
    }

    return 0;
}

int drawString_i(W_Font *font, char *character) {
    int width = 900;
    int hight = 600;
    printf("max width/hight %d/%d\n", width, hight);
    InitWindow(width, hight, "testing fonts");
    // while (!WindowShouldClose()) {}
    BeginDrawing();
    ClearBackground(RAYWHITE);
    renderCharBitmap(font, 'a', 42);
    EndDrawing();

    while (1) {
    }
    CloseWindow();

    return 0;
}
