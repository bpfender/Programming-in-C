#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mvm.h"

/* FIXME capitalisation of all words and dictionary */
/* FIXME default N value of 3 */
/* FIXME handling number of phenomes */
/* FIXME what other edge cases might there be? */

#define BUFF_SIZE 200
#define BUFF_FACT 4
#define FILENAME "./cmudict.txt"

#define STR_END(s) (s == '\0' || s == '\n')

/* TODO check types in all files */
/* FIXME write line ending checker */
typedef enum line_t { LF,
                      CRLF,
                      CR } line_t;

size_t getLine(char** buffer, size_t* size, FILE* file);
char* bufferAllocHandler(char* buffer, size_t size);
FILE* openFile(char* filename);

void loadDictionary(mvm* map1, mvm* map2, int n);
char* parseWord(char* line);
char* parsePhenome(char* line, size_t len, int n);
char** findRhymes(mvm* map2, char* phenome, int* n);
void loadDictionary(mvm* map1, mvm* map2, int n);
void printRhymes(mvm* map1, mvm* map2, char* word);
void truncateLineEnd(char* buffer, size_t* len);

void test(void);

int main(void) {
    test();
    return 0;
}

/* FIXME is it bad to just operate directly on the buffer line */
void loadDictionary(mvm* map1, mvm* map2, int n) {
    FILE* file = openFile(FILENAME);

    char* buffer = NULL;
    size_t size;
    size_t len;

    char *word, *phenome;

    while ((len = getLine(&buffer, &size, file))) {
        truncateLineEnd(buffer, &len);
        word = parseWord(buffer);
        phenome = parsePhenome(buffer, len, n);
        mvm_insert(map1, word, phenome);
        mvm_insert(map2, phenome, word);
    }

    free(buffer);
    fclose(file);
}

void printRhymes(mvm* map1, mvm* map2, char* word) {
    int n = 0;
    int i;
    char* phenome = mvm_search(map1, word);
    char** rhymes = findRhymes(map2, phenome, &n);

    printf("%s (%s): ", word, phenome);
    for (i = 0; i < n; i++) {
        printf("%s ", rhymes[i]);
    }
    printf("\n");
    free(rhymes);
}

/* Just a wrapper function for mvm_multisearch
 */
char** findRhymes(mvm* map2, char* phenome, int* n) {
    char** rhymes = mvm_multisearch(map2, phenome, n);
    return rhymes;
}

/* FIXME why allocate new memory. However could be written slightly more clearly*/
/* FIXME error handlign */

/* Simply modifies buffer string to avoid copying things around unecesarrily.
 * Replaces '#' in dictionary format with '\0' so that string functions will 
 * work on first half of the buffer properly.
 */
char* parseWord(char* line) {
    int i;
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
/* FIXME needs handling of longer n specifier than phenomes, this is horrible at the moment */
char* parsePhenome(char* line, size_t len, int n) {
    size_t i;
    int count = 0;

    for (i = len - 1; i != 0; i--) {
        if (line[i] == ' ') {
            count++;
        }
        if (count == n || line[i] == '\0') {
            return line + i + 1;
        }
    }

    return NULL;
}

FILE* openFile(char* filename) {
    FILE* file = fopen(filename, "r");

    if (!file) {
        ON_ERROR("Cannot open dictionary file\n");
    }
    return file;
}

/* Handles malloc and reallocing buffer. On the initial call buffer should be 
 * passed as NULL
 */
char* bufferAllocHandler(char* buffer, size_t size) {
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

/* Truncates line endings from lines. Can handle LF or CRLF. Uses short-circuit
 * evaluation to avoid reading outside array indices by checking for zero-length
 * strings and relevant line ending.
 */
void truncateLineEnd(char* buffer, size_t* len) {
    size_t end = *len;

    if (end && buffer[end - 1] == '\n') {
        if (end >= 2 && buffer[end - 2] == '\r') {
            buffer[end - 2] = '\0';
            *len = end - 2;
        } else {
            buffer[end - 1] = '\0';
            *len = end - 1;
        }
    }
}

/* Reads line from a file. Function returns number of characters in the string
 * including \n or -1 on file end or error. Must be passed initialised buffer or
 * NULL value ptr
 */
size_t getLine(char** buffer, size_t* size, FILE* file) {
    size_t i = 0;
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
        filled and needs ot be expanded */
        *size *= BUFF_FACT;
        *buffer = bufferAllocHandler(*buffer, *size);
    }
    /* Returns 0 on eof or error. This should be the only case where 0 can be
    returned as newlines will always include at least the line ending chars */
    return 0;
}

void test(void) {
    char* buffer = NULL;
    size_t buffer_size;
    size_t line_len;
    FILE* file;

    mvm* map1 = mvm_init();
    mvm* map2 = mvm_init();

    printf("Test start...\n");

    file = openFile("cmudict.txt");
    line_len = getLine(&buffer, &buffer_size, file);

    truncateLineEnd(buffer, &line_len);
    printf("%s\n", buffer);
    assert(strcmp(buffer, "STANO#S T AA1 N OW0") == 0);

    assert(strcmp(parseWord(buffer), "STANO") == 0);
    assert(strcmp(parsePhenome(buffer, line_len, 3), "AA1 N OW0") == 0);

    fclose(file);
    free(buffer);

    loadDictionary(map1, map2, 1);
    printf("%s\n", mvm_search(map1, "BOY"));

    printRhymes(map1, map2, "BOY");

    mvm_free(&map1);
    mvm_free(&map2);

    printf("Test End\n");
}
