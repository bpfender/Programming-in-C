#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ALPHABET 26
#define MAXLEN 40

int isogram(char* s);
int getLine(char** buffer, int* size, FILE* file);

int main(void) {
    char* buffer = NULL;
    int buff_size;
    int word_len;

    char word[MAXLEN];
    int longest = 0;

    FILE* file;

    file = fopen("./eng_370k_shuffle_short.txt", "r");
    if (file == NULL) {
        fprintf(stderr, "Error reading file...\n");
        return 1;
    }

    while ((word_len = getLine(&buffer, &buff_size, file))) {
        if (isogram(buffer)) {
            if (word_len > longest) {
                longest = word_len;
                strcpy(word, buffer);
            }
        }
    }

    printf("%s (%d)\n", word, longest);

    free(buffer);
    fclose(file);

    return 0;
}

int getLine(char** buffer, int* size, FILE* file) {
    const int factor = 2;
    int file_pos = (int)ftell(file);
    int i = 0;

    /* FIXME 20 magic number */
    if (*buffer == NULL) {
        *size = 50;
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

int isogram(char* s) {
    int i;
    int counts[ALPHABET] = {0};

    for (i = 0; s[i] != '\0'; i++) {
        counts[tolower(s[i]) - 'a']++;
    }

    for (i = 0; i < ALPHABET; i++) {
        if (counts[i] > 1) {
            return 0;
        }
    }
    return 1;
}
