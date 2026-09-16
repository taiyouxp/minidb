#include "typewriter.h"
#include <stdio.h>
#include <time.h>
#define HISTORY_PATH "../docs/HISTORY.txt"
// later today i will implement a way to show the author of the message before it

void write_note(const char* text)
{
	FILE * history = fopen(HISTORY_PATH, "a");
	
	if (history == NULL)
		return;
	
	time_t current_time;
	time(&current_time);
	struct tm * info = localtime(&current_time);
	
	char date_time[20];
	strftime(date_time, sizeof(date_time), "%d-%m-%Y %H:%M:%S", info);
	
	fprintf(history, "[%s] \"%s\"\n", date_time, text);
	
	fclose(history);
}