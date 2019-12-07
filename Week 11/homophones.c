#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mvm.h"

#define BUFF_SIZE 20
#define BUFF_FACT 4

#define STR_END(s) (s == '\0' || s == '\n')

/* TODO check types in all files */

size_t getLine(char** buffer, size_t* size, FILE* file);
FILE* openFile(char* filename);

void loadDictionary(mvm* map1, mvm* map2, int n);
char* parseWord(char* line);
char* parsePhenome(char* line, size_t len, int n);

void test(void);

int main(void) {
    test();
}

/* FIXME is it bad to just operate directly on the buffer line */
void loadDictionary(mvm* map1, mvm* map2, int n) {
    FILE* file = openFile("./cmudict.txt");
    char* buffer = NULL;
    size_t size;
    size_t len;
    char *word, *phenome;

    while ((len = getLine(&buffer, &size, file))) {
        word = parseWord(buffer);
        phenome = parsePhenome(buffer, len, n);
        mvm_insert(map1, word, phenome);
        mvm_insert(map2, phenome, word);
    }

    free(buffer);
    fclose(file);
}

void truncateLine(char* line, size_t len) {
    if (line[len - 1] == '\n') {
        line[len - 1] = '\0';
    }
}

/* FIXME why allocate new memory. However could be written slightly more clearly*/
/* FIXME error handlign */
char* parseWord(char* line) {
    size_t i;
    for (i = 0; line[i] != '#'; i++) {
        /* FIXME how is file terminated? Avoid exceeding buffer */
        if (line[i] == '\0') {
            ON_ERROR("Incorrect dictionary format. Exiting\n");
        }
    }
    line[i] = '\0';
    return line;
}

/* FIXME error handling*/
char* parsePhenome(char* line, size_t len, int n) {
    size_t i;
    int count = 0;

    for (i = len - 1; i != 0; i--) {
        if (line[i] == ' ') {
            count++;
            if (count == n) {
                return line + i + 1;
            }
        }
    }
    return NULL;
}

FILE* openFile(char* filename) {
    FILE* file = fopen(filename, "r");
    /* FIXME can this be written with ON_ERROR? */
    if (file == NULL) {
        fprintf(stderr, "Cannot open \"%s\"\n", filename);
        exit(EXIT_FAILURE);
    }
    return file;
}

/* FIXME modify so that \n is removed */
/* Reads line from a file. '\n' is appended by '\0' character. Must be passed
 * either initialised buffer or NULL pointer. Returns 0 when EOF reached or
 * error in file reading. This should be checked after return from getLine().
 * Returns number of characters in string including \n but not \0
 */
size_t getLine(char** buffer, size_t* size, FILE* file) {
    size_t i = 0;
    long int file_pos = ftell(file);
    char* tmp;

    /* If buffer has not been initialised, buffer is malloced and size is set */
    if (!*buffer) {
        *size = BUFF_SIZE;
        *buffer = (char*)malloc(BUFF_SIZE * sizeof(char));
        if (!*buffer) {
            fprintf(stderr, "Memory allocation failed\n");
            exit(EXIT_FAILURE);
        }
    }

    /* Use fgets to read into buffer. Space remaining in buffer is decremnted 
       based on how far into the file has been read. Allows for dyanmic resizing
       of buffer. The while loop handles \n characters. If EOF is reached, while
       loop will exit */
    while (fgets(*buffer + i, (int)(*size - i), file)) {
        /* Get number of characters read */
        i = ftell(file) - file_pos;

        /* Check whether last read character was a \n or eof reached*/
        if ((*buffer)[i - 1] == '\n' || feof(file)) {
            return i;
        }
        /* Check that buffer hasn't been filled. If it has expand, so rest of
           of the line can be read in */
        if (!(i < *size - 1)) {
            *size *= BUFF_FACT;
            tmp = realloc(*buffer, *size * sizeof(char));
            if (!*tmp) {
                fprintf(stderr, "Memory allocation failed\n");
                exit(EXIT_FAILURE);
            }
            *buffer = tmp;
        }
    }

    /* Returns 0 on eof or error */
    return 0;
}

void test(void) {
    char string[25] = "STANO#S T AA1 N OW0";
    size_t len = strlen(string);
    printf("%s\n", parseWord(string));
    printf("%s\n", parsePhenome(string, len, 3));
}
