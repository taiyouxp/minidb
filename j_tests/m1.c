#include <stdio.h>
#include <stdint.h>
#define BYTES 4096

typedef struct header
{
	uint16_t page_id;
	uint16_t page_type;
	uint16_t slots_filled;
	uint16_t register_size;
	uint64_t reserved;
} header;

/*
offset = byte that the register starts
lenght = size (in bytes) of the register
*/

typedef struct slot
{
	uint16_t offset;
	uint16_t lenght;
	uint16_t flags;
	uint16_t reserved;
} slot;

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
	slot * slots = (slot *)(buffer + sizeof(h));
	
	// writing on headers bytes=
	h->page_id = 1;
	
	//you can check the file.db using xxd file.db (linux) or Format-Hex file.db (WINDOWS)
	
	FILE * file = fopen("file.db", "w+b");
	
	write_page(file, 0, buffer);
	
	return 0;
}
