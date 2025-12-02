#include "parser.h"

uint32_t calcTableChecksum(uint32_t *table, uint32_t numberOfBytesInTable) {
    uint32_t sum = 0;
    uint32_t nLongs = (numberOfBytesInTable + 3) / 4;
    while (nLongs-- > 0)
        sum += *table++;
    return sum;
}

Font *parseFont(FILE *fontFile) {
    return NULL;
}