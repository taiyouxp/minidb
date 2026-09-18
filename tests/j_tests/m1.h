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
	uint16_t length;
	uint16_t flags;
	uint16_t reserved;
} slot;


void write_page(FILE * file, uint32_t n, const uint8_t * buffer);