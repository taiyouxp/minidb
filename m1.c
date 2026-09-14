/* 
For now we will build: 
a 4kb page, a 16 bytes header and a 8 bytes register
    1. write_pg(n, bytes) and read_pg(n) in a common file. 
	2. record a register inside slot 0 in the page 2, end the process, open it and then read the same register.
	3. commit in the duo's repo with a NOTES.md, telling which byte of the file the register has gone. 
*/
#include <stdio.h>
#include <string.h>
#define BYTES 4096

struct pg 
{
    char header[16];
    char registers[510][8];
};  

void write_pg(int n, struct pg *p, FILE *f) 
{
    fseek(f, BYTES * n, SEEK_SET);
    fwrite(p, BYTES, 1, f);
    fclose(f);
}

void read_pg(int n, struct pg *p, FILE *f)
{
    fseek(f, BYTES * n, SEEK_SET);
    fread(p, BYTES, 1, f);
    fclose(f);
}

int main (void)
{
    struct pg p2 = {0}; // initializer 'zeroing' the entire struct - its just like memset() 
    strcpy(p2.registers[0], "borabill");
    FILE* f1 = fopen("file.db", "w+b");
    write_pg(2, &p2, f1); // n: 2 = page 2
    // ended process within the function
   
    // initiating a new process
    FILE* f2 = fopen("file.db", "r+b");
    struct pg to_read = {0}; 
    read_pg(2, &to_read, f2);
    
    printf("%s!\n", to_read.registers[0]); // register on slot 0 of page 2: borabill
    
    // 'automatically' writting on NOTES.md
    long register_offset = (2L * 4096) + 16 + (0 * 8);  // 8208 (0x00002010), checkable with 'xxd file.db' command
    FILE *md = fopen("NOTES.md", "w");
    fprintf(md, "The offset byte of the register '%s' is 0x%08lx (%ld)", to_read.registers[0], register_offset, register_offset);
    fclose(md); 
}