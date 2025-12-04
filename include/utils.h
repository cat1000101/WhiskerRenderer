#ifndef UTILS_H
#define UTILS_H

#include <unistd.h>

typedef struct {
    char *data;
    size_t size;
} mappedFile;

void usage(const char *name);
int parseArgs(int argc, char *argv[]);
mappedFile mapFile(int fd);

#endif