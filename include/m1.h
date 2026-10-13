#ifndef M1_H
#define M1_H

#include <sys/stat.h>
#include <sys/types.h>

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>

#define PG_SIZE 4096
#define H_SIZE 16 // header size
#define SLOT_SIZE 8
#define R_QNT ((PG_SIZE - H_SIZE) / SLOT_SIZE) // registers that fills an entire page

typedef struct __attribute__((packed)) header {
    uint16_t n_slots; // bytes 0-1
    uint16_t reg_size; // bytes 2-3
    uint32_t n_page; // bytes 4-7
    uint64_t reserved;// bytes 8-15
} header;

typedef struct __attribute__((packed)) reg {
    uint32_t id; // 4 bytes
    char data[4]; // 4 bytes
} reg;

typedef struct page {
    header hdr;
    reg registers[R_QNT]; // 510 registers (510 slots)
} page;

_Static_assert(sizeof(header) == H_SIZE, "header must be 16 bytes");
_Static_assert(sizeof(reg) == SLOT_SIZE, "register must be 8 bytes");
_Static_assert(sizeof(page) == PG_SIZE, "page must be 4096 bytes");

long offset_desloc(int n_pg, int n_slot);

int write_pg(int n_pg, const page *p, FILE *f);
int read_pg(int n_pg, page *p, FILE *f);
void page_init(page *p, int n_pg);

void serialise(const reg *r, uint8_t out[SLOT_SIZE]);
void deserialise(const uint8_t in[SLOT_SIZE], reg *r);
int  insert(int n_pg, page *p, FILE *f, const reg *r);  // slot used, or -1

#endif