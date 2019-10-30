#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ALPHABET 26

/* TODO malloc of word? */
typedef struct node {
    char* str;
    struct node* next;
} node;

int isAnagram(char* str1, char* str2);
void test(void);

int main(void) {
    test();
    return 0;
}

node* createNode(char* str) {
    node* ptr = (node*)malloc(sizeof(node));
    if (ptr == NULL) {
        fprintf(stderr, "Node allocation failed\n");
        exit(EXIT_FAILURE);
    }

    strcpy(ptr->str, str);
    ptr->next = NULL;
    return ptr;
}

/* TODO is this right? */
void destroyLinkedList(node* ptr) {
    if (ptr->next != NULL) {
        destroyLinkedList(ptr->next);
    }
    free(ptr);
}

int isAnagram(char* str1, char* str2) {
    int i;
    int count[ALPHABET] = {0};

    /* What about invalid inputs non alphabetical chars? */
    for (i = 0; str1[i] != '\0'; i++) {
        if (isalpha(str1[i])) {
            count[tolower(str1[i]) - 'a']++;
        }
    }

    for (i = 0; str2[i] != '\0'; i++) {
        if (isalpha(str2[i])) {
            count[tolower(str2[i]) - 'a']--;
        }
    }

    for (i = 0; i < ALPHABET; i++) {
        if (count[i]) {
            return 0;
        }
    }
    return 1;
}

/* Must be passed inititialised buffer TODO expand to create buffer */
int getLine(char** buffer, int* size, FILE* file) {
    const int factor = 2;
    long int file_pos = ftell(file);
    long int i = 0;

    /*TODO rewrite this with fgets and buffered inputs */
    while (fgets(*buffer + i, *size, file)) {
        i = ftell(file) - file_pos;

        if ((*buffer)[i - 1] == '\n') {
            return i;
        }
        if (!(i < *size - 1)) {
            *size *= factor;
            /* TODO, can this be nicer? */
            *buffer = (char*)realloc(*buffer, *size * sizeof(char));
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
    FILE* test_file;
    char* filename = "test_lines.txt";

    int size = 2;
    char* buffer = (char*)malloc(size * sizeof(char));
    if (buffer == NULL) {
        fprintf(stderr, "ERROR\n");
        exit(EXIT_FAILURE);
    }

    assert(isAnagram("aa", "bb") == 0);
    assert(isAnagram("aa", "aa") == 1);
    assert(isAnagram("astringe", "gantries") == 1);

    test_file = fopen(filename, "r");
    if (test_file == NULL) {
        fprintf(stderr, "Cannot open \"%s\"\n", filename);
        exit(EXIT_FAILURE);
    }

    printf("%d %s\n", getLine(&buffer, &size, test_file), buffer);
    printf("%d %s\n", getLine(&buffer, &size, test_file), buffer);
    printf("%d %s\n", getLine(&buffer, &size, test_file), buffer);
    printf("%d %s\n", getLine(&buffer, &size, test_file), buffer);

    fclose(test_file);
}
