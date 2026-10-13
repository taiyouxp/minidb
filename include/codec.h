// explicit little endian
#ifndef CODEC_H
#define CODEC_H
#include <stdint.h>

static inline void put32(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)v;
    p[1] = (uint8_t)(v >> 8);
    p[2] = (uint8_t)(v >> 16);
    p[3] = (uint8_t)(v >> 24);
}

static inline uint32_t get32(const uint8_t *p)
{
    return 
    (uint32_t)p[0] | (uint32_t)p[1] << 8
    | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24;
}
#endif