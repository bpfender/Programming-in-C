#include "textfile.h"
#include <stdlib.h>

#define BUFF_SIZE 50
#define BUFF_FACT 4

/* Macro defines maximum value of line_t type */
#define LINE_T_MAX (line_t) ~0

#define ON_ERROR(STR)     \
    fprintf(stderr, STR); \
    exit(EXIT_FAILURE)

FILE* openFile(char* filename) {
    FILE* file = fopen(filename, "r");

    if (!file) {
        ON_ERROR("Cannot open dictionary file\n");
    }
    return file;
}

line_t getLine(char** buffer, line_t* size, FILE* file) {
    line_t i = 0;
    long int file_pos = ftell(file);

    /* If buffer has not been initialised, buffer is allocated and size is set */
    if (!*buffer) {
        *size = BUFF_SIZE;
        *buffer = bufferAllocHandler(NULL, *size);
    }

    /* Use fgets to read into buffer. Space remaining in buffer is decremented 
       based on how far into the file has been read. Allows for dyanmic resizing
       of buffer. The while loop handles \n characters. */
    while (fgets(*buffer + i, *size - i, file)) {
        /* Get number of characters read */
        i = ftell(file) - file_pos;

        /* When '\n' read of the first time stream reaches the
        EOF, fgets will end up here. Can return number of characters read */
        if (feof(file) || (*buffer)[i - 1] == '\n') {
            return i;
        }

        /* If fgets stopped reading and last char wasn't \n of eof, buffer was 
        filled and needs to be expanded. Conditions below expand buffer up to
        maximum size of LINE_T_MAX, at which point the function will fail */
        if (*size == LINE_T_MAX) {
            ON_ERROR("Line buffer length exceeded. Exiting...\n");
        }

        if (LINE_T_MAX / BUFF_FACT <= *size) {
            *size = LINE_T_MAX;
        } else {
            *size *= BUFF_FACT;
        }
        *buffer = bufferAllocHandler(*buffer, *size);
    }
    /* Returns 0 on eof or error. This should be the only case where 0 can be
    returned as newlines will always include at least the line ending chars */
    return 0;
}

void truncateLineEnd(char* buffer, line_t* len) {
    line_t size = *len;

    if (size && buffer[size - 1] == '\n') {
        if (size >= 2 && buffer[size - 2] == '\r') {
            buffer[size - 2] = '\0';
            *len = size - 2;
        } else {
            buffer[size - 1] = '\0';
            *len = size - 1;
        }
    }
}

char* bufferAllocHandler(char* buffer, line_t size) {
    char* tmp = (char*)realloc(buffer, sizeof(char) * size);
    if (!tmp) {
        if (!buffer) {
            ON_ERROR("Line buffer allocation failed\n");
        } else {
            ON_ERROR("Line buffer reallocation failed\n");
        }
    }
    return tmp;
}