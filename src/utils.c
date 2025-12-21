#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>

#include "utils.h"

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

int mapFile(int fd, MappedFile *mappedFile) {
    int fileSize = lseek(fd, 0, SEEK_END);
    if (fileSize == -1) {
        perror("seeking file size error");
        close(fd);
        return 0;
    }
    mappedFile->data = mmap(NULL, fileSize, PROT_READ, MAP_PRIVATE, fd, 0);
    mappedFile->size = fileSize;
    close(fd);
    if (mappedFile->data == MAP_FAILED) {
        perror("mapping file error");
        return 0;
    }
    return 1;
}

void unmapFile(MappedFile mf) { munmap(mf.data, mf.size); }
