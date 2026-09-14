#include <stdlib.h>
#include <stdio.h>
#define bytes 4096 
#define r_bytes 4080
/* 
testing language structures/features according to the proposal of M1:
t1. persistence in a file 
t2. page addressing
t3. slot offset math

UPDATE: this is deprecated and turned mainly into some kind of "documentation" on the bottom of this file, 
later i will write into a dedicated file.   
*/

struct pg 
{
    char header[16]; // 16 bytes header: counting how many slots are fulfilled
    // theres 3 slots occupied in every page initialization though.
    char r_data[r_bytes]; 
    // slot 0: byte 0 to 7 -> there goes register 0 data
    // 4080 / 8 = 509 registers can be written
} pg0, pg1, pg2; 
    
  
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
    puts("[T1]\n"); 
    // t1.1: seeing the memory allocation of the struct fields (should be contiguous - pass)
    printf("page 0 address: %p\nheader address (same as page 0): %p\nregisters addresses (slots): %p\n", &pg0, &pg0.header, &pg0.r_data);
    
    // t1.2: seeing if that the declared struct allocated size is 4096 (pass)
    printf("size of page: %zu bytes\n", sizeof(pg0)); 

    // t1.3: filling slot 0 with an array of 8 chars
    // (these kind of operation needs to be modularized later)
    
    char name[8] = "borabill"; 
    for (int i = 0; i < 8; ++i) 
    {
        pg0.r_data[i] = name[i];
        pg1.r_data[i] = name[i];
        pg2.r_data[i] = name[i];
    } 

    // puts(pg0.r_data);
    
    // t1.4 - t1.6: opening a file and using fwrite() to store
    // the first 'slot' of page 0 then assign the same pattern for the 'page 1'
    // to see the persistence of data. (pass)
    FILE* f1 = fopen("file.bin", "w+b");
    fwrite(pg0.r_data, sizeof(pg0.r_data[0]), r_bytes, f1);
    fclose(f1);
     
    f1 = fopen("file.bin", "rb");
    size_t r1 = fread(pg0.r_data, sizeof pg0.r_data[0], sizeof(name[0]), f1);
    if (r1 == sizeof(name[0]))
    {
        printf("Array (register) at %p (slot 0 of page 0) read successfully, contents:\n", (void*)&pg1.r_data);
        puts(pg1.r_data);
    } 
    fclose(f1);
    
    puts("\n[T2]\n");
    /*
     * t2 figuring out how to add page addressing
        i want to make sure that the page 2 will be written in the correct address: 
        - pg0: 0 -> 4096 
        - pg1: 4096 -> 8192
        - pg2: (2 * 4096) = 8192 -> ...
    */

    struct pg* ptr0 = &pg0; 
    struct pg* ptr1 = &pg1;
    struct pg* ptr2 = &pg2;
    
    long offset_st0, offset_st1, offset_st2; 
    offset_st0 = (long) ptr0;
    offset_st1 = (long) ptr1;
    offset_st2 = (long) ptr2;
    long offset2 = offset_st2 - offset_st0; // 2 * 4096 = 8192 (in decimal)

    printf("dec:\n %ld\n %ld\n offset 2: %ld\n", offset_st0, offset_st2, offset2);
    printf("hex:\n %p\n %p\n offset 2: %p\n", ptr0, ptr1, ptr2);

    // now i will try to write data inside the slot 0 of page 2 with this offset in mind in the same file using fseek()
    fseek(f1, offset2, SEEK_SET); // this offset needs to be automatically calculated (obviously)
    f1 = fopen("file.bin", "ab"); // append bytes mode 
    fwrite(pg2.r_data, sizeof(pg2.r_data[0]), r_bytes, f1);
    fclose(f1);
    
    f1 = fopen("file.bin", "r+b");
    r1 = fread(pg2.r_data, sizeof pg2.r_data[0], sizeof(name[0]), f1);
    
    if (r1 == sizeof(name[0]))
    {
        printf("Array (register) at %p (slot 0 of page 2) read successfully, contents:\n", (void*)&pg2.r_data);
        puts(pg2.r_data);
    }
    fclose(f1);
    return 1;
}

/* (12.9 - sat)
 * So i've realized that this offset calculation to write a page into the actual right place inside the file is little bit more complex
 * than i expected. atleast from these previously 'tests' i can infer the actual memory offset positions of the 'pages'  
 * inside the program 'heap' (i think this is correct) which for my current goal is slot 0 for page 2 (8192).
 * for tomorrow: 
 *  1. resolve how to actually do the page addressing for the file and so fetch the data from there.
 *  2. improve the way to identify actual registers instead of using a multiple of 8 as reference... 
 *  3. finally define the structure of metadata page and the actual page and how to make use of the header of each one.
 *  4. to finish M1: define the structure of write_pg(), read_pg(), the main loop to keep the program alive 
 *  as a process so we can use kill command from the kernel (?).
 *  5. Answer in one paragraph: Why does the DBMS manage its own pages instead of leaving that to the operating system? 
 *  [bonus]. write the byte addresses automatically to NOTES.md with a 
 *  helper function that could work as 'operations' log. 
 *
 * (13.9 - sun)
 * this is tomorrow: 
 * i resolved '1' from yesterday
 * still need to implement '2', '3', '4' and '5' - this is for the new "tomorrow"
 *
 * (14.9 - mon - the new tomorrow from yesterday)
 * fix: so i thought i had resolved '1' but now m1_test1.c should be the correct way of I/O handling with offsets.
 * i am writting a new test file to use that approach with structs. (this one is now deprecated)
 * the new file is m1_t2.c and will mainly be on m1 structure. we're going to deliver almost all previous 'tasks' tho. 
 * i am just going to set m1 as simple as possible (only what was asked in classes for 15.9).
 * we should be improving the repo according to the advance of the modules, 
 * and with extra stuff that i should it be cool to have, culminating into a main program.   
 * */