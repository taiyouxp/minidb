#include <stdio.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdbool.h> // this is part of the cache structure

#define PG_SIZE 4096
#define H_SIZE 16 // header
#define SLOT_SIZE 8
#define R_QNT (PG_SIZE - H_SIZE) / SLOT_SIZE // registers that fills an entire page
#define CACHE_MAX_SIZE 50

/*
 *this file is a mess i know but many things here will be refactored into many different files and headers.
 *this cache / bufferpool implementation is very basic as i am yet to find better and more efficient ways of solving this
 *
 */

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

typedef struct frame
{
	page Page;
	bool is_dirty;
} frame;

typedef struct cache
{
	FILE * file;
	frame queue[CACHE_MAX_SIZE];
	unsigned int miss;
	unsigned int hit;
	int front;
	int rear;
} cache_manager;

long offset(page pg, int n_pg, int n_slot)
{
    return (n_pg * PG_SIZE) + sizeof(pg.hdr) + (n_slot * SLOT_SIZE);
}
/* write force to disk (kill -9 survive)
function need further testing  =
static int sync_page(FILE *f, int n, const page *p)
{
    if (fseek(f, (long)PG_SIZE * n, SEEK_SET)) return -1;
    if (fwrite(p, PG_SIZE, 1, f) != 1) return -1;
    if (fflush(f)) return -1; // buffer -> kernel 
    if (fsync(fileno(f))) return -1;  // kernel -> disk 
    return 0;
} 
*/

int write_pg(int n_pg, const page *p, FILE *f)
{
    if (!f || !p) {
        errno = EINVAL;
        perror("write_pg");
        return -1;
    }

    if (fseek(f, (long)PG_SIZE * n_pg, SEEK_SET) != 0) {
        perror("write_pg: fseek");
        fclose(f); // still close what we opened 
        return -1;
    }

    if (fwrite(p, PG_SIZE, 1, f) != 1) {
        perror("write_pg: fwrite");
        fclose(f);
        return -1;
    }

    if (fclose(f) != 0) {
        perror("write_pg: fclose");
        return -1;
    }

    return 0;
}

int read_pg(int n_pg, page *p, FILE *f)
{
    if (!f || !p) {
        errno = EINVAL;
        perror("read_pg");
        return -1;
    }

    if (fseek(f, (long)PG_SIZE * n_pg, SEEK_SET) != 0) {
        perror("read_pg: fseek");
        fclose(f);
        return -1;
    }

    if (fread(p, PG_SIZE, 1, f) != 1) {
        if (feof(f))
            fprintf(stderr, "read_pg: fread hit EOF (file too short?)\n");
        else
            perror("read_pg: fread");
        fclose(f);
        return -1;
    }

    if (fclose(f) != 0) {
        perror("read_pg: fclose");
        return -1;
    }

    return 0;
}

void new_register(page p, const char * data, int id, int reg_num)
{
	
}

