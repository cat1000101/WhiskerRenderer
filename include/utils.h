#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define UNUSED(x) (void)(x)
// used for structs
#define IS_ZERO(x) ({                    \
    uint8_t _zeros[sizeof(x)] = {0};     \
    memcmp(&x, &_zeros, sizeof(x)) == 0; \
})

// #define SWAP_ENDIAN_16(x) (((uint16_t)(x) & 0xFF00) >> 8 | ((uint16_t)(x) & 0x00FF) << 8)
// #define SWAP_ENDIAN_32(x) (SWAP_ENDIAN_16((uint32_t)(x) & 0xFFFF) << 16 | SWAP_ENDIAN_16(((uint32_t)(x) & 0xFFFF0000) >> 16))

#define SWAP_ENDIAN_16(x) __builtin_bswap16(x)
#define SWAP_ENDIAN_32(x) __builtin_bswap32(x)

#define READ_TYPE_ENDIAN(type)                                                     \
    static inline type read_##type##_endian(void *ptr) {                           \
        if (sizeof(type) == 2) {                                                   \
            return (type)SWAP_ENDIAN_16(*(uint16_t *)(ptr));                       \
        } else if (sizeof(type) == 4) {                                            \
            return (type)SWAP_ENDIAN_32(*(uint32_t *)(ptr));                       \
        } else {                                                                   \
            fprintf(stderr, "READ_SIZE_ENDIAN used with undefined size for it\n"); \
            exit(1);                                                               \
            return 0;                                                              \
        }                                                                          \
    }

READ_TYPE_ENDIAN(uint16_t)
READ_TYPE_ENDIAN(int16_t)
READ_TYPE_ENDIAN(uint32_t)
READ_TYPE_ENDIAN(int32_t)

#define OFFSET_OF(type, member) ((size_t)&(((type *)0)->member))

#define SAFE_MALLOC(size) ({  \
    void *ptr = malloc(size); \
    if (!ptr) {               \
        perror(__func__);     \
        exit(EXIT_FAILURE);   \
    }                         \
    ptr;                      \
})

#define CREATE_VEC(name, type)                                   \
    typedef struct {                                             \
        type *data;                                              \
        size_t len;                                              \
        size_t cap;                                              \
    } name;                                                      \
                                                                 \
    static void name##_init(name *v) {                           \
        v->data = NULL;                                          \
        v->len = 0;                                              \
        v->cap = 0;                                              \
    }                                                            \
                                                                 \
    static void name##_push(name *v, type value) {               \
        if (v->len == v->cap) {                                  \
            v->cap = v->cap ? v->cap * 2 : 4;                    \
            type *tmp = realloc(v->data, v->cap * sizeof(type)); \
            if (!tmp) {                                          \
                perror(#name " realloc\n");                      \
                exit(1);                                         \
            }                                                    \
            v->data = tmp;                                       \
        }                                                        \
        v->data[v->len++] = value;                               \
    }                                                            \
                                                                 \
    static void name##_free(name *v) {                           \
        free(v->data);                                           \
        v->data = NULL;                                          \
        v->len = 0;                                              \
        v->cap = 0;                                              \
    }

typedef struct {
    uint8_t *data;
    size_t size;
} mappedFile;

void usage(const char *name);
int parseArgs(int argc, char *argv[]);
mappedFile mapFile(int fd);
void unmapFile(mappedFile mf);
#endif