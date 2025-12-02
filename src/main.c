#include <stdio.h>

#include "parser.h"
#include "renderer.h"

void usage(const char *name) {
    printf("usage:\n%s fontFile.ttf\n", name);
}

FILE *parseArgs(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Error: Incorrect number of arguments\n");
        usage(argv[0]);
        return NULL;
    }
    FILE *fptr = fopen(argv[1], "rb");
    if (!fptr) {
        perror(argv[1]);
        usage(argv[0]);
        return NULL;
    }
    return fptr;
}

int main(int argc, char *argv[]) {
    FILE *fontFile = parseArgs(argc, argv);
    if (!fontFile) {
        return 1;
    }

    W_Font *font = parseFont(fontFile);
    printf("render time\n");
    int success = drawChar(font, 'a');

    fclose(fontFile);
    return success;
}