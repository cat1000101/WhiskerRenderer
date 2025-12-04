#include <stdio.h>
#include <sys/mman.h>

#include "utils.h"
#include "parser.h"
#include "renderer.h"

int main(int argc, char *argv[]) {
    int fontFile = parseArgs(argc, argv);
    if (fontFile == -1) return 1;

    size_t fontSize = 0;
    char *fontMapped = mapFile(fontFile, &fontSize);
    if (!fontMapped) return 2;

    W_Font *font = parseFont(fontMapped);
    printf("render time\n");
    int success = drawChar(font, 'a');

    munmap(fontMapped,fontSize);
    return success;
}