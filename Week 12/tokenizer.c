#include "tokenizer.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define LINE_SIZE 50
#define FACTOR 4
#define WHITESPACE " \t\n\v\f\r"
#define TERMINATORS "(){},=#\" \t\n\v\f\r"

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

void loadFile(prog_t* program, char* filename) {
    FILE* file = openFile(filename);

    char* buffer = NULL;
    char* pos;
    line_t size;
    line_t line_len, word_len;
    int line = 0, word;

    while ((line_len = getLine(&buffer, &size, file))) {
        truncateLineEnd(buffer, &line_len);

        pos = buffer;
        line++;
        word = 0;

        /* Parse through words in the line */
        while ((word_len = parseBufferWords(&pos))) {
            word++;
            enqueueToken(program, pos, word_len, line, word);
            pos += word_len;
        }
    }

    free(buffer);
    fclose(file);
}

/* FIXME this does not deal with \" chars yet */
line_t parseBufferWords(char** pos) {
    if (*pos[0] == '\0') {
        return 0;
    }
    /* Remove leading whitespace */
    *pos += strspn(*pos, WHITESPACE);

    switch (*pos[0]) {
        case '(':
        case ')':
        case '{':
        case '}':
        case '=':
        case ',':
            return 1;
            break;
        case '"':
        case '#':
            /* FIXME a little bit dirty at the moment */
            return strchr(*pos + 1, *pos[0]) - *pos + 1;
            break;
        default:
            return strcspn(*pos, TERMINATORS);
            break;
    }
}

line_t getLine(char** buffer, line_t* size, FILE* file) {
    line_t i = 0;
    long int file_pos = ftell(file);

    /* If buffer has not been initialised, buffer is allocated and size is set */
    if (!*buffer) {
        *size = LINE_SIZE;
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

        if (LINE_T_MAX / FACTOR <= *size) {
            *size = LINE_T_MAX;
        } else {
            *size *= FACTOR;
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