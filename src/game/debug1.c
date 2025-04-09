#include <ultra64.h>
#include <stdint.h>
#include "game/debug.h"
#include <stdio.h>
#include <types.h>

int debug_log(const char *message, int num)
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

int debug_log_float(const char *message, float num)
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

int debug_log_string(const char *message)
{
	FILE *debug_file = fopen("debug.log", "a");
    if (debug_file == NULL) {
        perror("Error opening debug.log");
        return 0;
    }

    fprintf(debug_file, message);
    fclose(debug_file);

	return 1;
}

int debug_erase()
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

void debug_log_coord(const struct coord *pos) {
    FILE *file = fopen("debug.log", "a"); // Open in append mode
    if (file) {
        fprintf(file, "coord: x = %.6f, y = %.6f, z = %.6f\n", pos->x, pos->y, pos->z);
        fclose(file);
    } else {
        perror("Failed to open debug.log");
    }
}