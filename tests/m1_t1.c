#include <stdio.h>
#include <string.h>
#define bytes 4096
#define offset0 0
#define offset2 8192
int main(void) 
{
    // testing memory addresses and offsets in depth
    
    // run 1
    char str1[] = "borabill";
    char str2[] = "neymarjr";
   /*  puts(str);
    memset(str, 'a', 4);
    puts(str);
     */
    char pg0[bytes], pg2[bytes];
    memset(pg0, 0, bytes);
    memset(pg2, 0, bytes);
    
    for(int i=16,j=0; i < 24; i++, j++)
    {
        pg0[i] = str1[j];
        pg2[i] = str2[j]; 
    }
    
    FILE* f1 = fopen("t1.db", "w+b");
    fwrite(pg0, bytes ,1, f1);
    fseek(f1, offset2, SEEK_SET);
    
    fwrite(pg2, bytes, 1, f1);
    
    fclose(f1);

    // run 2 
    FILE* f2 = fopen("t1.db", "r+b");
    
    char buf[bytes];
    memset(buf, 0, bytes);

    fseek(f2, offset0, SEEK_SET);
    size_t r2 = fread(buf, bytes, 1, f2);
    
    for(int i = 16; i < 24; i++)
        printf("%c\n", buf[i]);

    fseek(f2, offset2, SEEK_SET);
    fread(buf, offset2, 1, f2);
    
    for(int i = 16; i < 24; i++)
        printf("%c\n", buf[i]);
    
    fclose(f2);

    // now just do a 'xxd test1.db' to see both 
    // 'borabill' (offset 16) and 'neymarjr' (offset 8208, which is 8192 + 16) 
    // at their actual right offsets  
    return 0;
}