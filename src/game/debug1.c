#include <ultra64.h>
#include "game/debug.h"
#include <stdio.h>

s32 debug_log(const char *message, s32 num)
{
	FILE *debug_file = fopen("debug.log", "a");
    if (debug_file == NULL) {
        perror("Error opening debug.log");
        return 0;
    }

    fprintf(debug_file, message, num);
    fclose(debug_file);

	return 1;
}

s32 debug_log_float(const char *message, f32 num)
{
	FILE *debug_file = fopen("debug.log", "a");
    if (debug_file == NULL) {
        perror("Error opening debug.log");
        return 0;
    }

    fprintf(debug_file, message, num);
    fclose(debug_file);

	return 1;
}

s32 debug_erase()
{
	FILE *file = fopen("debug.log", "w"); // Open in write mode, truncates the file
    if (file == NULL) {
        perror("Error opening file");
        return 1; // Return error code
    }

    fclose(file); // Close the file
    printf("Log file erased successfully.\n");

    return 0;
}