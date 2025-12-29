#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define UNUSED(x) (void)(x)
#define TODO(message)                                                      \
    do {                                                                   \
        fprintf(stderr, "%s:%d: TODO: %s\n", __FILE__, __LINE__, message); \
        abort();                                                           \
    } while (0)
#ifdef NDEBUG
#define UNREACHABLE(fmt, ...)                                     \
    do {                                                          \
        fprintf(stderr, "UNREACHABLE: " fmt "\n", ##__VA_ARGS__); \
        abort();                                                  \
    } while (0)
#else
#define UNREACHABLE(fmt, ...)                                                                \
    do {                                                                                     \
        fprintf(stderr, "%s:%d: UNREACHABLE: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__); \
        abort();                                                                             \
    } while (0)
#endif
#define ERROR_OUT(fmt, ...)                                                            \
    do {                                                                               \
        fprintf(stderr, "%s:%d: error: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__); \
        return 1;                                                                      \
    } while (0)

// used for structs
#define IS_ZERO(x)                           \
    ({                                       \
        uint8_t _zeros[sizeof(x)] = {0};     \
        memcmp(&x, &_zeros, sizeof(x)) == 0; \
    })

// #define SWAP_ENDIAN_16(x) (((uint16_t)(x) & 0xFF00) >> 8 | ((uint16_t)(x) & 0x00FF) << 8)
// #define SWAP_ENDIAN_32(x) (SWAP_ENDIAN_16((uint32_t)(x) & 0xFFFF) << 16 | SWAP_ENDIAN_16(((uint32_t)(x) & 0xFFFF0000)
// >> 16))

#define SWAP_ENDIAN_16(x) __builtin_bswap16(x)
#define SWAP_ENDIAN_32(x) __builtin_bswap32(x)

#define READ_TYPE_ENDIAN(type)                                               \
    static inline type read_##type##_endian(void *ptr) {                     \
        if (sizeof(type) == 2) {                                             \
            return (type)SWAP_ENDIAN_16(*(uint16_t *)(ptr));                 \
        } else if (sizeof(type) == 4) {                                      \
            return (type)SWAP_ENDIAN_32(*(uint32_t *)(ptr));                 \
        } else {                                                             \
            UNREACHABLE("READ_SIZE_ENDIAN used with undefined size for it"); \
            return 0;                                                        \
        }                                                                    \
    }

READ_TYPE_ENDIAN(uint16_t)
READ_TYPE_ENDIAN(int16_t)
READ_TYPE_ENDIAN(uint32_t)
READ_TYPE_ENDIAN(int32_t)

static inline int isFlagBitSet(size_t flag, size_t index) { return ((flag >> index) & 1) == 1; }

#define OFFSET_OF(type, member) ((size_t)&(((type *)0)->member))

#define STR2(x) #x
#define STR(x) STR2(x)

// it is "safe"
#define SAFE_MALLOC(size)                           \
    ({                                              \
        void *ptr = malloc(size);                   \
        if (!ptr) {                                 \
            perror(__FILE__ ":" STR(__LINE__) ":"); \
            abort();                                \
        }                                           \
        ptr;                                        \
    })

#define CREATE_VEC(name, type)                                   \
    typedef struct {                                             \
        type *data;                                              \
        size_t len;                                              \
        size_t cap;                                              \
    } name;                                                      \
                                                                 \
    static inline void name##_init(name *v) {                    \
        v->data = NULL;                                          \
        v->len = 0;                                              \
        v->cap = 0;                                              \
    }                                                            \
                                                                 \
    static inline void name##_push(name *v, type value) {        \
        if (v->len == v->cap) {                                  \
            v->cap = v->cap ? v->cap * 2 : 16;                   \
            type *tmp = realloc(v->data, v->cap * sizeof(type)); \
            if (!tmp) {                                          \
                perror(#name " realloc\n");                      \
                abort();                                         \
            }                                                    \
            v->data = tmp;                                       \
        }                                                        \
        v->data[v->len++] = value;                               \
    }                                                            \
                                                                 \
    static inline void name##_free(name *v) {                    \
        free(v->data);                                           \
        v->data = NULL;                                          \
        v->len = 0;                                              \
        v->cap = 0;                                              \
    }

CREATE_VEC(i16Vec, int16_t)
CREATE_VEC(u8Vec, uint8_t)

#endif
