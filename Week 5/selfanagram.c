#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ALPHABET 28
#define BUFFER_SIZE 10

typedef struct node {
    int is_word;
    struct node* children[ALPHABET];
} node;

node* loadDictionary(char* filename);
node* createNode(void);
void unloadTrie(node* ptr);
int getLine(char** buffer, int* size, FILE* file);
void test(void);

int main(void) {
    test();
    return 0;
}

node* loadDictionary(char* filename) {
    node* root;
    node* current;
    int i = 0;
    int index;

    char* buffer = NULL;
    int size;

    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Cannot open \"%s\"\n", filename);
        return NULL;
    }

    root = createNode();

    while (getLine(&buffer, &size, file)) {
        current = root;

        while (!(buffer[i] == '\n' || buffer[i] == '\0')) {
            index = tolower(buffer[i]) - 'a';
            if (current->children[index] == NULL) {
                current->children[index] = createNode();
            }
            current = current->children[index];
            i++;
        }
        current->is_word = 1;
    }
    free(buffer);
    fclose(file);
    return root;
}

node* createNode(void) {
    int i;
    node* ptr = (node*)malloc(sizeof(node));
    if (ptr == NULL) {
        fprintf(stderr, "Error building trie\n");
        exit(EXIT_FAILURE);
    }

    ptr->is_word = 0;
    for (i = 0; i < ALPHABET; i++) {
        ptr->children[i] = NULL;
    }

    return ptr;
}

int getLine(char** buffer, int* size, FILE* file) {
    const int factor = 2;
    int file_pos = (int)ftell(file);
    int i = 0;

    /* FIXME 20 magic number */
    if (*buffer == NULL) {
        *size = 5;
        *buffer = (char*)malloc(*size * sizeof(char));
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
            return i;
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

void unloadTrie(node* ptr) {
    int i;
    for (i = 0; i < ALPHABET; i++) {
        if (ptr->children[i]) {
            unloadTrie(ptr->children[i]);
        }
    }
    free(ptr);
}

void test(void) {
    node* root;
    root = loadDictionary("./eng_370k_shuffle.txt");
    unloadTrie(root);
}