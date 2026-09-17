#include <stdio.h>
#include <stdint.h>
#include "m1.c"

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
	slot * slots = (slot *)(buffer + sizeof(h));
	
	FILE * file = fopen("file.db", "w+b");
	
	write_page(file, 0, buffer);
	
	return 0;
}