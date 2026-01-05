#include <fcntl.h>
#include <sys/mman.h>

#include "parser.h"
#include "utils.h"
#include "whiskerRenderer.h"
#include "whiskerRendererTypes.h"

void *parseFont(MappedFile fontFile) {
    W_Font *font = SAFE_MALLOC(sizeof(W_Font));
    font->parser = SAFE_MALLOC(sizeof(Parser));
    parseFont_i(fontFile, font->parser);
    return font;
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
