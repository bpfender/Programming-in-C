#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct node {
    char* str;
    struct node* next;
} node;

node* loadDictionary(char* filename);
node* createNode(char* str, size_t size);
size_t getLine(char** buffer, size_t* size, FILE* file);
void destroyLinkedList(node* ptr);
int checkPosition(char* s1, char* s2);

void insertSortedWord(node* list, char* str);

void printList(node* list);

void test(void);

int main(void) {
    test();
    return 0;
}

void test(void) {
    node* list;
    list = loadDictionary("../Week 5/Text Files/eng_370k_shuffle.txt");
    printList(list);
}

void insertSortedWord(node* list, char* str) {
    node* tmp;

    if (checkPosition(str, list->str) == 0) {
        tmp = list->next;
        list->next = createNode(list->str, strlen(list->str));

        strcpy(list->str, str);
        list->next->next = tmp;
        return;
    } else if (list->next == NULL) {
        list->next = createNode(str, strlen(str));
        return;
    }
    insertSortedWord(list->next, str);
}

void printList(node* list) {
    printf("%s\n", list->str);
    if (list->next == NULL) {
        return;
    }
    printList(list->next);
}

node* loadDictionary(char* filename) {
    int i = 0;
    FILE* file;
    char* buffer = NULL;
    size_t buffer_size;
    size_t word_len;

    node* start = NULL;
    node* current = NULL;

    file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Cannot open \"%s\"\n", filename);
        return NULL;
    }

    if ((word_len = getLine(&buffer, &buffer_size, file))) {
        start = createNode(buffer, word_len);
    }
    while ((word_len = getLine(&buffer, &buffer_size, file))) {
        printf("%i\n", i++);
        insertSortedWord(start, buffer);
    }

    free(buffer);
    fclose(file);

    return start;
}

node* createNode(char* str, size_t size) {
    node* ptr = (node*)malloc(sizeof(node));
    if (ptr == NULL) {
        fprintf(stderr, "Node allocation failed\n");
        exit(EXIT_FAILURE);
    }

    ptr->str = (char*)malloc((size + 1) * sizeof(char));
    if (ptr->str == NULL) {
        fprintf(stderr, "String allocation failed\n");
        exit(EXIT_FAILURE);
    }

    strcpy(ptr->str, str);
    ptr->next = NULL;

    return ptr;
}

void destroyLinkedList(node* ptr) {
    free(ptr->str);
    if (ptr->next != NULL) {
        destroyLinkedList(ptr->next);
    }
    free(ptr);
}

/* FIXME finalise getline function please */
size_t getLine(char** buffer, size_t* size, FILE* file) {
    const unsigned int factor = 2;
    long int file_pos = ftell(file);
    long int i = 0;

    /* FIXME 20 magic number */
    if (*buffer == NULL) {
        *size = 5;
        *buffer = (char*)malloc(*size * sizeof(char));
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
    return (size_t)i;
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
