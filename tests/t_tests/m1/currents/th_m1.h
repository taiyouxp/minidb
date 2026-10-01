// this is the new header so the page will become
// a bytearray replacing the previously 
// structs implementation in order to work 
// with a page as mutable buffer

#include <stdint.h>
#define PG_SIZE 4096
#define H_SIZE 16
#define SLOT_SIZE 8
#define MAX_SLOTS ((PG_SIZE - H_SIZE) / SLOT_SIZE) // 510 

typedef struct page { uint8_t b[PG_SIZE]; } page;

// page header field offsets 
#define PH_NSLOTS 0     // u16 occupied slots
#define PH_REGSIZE 2    // u16 record size    
#define PH_PGNUM 4      // u32 this page's number 
#define PH_CRC 8        // u32 crc32 
#define PH_LSN 12       // u32 reserved for M7 (12-15)