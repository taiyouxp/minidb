#include <stdio.h>
#include <string.h>
#include <errno.h>

#define PG_SIZE 4096
#define H_SIZE 16 // header
#define SLOT_SIZE 8
#define R_QNT (PG_SIZE - H_SIZE) / SLOT_SIZE // registers that fills an entire page 

struct page
{
    char header[H_SIZE];
    char registers[R_QNT][SLOT_SIZE]; // 510 slots for registers of 8 bytes (slot size)
};  

long displacement(struct page pg, int n_pg, int n_slot)
{
    return (n_pg * PG_SIZE) + sizeof(pg.header) + (n_slot * SLOT_SIZE);
}

int write_pg(int n_pg, const struct page *p, FILE *f)
{
    if (!f || !p) {
        errno = EINVAL;
        perror("write_pg");
        return -1;
    }

    if (fseek(f, (long)PG_SIZE * n_pg, SEEK_SET) != 0) {
        perror("write_pg: fseek");
        fclose(f);          /* still close what we opened */
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

int read_pg(int n_pg, struct page *p, FILE *f)
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

int main (void)
{
    struct page p2 = {0}; // initializer 'zeroing' the entire struct - its just like memset() 
    strcpy(p2.registers[0], "borabill");
    FILE* f1 = fopen("m1.db", "w+b");
    write_pg(2, &p2, f1); // n: 2 = page 2
    // ended process within the function
   
    // initiating a new process
    FILE* f2 = fopen("m1.db", "r+b");
    struct page to_read = {0}; 
    read_pg(2, &to_read, f2);
    
    printf("%s!\n", to_read.registers[0]); // register on slot 0 of page 2: borabill
    
    // 'automatically' writting on NOTES.md - will use typewritter.c for trully automate log writting
    long register_offset = displacement(to_read, 2, 0);  // 8208 (0x00002010), checkable with 'xxd file.db' command
    
    FILE* md = fopen("../docs/NOTES.md", "w");
    fprintf(md, "The offset byte of the register '%s' is 0x%08lx (%ld)", to_read.registers[0], register_offset, register_offset);
    fclose(md); 
}