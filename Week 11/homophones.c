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

int getLine(char** buffer, size_t* size, FILE* file);
char* bufferAllocHandler(char* buffer, size_t size);
FILE* openFile(char* filename);

void loadDictionary(mvm* map1, mvm* map2, int n);
char* parseWord(char* line);
char* parsePhenome(char* line, size_t len, int n);
char** findRhymes(mvm* map2, char* phenome, int* n);
void loadDictionary(mvm* map1, mvm* map2, int n);
void printRhymes(mvm* map1, mvm* map2, char* word);

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

    while ((len = getLine(&buffer, &size, file)) != -1) {
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

char** findRhymes(mvm* map2, char* phenome, int* n) {
    char** rhymes = mvm_multisearch(map2, phenome, n);
    return rhymes;
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

/* Reads line from a file. Line endings are replaced with '\0' and function
 * LF and CRLF endings automatically. Function returns number of characters in 
 * the string or -1 on file end or error. Must be passed initialised buffer or
 * NULL value ptr
 */
int getLine(char** buffer, size_t* size, FILE* file) {
    /* FIXME some weird typing issues that need to be fixed */
    unsigned int i = 0;
    long int file_pos = ftell(file);

    /* If buffer has not been initialised, buffer is allocated and size is set */
    if (!*buffer) {
        *size = BUFF_SIZE;
        *buffer = bufferAllocHandler(NULL, *size);
    }

    /* Use fgets to read into buffer. Space remaining in buffer is decremented 
       based on how far into the file has been read. Allows for dyanmic resizing
       of buffer. The while loop handles \n characters. If EOF is reached, while
       loop will exit */
    while (fgets(*buffer + i, *size - i, file)) {
        /* Get number of characters read */
        i = ftell(file) - file_pos;

        /* First time stream reaches the EOF, fgets will end up here. Can return
        number of characters read */
        if (feof(file)) {
            return i;
        }

        /* Check whether last read character was a \n or eof reached*/
        /* FIXME this line end check is super dirty at the moment write functino to check line end*/
        if ((*buffer)[i - 1] == '\n') {
            if ((*buffer)[i - 2] == '\r') {
                (*buffer)[i - 2] = '\0';
                return i - 2;
            }

            (*buffer)[i - 1] = '\0';
            return i - 1;
        }

        /* If fgets stopped reading and last char wasn't \n, buffer was filled
        and needs ot be expanded */
        *size *= BUFF_FACT;
        *buffer = bufferAllocHandler(*buffer, *size);
    }

    /* Returns 0 on eof or error */
    return -1;
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
