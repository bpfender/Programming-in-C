#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "./file_handling.c"

/* TODO this is currently horrible. Might need to be rewritten with proper range
 * as currently it includes characters that aren't going to appear in text e.g.
 * NULL
 */
#define ASCII_SIZE 127

void countFileChars(int counts[], char* filename);
void frequencyCount(int counts[], char* s);

void test(void);

int main(void) {
    test();
    return 0;
}

/* Count all characters in file */
void countFileChars(int counts[], char* filename) {
    FILE* file = openFile(filename);

    char* buffer = NULL;
    size_t size;

    while (getLine(&buffer, &size, file)) {
        frequencyCount(counts, buffer);
    }
    /* Error checking on readline 0 output */
    if (!feof(file)) {
        fprintf(stderr, "File reading error\n");
        exit(EXIT_FAILURE);
    }

    free(buffer);
    fclose(file);
}

/* Increments character count array */
/* TODO currently no invalid char checking */
void frequencyCount(int counts[], char* s) {
    size_t i = 0;

    for (i = 0; s[i] != '\0'; i++) {
        counts[(int)s[i]]++;
    }
}

void test(void) {
    size_t i;
    int counts[ASCII_SIZE] = {0};

    frequencyCount(counts, "aaa");
    assert(counts['a'] == 3);

    frequencyCount(counts, "Hello you are a cat");
    assert(counts['a'] == 6);
    assert(counts['o'] == 2);
    assert(counts['l'] == 2);

    for (i = 0; i < ASCII_SIZE; i++) {
        counts[i] = 0;
    }

    countFileCharacters(counts, "./test.txt");
    assert(counts['H'] == 2);
    assert(counts['B'] == 1);
    assert(counts['h'] == 4);
    assert(counts['o'] == 5);
    assert(counts['\n'] == 4);
}
