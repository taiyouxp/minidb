#include <stdio.h>
#include <string.h>
#define BYTES 4096
#define OFFSET0 BYTES * 0
#define OFFSET2 BYTES * 2

struct pg 
{
    char header[16];
    char registers[510][8];
} pg0, pg2;  


int main (void)
{
    // run 1 
    printf("%zu\n", sizeof(pg0));
    memset(&pg0, 0, BYTES);
    memset(&pg2, 0, BYTES);
    
    strcpy(pg0.registers[0], "borabill");
    strcpy(pg2.registers[0], "neymarjr");

    FILE* f1 = fopen("t2.db", "w+b");
    
    fwrite(&pg0, BYTES, 1, f1);
    fseek(f1, OFFSET2, SEEK_SET);
    fwrite(&pg2, BYTES, 1, f1);
    fclose(f1);
    
    // run 2
    FILE* f2 = fopen("t2.db", "r+b");
 
    struct pg pg3; 

    memset(&pg3, 0, BYTES);
    fseek(f2, OFFSET0, SEEK_SET);
    fread(&pg3, BYTES, 1, f2);
    
    printf("%s\n", pg3.registers[0]); // borabill

    fseek(f2, OFFSET2, SEEK_SET);
    fread(&pg3, BYTES, 1, f2);

    printf("%s\n", pg3.registers[0]); // neymarjr
    fclose(f2);
}