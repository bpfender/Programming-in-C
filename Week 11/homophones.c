#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mvm.h"

/* FIXME size_t types? */
/* FIXME capitalisation of all words and dictionary */
/* FIXME default N value of 3 */
/* FIXME handling number of phenomes */
/* FIXME what other edge cases might there be? */

#define DICTIONARY "./cmudict.txt"
/* FIXME CHECK THAT THESE ARE SENSIBLE VALUES */
/* Constants for getLine() function buffer */
#define BUFF_SIZE 2
#define BUFF_FACT 4

/* ------ LOADING AND SEARCHING FUNCTIONS ------ */
void loadDictionary(mvm* map1, mvm* map2, int n);
void printRhymes(mvm* map1, mvm* map2, char* word);

/* ------ LINE HANDLING/PARSING FUNCTIONS ------ */
void truncateLineEnd(char* buffer, size_t* len);
char* parseWord(char* line, size_t len);
char* parsePhenome(char* line, size_t len, int n);

/* ------ FILE HANDLING FUNCTIONS ------ */
FILE* openFile(char* filename);
size_t getLine(char** buffer, size_t* size, FILE* file);
char* bufferAllocHandler(char* buffer, size_t size);

void test(void);

int main(int argc, char* argv[]) {
    int n, i;
    mvm *map1, *map2;

    test();

    if (argc > 3 && !strcmp(argv[1], "-n")) {
        n = atoi(argv[2]);
        i = 3;
    } else if (argc > 1) {
        n = 3;
        i = 1;
    } else {
        printf("Incorrect usage...\n");
        return 1;
    }

    map1 = mvm_init();
    map2 = mvm_init();
    loadDictionary(map1, map2, n);

    for (; i < argc; i++) {
        printRhymes(map1, map2, argv[i]);
    }

    mvm_free(&map1);
    mvm_free(&map2);

    return 0;
}

/* ------ LOADING AND SEARCHING FUNCTIONS ------ */
/* Loads dictionary file into map1 and map2. Map1 uses the word as the key and
 * and the phenome as the data, map2 is the other way round. "n" specifies the
 * number of phenomes to be stored. Requires initiliased mvm types.
 */
void loadDictionary(mvm* map1, mvm* map2, int n) {
    FILE* file = openFile(DICTIONARY);
    char* buffer = NULL;
    size_t size;
    size_t len;
    char *word, *phenome;

    /* While returns raw line. Line endings are removed (not strictly necesarry
    but could save some headaches for extensibility e.g. searching for user-
    specified phenome) and the word and phenome sub-strings identified. These 
    are then added to map1 and map2.*/
    while ((len = getLine(&buffer, &size, file))) {
        truncateLineEnd(buffer, &len);

        word = parseWord(buffer, len);
        phenome = parsePhenome(buffer, len, n);
        if (!(word && phenome)) {
            printf("Dictionary format error: line %d", map1->numkeys + 1);
            exit(1);
        }

        mvm_insert(map1, word, phenome);
        mvm_insert(map2, phenome, word);
    }

    if (!feof(file)) {
        ON_ERROR("Error reading dictionary file\n");
    }

    free(buffer);
    fclose(file);
}

void printRhymes(mvm* map1, mvm* map2, char* word) {
    int n, i;
    char* phenome;
    char** rhymes;

    phenome = mvm_search(map1, word);
    if (!phenome) {
        printf("%s not found in dictionary\n", word);
        return;
    }

    rhymes = mvm_multisearch(map2, phenome, &n);

    printf("%s (%s): ", word, phenome);
    for (i = 0; i < n; i++) {
        printf("%s ", rhymes[i]);
    }
    printf("\n");

    free(rhymes);
}

/* ------ LINE HANDLING/PARSING FUNCTIONS ------ */
/* Truncates line endings from lines. Can handle LF or CRLF. Uses short-circuit
 * evaluation to avoid reading outside array indices by checking for zero-length
 * strings and ending type. Does nothing to string if line ends aren't
 * found.
 */
void truncateLineEnd(char* buffer, size_t* len) {
    size_t size = *len;

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

/* Modifies buffer in place to avoid copying things around unecesarrily.
 * Replaces '#' in dictionary format string with '\0' so that string functions
 * will work on first half of the buffer properly.
 */
char* parseWord(char* line, size_t len) {
    size_t i;
    for (i = 0; i < len; i++) {
        if (line[i] == '#') {
            line[i] = '\0';
            return line;
        }
    }
    return NULL;
}

/* Reads from buffer in place. Counts back from end of string to "n" desired
 * phenomes. If n > than the number of phenomes, all the phenomes of the word
 * read into the dictionary. Returns pointer to beginning of phenome string 
 * stored in the line buffer.
 */
char* parsePhenome(char* line, size_t len, int n) {
    size_t i;
    int count = 0;

    for (i = len - 1; i != 0; i--) {
        if (line[i] == ' ') {
            count++;
        }
        /* Extra condition for '\0' and '#' allows this to work whether it's
        called before or after parseWord() */
        if (count == n || line[i] == '\0' || line[i] == '#') {
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
 * passed as NULL. For resizing, the existing pointer should be passed.
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

/* Reads line from a file. Function returns number of characters in the string
 * including \n or 0 on file end or error. Must be passed initialised buffer or
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
        filled and needs to be expanded */
        *size *= BUFF_FACT;
        *buffer = bufferAllocHandler(*buffer, *size);
    }
    /* Returns 0 on eof or error. This should be the only case where 0 can be
    returned as newlines will always include at least the line ending chars */
    return 0;
}

void test(void) {
    /*char* buffer = NULL;
    size_t buffer_size;
    size_t line_len;
    FILE* file;*/

    char str[50];
    size_t str_len;

    /*mvm* map1 = mvm_init();
    mvm* map2 = mvm_init();
*/
    printf("Test start...\n");

    /* ------ LINE HANDLING/PARSING FUNCTIONS TESTING ------ */
    /* Truncate on \r\n */
    strcpy(str, "This is a testing line\r\n");
    str_len = strlen(str);

    truncateLineEnd(str, &str_len);
    assert(strlen(str) == str_len);
    assert(str[str_len - 1] == 'e');
    /* Second call shouldn't do anything */
    truncateLineEnd(str, &str_len);
    assert(str[str_len - 1] == 'e');
    assert(strcmp(str, "This is a testing line") == 0);

    /* Truncate on \n */
    strcpy(str, "This is a testing line\n");
    str_len = strlen(str);

    truncateLineEnd(str, &str_len);
    assert(strlen(str) == str_len);
    assert(str[str_len - 1] == 'e');
    /* Second call shouldn't do anything */
    truncateLineEnd(str, &str_len);
    assert(str[str_len - 1] == 'e');
    assert(strcmp(str, "This is a testing line") == 0);

    /* Additional truncation on 0 length strings */
    strcpy(str, "\r\n");
    str_len = strlen(str);
    truncateLineEnd(str, &str_len);
    assert(strcmp(str, "") == 0);

    strcpy(str, "\n");
    str_len = strlen(str);
    truncateLineEnd(str, &str_len);
    assert(strcmp(str, "") == 0);

    /* Phenome and word parsing */
    strcpy(str, "STANO#S T AA1 N OW0");
    str_len = strlen(str);

    /* parsePhenome call before parseWord call */
    assert(strcmp(parsePhenome(str, str_len, 100), "S T AA1 N OW0") == 0);
    assert(strcmp(parsePhenome(str, str_len, 1), "OW0") == 0);

    /* parseWord call and checking modification of str */
    assert(strcmp(parseWord(str, str_len), "STANO") == 0);
    assert(str[5] == '\0');

    /* parsePhenome call after parseWord call */
    assert(strcmp(parsePhenome(str, str_len, 3), "AA1 N OW0") == 0);
    assert(strcmp(parsePhenome(str, str_len, 100), "S T AA1 N OW0") == 0);

    strcpy(str, "This is not a dictionary string");
    str_len = strlen(str);

    assert(parseWord(str, str_len) == NULL);
    assert(parsePhenome(str, str_len, 50) == NULL);
    /* Check that string remains unchanged */
    assert(strcmp(str, "This is not a dictionary string") == 0);

    /* ------ FILE HANDLING FUNCTIONS ------ */
    /* file = openFile("cmudict.txt");
    line_len = getLine(&buffer, &buffer_size, file);

    truncateLineEnd(buffer, &line_len);
    printf("%s\n", buffer);
    assert(strcmp(buffer, "STANO#S T AA1 N OW0") == 0);

    assert(strcmp(parseWord(buffer, line_len), "STANO") == 0);
    assert(strcmp(parsePhenome(buffer, line_len, 3), "AA1 N OW0") == 0);

    fclose(file);
    free(buffer);

    loadDictionary(map1, map2, 7);
    printf("%s\n", mvm_search(map1, "BOY"));

    printRhymes(map1, map2, "BLAHBLAH");

    mvm_free(&map1);
    mvm_free(&map2);
    */
    printf("Test End\n");
}
