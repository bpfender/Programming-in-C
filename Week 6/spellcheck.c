#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_WORDS 5
#define MAX_LEN 40

typedef struct list {
    char sorted[MAX_WORDS][MAX_LEN];
    size_t index;
} list;

int loadDictionary(list* list, char* filename);
void insertWord(list* list, char* word);
void shiftList(list* list, int pos);

int checkPosition(char* s1, char* s2);

int getLine(char** buffer, int* size, FILE* file);

void test(void);

int main(void) {
    int i;
    static list list;
    list.index = 0;

    /*test();*/

    loadDictionary(&list, "../Week 5/Text Files/test_lines.txt");

    for (i = 0; i < MAX_WORDS; i++) {
        printf("Index %d %s\n", i, list.sorted[i]);
    }

    return 0;
}

void insertWord(list* list, char* word) {
    size_t i;
    printf("Index %li\n", list->index);
    if (list->index == 0) {
        strcpy(list->sorted[0], word);
        list->index++;
        return;
    }

    for (i = 0; i < list->index; i++) {
        if (checkPosition(word, list->sorted[i]) == 0) {
            shiftList(list, i);
            strcpy(list->sorted[i], word);
            list->index++;
            return;
        }
    }
    strcpy(list->sorted[i], word);
    list->index++;
}

int loadDictionary(list* list, char* filename) {
    char* buffer = NULL;
    int size;

    FILE* file;
    file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Cannot open \"%s\"\n", filename);
        return 0;
    }

    while (getLine(&buffer, &size, file)) {
        insertWord(list, buffer);
    }

    fclose(file);
    free(buffer);
    return 1;
}

void shiftList(list* list, int pos) {
    int i;
    for (i = list->index; i >= pos; i--) {
        strcpy(list->sorted[i + 1], list->sorted[i]);
    }
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
/* FIXME return value need to be modified */
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
        exit(EXIT_FAILURE);
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
                exit(EXIT_FAILURE);
            }
        }
    }

    if (!feof(file)) {
        fprintf(stderr, "Error reading file\n");
        exit(EXIT_FAILURE);
    }
    return i;
}

void test(void) {
    int i;
    list test_list;

    test_list.index = 1;
    strcpy(test_list.sorted[0], "hello");
    strcpy(test_list.sorted[1], "goodbye");

    assert(checkPosition("apple", "hello") == 0);
    assert(checkPosition("ass", "abba") == 1);
    assert(checkPosition("cat", "decapod") == 0);
    assert(checkPosition("benedict", "george") == 0);
    assert(checkPosition("hello", "alien") == 1);

    shiftList(&test_list, 0);
    test_list.index = 2;
    assert(strcmp("hello", test_list.sorted[1]) == 0);
    assert(strcmp("goodbye", test_list.sorted[2]) == 0);

    insertWord(&test_list, "apple");
    for (i = 0; i < 10; i++) {
        printf("Index %d %s\n", i, test_list.sorted[i]);
    }

    assert(strcmp("apple", test_list.sorted[0]) == 0);
    assert(strcmp("hello", test_list.sorted[1]) == 0);
    assert(strcmp("goodbye", test_list.sorted[3]) == 0);
}