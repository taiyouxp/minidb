#include "m2.h"

int main(void)
{
	/*
	 * main flow:
	 *
	 * ->opens file, creates one if it doesnt exist
	 * ->starts cache
	 * ->checks if file is empty, if so, writing will start on page 1
	 *
	 */
	 
	FILE *f = fopen("tests.db", "r+b");
	if (!f) f = fopen("tests.db", "w+b");
	if (!f) {
		perror("main: fopen");
		return 1;
	}
	
	fseek(f, 0, SEEK_END);
	long file_size = ftell(f);
	
	if (file_size == 0) {
		page p0 = {0};
		p0.hdr.n_page = 0;
		strcpy(p0.registers[0].data, "jean");
		
		if ((write_pg(0, &p0, f) != 0)) {
			fclose(f);
			return -1;
		}
	}
	
	fseek(f, 0, SEEK_SET);
	
	cache c;
	cache_init(&c, f);
	
	page p = {0};

	fclose(f);
	
	return 0;
}