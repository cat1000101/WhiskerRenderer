#ifndef UTILS_H
#define UTILS_H

#include <unistd.h>

#define big16ToLittle16(x) (((x) & 0xFF00) >> 8 | ((x) & 0x00FF) << 8)
#define big32ToLittle32(x) (big16ToLittle16((x) & 0xFFFF) << 16 | big16ToLittle16(((x) & 0xFFFF0000) >> 16))

typedef struct {
    char *data;
    size_t size;
} mappedFile;

void usage(const char *name);
int parseArgs(int argc, char *argv[]);
mappedFile mapFile(int fd);

#endif