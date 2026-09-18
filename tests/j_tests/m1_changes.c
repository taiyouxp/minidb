#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#define PG_SIZE 4096
#define H_SIZE 16 // header
#define SLOT_SIZE 8
#define R_QNT (PG_SIZE - H_SIZE) / SLOT_SIZE // registers that fills an entire page 

/*
grounds prepared so i can work on this later today - jean
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

static short pg_count = 0; // memory control, sync() persists 

long travel(page pg, int n_pg, int n_slot)
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

// return next free page 
int alloc(void)
{
    return (int)pg_count++; // pg sequence
}

void new_register(page p, const char * data, int id, int reg_num)
{
	
}

int main (void)
{
	// alloc() and sync() still in need to be used with write_pg and read_pg
    page p2 = {0};
    
    p2.hdr.n_slots = 1;
    p2.hdr.reg_size = sizeof(reg);
    p2.hdr.n_page = 2;
    
    p2.registers[0].id = 1;
    p2.registers[0].reg_num = 1001;
    memcpy(p2.registers[0].data, "abcd", 4);
    
    FILE *f1 = fopen("m1.db", "w+b");
    write_pg(2, &p2, f1);
	
    FILE *f2 = fopen("m1.db", "r+b");
	page to_read = {0};
    read_pg(2, &to_read, f2);
    
    printf("id=%u reg_num=%u data=%.4s\n",
           to_read.registers[0].id,
           to_read.registers[0].reg_num,
           to_read.registers[0].data); 
    
    // 'automatically' writting on NOTES.md - will use within typewritter.c to truly automate operation logging
    long register_offset = displacement(to_read, 2, 0);  // 8208 (0x00002010), checkable with 'xxd file.db' command
    
    FILE* md = fopen("../docs/NOTES.md", "w");
    fprintf(md, "The offset byte of the register id=%u reg_num=%u data=%.4s is 0x%08lx (%ld)", 
        to_read.registers[0].id,
        to_read.registers[0].reg_num,
        to_read.registers[0].data, 
        register_offset, 
        register_offset);
    
    fclose(md); 
 
}