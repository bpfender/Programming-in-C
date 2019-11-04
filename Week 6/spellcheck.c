#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_WORDS 37019

typedef struct list {
    char sorted[MAX_WORDS];
} list;

void test(void);
int checkPosition(char* s1, char* s2);
int getLine(char** buffer, int* size, FILE* file);

int main(void) {
    test();
    return 0;
}

int checkPosition(char* s1, char* s2) {
    int i = 0;
    while (!(s1[i] == '\0' || s2[i] == '\0')) {
        if (tolower(s1[i]) < tolower(s2[i])) {
            return 0;
        }
        if (tolower(s1[i]) > tolower(s2[i])) {
            return 1;
        }

        i++;
    }
    if (s1[i] == '\0' && s2[i] != '\0') {
        return 0;
    }
    return 1;
}

/* MODIFIED TO RETURN NULL CHARACTER TERMINATED LINE \n removed*/
int getLine(char** buffer, int* size, FILE* file) {
    const int factor = 2;
    int file_pos = (int)ftell(file);
    int i = 0;

    /* FIXME 20 magic number */
    if (*buffer == NULL) {
        *size = 5;
        *buffer = (char*)malloc((*size + 1) * sizeof(char));
    }
    if (*buffer == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return -1;
    }

    while (fgets(*buffer + i, *size - i, file)) {
        /* Additional i-1 for index, because file pos will be one higher after reading \n */
        /* TODO some clearer coding around returning i would be preferred */
        i = ftell(file) - file_pos;
        if ((*buffer)[i - 1] == '\n') {
            (*buffer)[i - 1] = '\0';
            return i - 1;
        }

        if (!(i < *size - 1)) {
            *size *= factor;
            /* TODO, can this be nicer? */
            *buffer = realloc(*buffer, *size * sizeof(char));
            if (*buffer == NULL) {
                fprintf(stderr, "Memory allocation failed\n");
                return -1;
            }
        }
    }

    if (!feof(file)) {
        fprintf(stderr, "Error reading file\n");
        return -1;
    }
    return i;
}

void test(void) {
    assert(checkPosition("ass", "abba") == 1);
    assert(checkPosition("cat", "decapod") == 0);
    assert(checkPosition("benedict", "george") == 0);
    assert(checkPosition("hello", "alien") == 1);
}