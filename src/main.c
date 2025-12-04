#include <stdio.h>
#include <sys/mman.h>

#include "parser.h"
#include "renderer.h"
#include "utils.h"

int main(int argc, char *argv[]) {
    int fontFile = parseArgs(argc, argv);
    if (fontFile == -1) return 1;

    mappedFile fontMapped = mapFile(fontFile);
    if (!fontMapped.data) return 2;

    W_Font *font = parseFont(fontMapped);
    if (!font) return 3;
    printf("render time\n");
    int success = drawString(font, "meow");

    munmap(fontMapped.data, fontMapped.size);
    return success;
}