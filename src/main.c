#include <stdio.h>

#include "whiskerRenderer.h"
#include "whiskerRendererTypes.h"

int main(int argc, char *argv[]) {
    int fontFile = parseArgs(argc, argv);
    if (!fontFile) return 1;

    MappedFile fontMapped = (MappedFile){0};
    if (!mapFile(fontFile, &fontMapped)) return 1;

    W_Font *font = parseFont(fontMapped);
    if (!font) return 1;
    printf("render time\n");
    int success = drawString(font, "meow");

    unmapFile(fontMapped);
    return success;
}
