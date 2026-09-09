/* 
For now we will build: 
a 4kb page, a 16 bytes header and a 8 bytes register
    1. write_pg(n, bytes) and read_pg(n) in a common file. 
	2. record a register inside slot 0 in the page 2, end the process, open it and then read the same register.
	3. commit in the duo's repo with a NOTES.md, telling which byte of the file the register has gone. 
*/

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#define bytes 4096 // 4096 bytes - converted with char type

struct pg 
{
    char header[16]; // 16 bytes header: counting how many slots are fulfilled
    // theres 3 slots occupied in every page initialization though.
    char r_data[bytes - 16]; 
    // slot 0: byte 0 to 7 -> there goes register 0 data
    // 4080 / 8 = 509 registers can be written
} pg0, pg1; 
    
  
void write_pg(int n); // n = page number. idk if we need the other parameter as i defined a macro. 

/*    
    if (n == 0) 
        should init metadata 'page' telling how much pages are stored inside the file  
        (it should fetch the count of pages and its remaining size 
        to store a register (so we only need to fetch data of eachs page header??). 
        i think this needs indexing, a headache for later.  
    else 
        write a new page into the file (storing based on a byte offset operation). 
    */       

void read_pg(int n);

int main(void)
{

    // t1: seeing the memory allocation of the struct fields (should be contiguous - pass)
/*     printf("%p %p %p \n", &pg_0, &pg_0.header, &pg_0.r_data);
    
    // t2: seeing if that the declared struct allocated is 4096 (pass)
    printf("%zu\n", sizeof(pg_0)); */
    
    // t3: filling slot 0 with an array of 8 chars
    // these kind of operation needs to be modularized later
    char name[8] = "borabill"; int slot = 0; 
    for (int i = 0; i < 8; ++i) 
    {
        pg0.r_data[i] = name[i]; 
        if (i % 8 == 0) 
        {
            pg0.header[slot] += 1; // first slot, useless for now
            slot++;     
        } // wip :)
    }
    // t4: opening a file and using fwrite() to store the actual page 
    FILE* f1 = fopen("file.bin", "wb");
    assert(f1);
    size_t r1 = fwrite(pg0.r_data, sizeof(pg0.r_data[0]), bytes, f1);
    printf("wrote %zu elements out of %d requested\n", r1, bytes);

    
    FILE* f2 = fopen("file.bin", "rb");
    size_t r2 = fread(pg1.r_data, sizeof pg1.r_data[0], bytes, f2);
    // wip :) 
    return 1;
}
/* to do:
    1. write the next steps properly   
*/