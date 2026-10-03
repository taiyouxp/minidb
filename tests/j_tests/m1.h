#ifndef M1_H
#define M1_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#define PG_SIZE 4096
#define H_SIZE 16 // header
#define SLOT_SIZE 8
#define R_QNT ((PG_SIZE - H_SIZE) / SLOT_SIZE) // registers that fills an entire page

typedef struct __attribute__((packed)) header {
    uint16_t n_slots; // bytes 0-1
    uint16_t reg_size; // bytes 2-3
    uint32_t n_page; // bytes 4-7
    uint64_t reserved;// bytes 8-15
} header;

typedef struct __attribute__((packed)) reg {
    uint16_t id; // 2 bytes
    uint16_t reg_num; // 2 bytes
    char data[4]; // 4 bytes
} reg;

typedef struct page {
    header hdr;
    reg registers[R_QNT]; // 510 registers (510 slots)
} page;

long offset(int n_pg, int n_slot);
int write_pg(int n_pg, const page *p, FILE *f);
int read_pg(int n_pg, page *p, FILE *f);
/*
 * returns the next free page but doesnt increment into the page 0. this will be done in the new_register function
 */
int next_free_page(FILE *f);

#endif