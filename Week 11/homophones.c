#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mvm.h"

/* FIXME capitalisation of all words and dictionary */
/* FIXME default N value of 3 */
/* FIXME handling number of phenomes */
/* FIXME what other edge cases might there be? */

#define BUFF_SIZE 20
#define BUFF_FACT 4

#define ON_ERROR(STR)     \
    fprintf(stderr, STR); \
    exit(EXIT_FAILURE)
#define STR_END(s) (s == '\0' || s == '\n')

/* TODO check types in all files */
/* FIXME write line ending checker */
typedef enum line_t { LF,
                      CRLF,
                      CR } line_t;

size_t getLine(char** buffer, size_t* size, FILE* file);
FILE* openFile(char* filename);

/*void loadDictionary(mvm* map1, mvm* map2, int n);*/
char* parseWord(char* line);
char* parsePhenome(char* line, size_t len, int n);
char** findRhymes(mvm* map2, char* phenome, int* n);
void loadDictionary(mvm* map1, mvm* map2, int n);
void printRhymes(mvm* map1, mvm* map2, char* word);

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
        *buffer = (char*)malloc(sizeof(char) * BUFF_SIZE);
        if (!*buffer) {
            ON_ERROR("Line buffer allocation failed\n");
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
        /* FIXME this line end check is super dirty at the moment write functino to check line end*/
        /* QUESTION how does C read the newline character? */
        if ((*buffer)[i - 1] == '\n') {
            if ((*buffer)[i - 2] == '\r') {
                (*buffer)[i - 2] = '\0';
                return i - 2;
            }

            (*buffer)[i - 1] = '\0';
            return i - 1;

        } else if (feof(file)) {
            return i;
        }
        /* Check that buffer hasn't been filled. If it has expand, so rest of
           of the line can be read in */
        if (!(i < *size - 1)) {
            *size *= BUFF_FACT;
            tmp = realloc(*buffer, sizeof(char) * *size);
            if (!*tmp) {
                ON_ERROR("Line buffer reallocation failed\n");
            }
            *buffer = tmp;
        }
    }

    /* Returns 0 on eof or error */
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
