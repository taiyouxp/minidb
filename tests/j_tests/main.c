#include "m1.h"

void write_page(FILE * file, uint32_t n, const uint8_t * buffer)
{
	long offset = (long)n * BYTES;
	
	fseek(file, offset, SEEK_SET);
	fwrite(buffer, BYTES, 1, file);
}

int main(void)
{	
/*
	to write the page into the file
	1. a continuous 4096 bytes buffer
	2. casting the first 16 bytes into a header
	3. casting the bytes next to the header into slots as we need them (byte 15 - byte n)
	4. for now i am casting a single slot for testing porpuses
	*/
	uint8_t buffer[BYTES] = {0};
	header * h = (header *)buffer;
	slot * slots = (slot *)(buffer + sizeof(header));
	
	slots->offset=16;
	FILE * file = fopen("file.db", "w+b");
	
	write_page(file, 0, buffer);
	int i = 16;
	printf("%x\n" ,i);	
	return 0;
}