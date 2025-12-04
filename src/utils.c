#include <fcntl.h>
#include <sys/mman.h>
#include <stdio.h>

#include "utils.h"


void usage(const char *name) {
    printf("usage:\n%s fontFile.ttf\n", name);
}

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

char *mapFile(int fd, size_t *setSize) {
    int fileSize = lseek(fd, 0, SEEK_END);
    if (fileSize == -1) {
        perror("seeking file size error");
        close(fd);
        return NULL;
    }
    int lseekError = lseek(fd, 0, SEEK_SET);
    if (lseekError == -1) {
        perror("seek reseting to start of file error");
        close(fd);
        return NULL;
    }

    char *mapped = mmap(NULL, fileSize, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);
    if (mapped == MAP_FAILED) {
        perror("mapping file error");
        return NULL;
    }
    *setSize = fileSize;
    return mapped;
}