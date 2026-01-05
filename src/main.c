#include <fcntl.h>
#include <stdio.h>

#include "whiskerRenderer.h"

#include "raylib.h"

void usage(const char *name);
int parseArgs(int argc, char *argv[]);

int main(int argc, char *argv[]) {
    int fontFile = parseArgs(argc, argv);
    if (!fontFile) return 1;

    MappedFile fontMapped = (MappedFile){0};
    if (!mapFile(fontFile, &fontMapped)) return 1;

    void *font = parseFont(fontMapped);
    if (!font) return 1;

    InitWindow(1200, 800, "testing fonts");
    char text[256] = {0};
    size_t pos = 0;
    while (!WindowShouldClose()) {
        char tmp = GetCharPressed();
        if (tmp) {
            text[pos++] = tmp;
        }
        BeginDrawing();
        ClearBackground(RAYWHITE);
        raylibDrawString(font, text, 52, 50, 50);
        EndDrawing();
    }
    CloseWindow();

    unmapFile(fontMapped);
    return 0;
}

void usage(const char *name) { printf("usage:\n%s fontFile.ttf\n", name); }

int parseArgs(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Error: Incorrect number of arguments\n");
        usage(argv[0]);
        return -1;
    }
    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror(argv[1]);
        usage(argv[0]);
        return fd;
    }
    return fd;
}
