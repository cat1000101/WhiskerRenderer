#include <stdio.h>

void usage(const char* name) {
    printf("usage:\n%s fontFile.ttf\n",name);
}

FILE* parseArgs(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Error: Incorrect number of arguments\n");
        usage(argv[0]);
        return NULL;
    }
    FILE* fptr =  fopen(argv[1], "rb");
    if (!fptr) {
        perror(argv[1]);
        usage(argv[0]);
        return NULL;
    }
    return fptr;
}

int main(int argc, char *argv[]) {
    FILE* font = parseArgs(argc,argv);
    if (!font) {
        return 1;
    }

    printf("render time\n");

    fclose(font);
    return 0;
}