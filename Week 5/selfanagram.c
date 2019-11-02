#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ALPHABET 26
#define BUFFER_SIZE 10

typedef struct node {
    char* str;
    int len;
    int count[ALPHABET];
    struct node* next;
    struct node* prev;
} node;

node* createNode(node* prev, char* str, int size);
node* loadDictionary(char* filename);
void pruneNode(node* ptr);
void charCounts(int counts[ALPHABET], char* str);
int compareCounts(int count1[ALPHABET], int count2[ALPHABET]);

void traverseLinkedList(node* ptr);
void unloadLinkedList(node* ptr);
int getLine(char** buffer, int* size, FILE* file);

void test(void);
int words = 0;

int main(void) {
    test();
    return 0;
}

void selfAnagram(node* ptr, int count[ALPHABET], int size) {
    if (ptr->prev == ptr) {
        printf("%s ", ptr->str);
        words++;
    } else {
        if (size == ptr->len) {
            if (compareCounts(ptr->count, count)) {
                words++;
                printf("%s ", ptr->str);
                pruneNode(ptr);
            }
        }
    }
    if (ptr->next != NULL) {
        selfAnagram(ptr->next, count, size);
    }
}

void pruneNode(node* ptr) {
    node* next = ptr->next;
    node* prev = ptr->prev;

    free(ptr->str);
    free(ptr);

    next->prev = prev;
    prev->next = next;
}

node* createNode(node* prev, char* str, int size) {
    node* ptr = (node*)malloc(sizeof(node));
    if (ptr == NULL) {
        fprintf(stderr, "Node allocation failed\n");
        exit(EXIT_FAILURE);
    }

    ptr->str = (char*)malloc((size + 1) * sizeof(char));
    if (ptr->str == NULL) {
        fprintf(stderr, "ERRROR\n");
    }

    strcpy(ptr->str, str);
    charCounts(ptr->count, str);
    ptr->len = size;

    ptr->next = NULL;
    ptr->prev = prev;
    return ptr;
}

node* loadDictionary(char* filename) {
    node* start = NULL;
    node* curr = NULL;
    int size;
    FILE* file;

    char* line_buff = NULL;
    int buff_size;

    file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Cannot open \"%s\"\n", filename);
        return 0;
    }

    if ((size = getLine(&line_buff, &buff_size, file))) {
        start = curr = createNode(curr, line_buff, size);
        start->prev = start;
    }
    while ((size = getLine(&line_buff, &buff_size, file))) {
        curr->next = createNode(curr, line_buff, size);
        curr = curr->next;
    }

    free(line_buff);
    fclose(file);

    return start;
}

void charCounts(int counts[ALPHABET], char* str) {
    int i;
    for (i = 0; str[i] != '\0'; i++) {
        counts[tolower(str[i]) - 'a']++;
    }
}

int compareCounts(int count1[ALPHABET], int count2[ALPHABET]) {
    int i;
    for (i = 0; i < ALPHABET; i++) {
        if (count1[i] != count2[i]) {
            return 0;
        }
    }
    return 1;
}

/* TODO is this right? */
void unloadLinkedList(node* ptr) {
    free(ptr->str);
    if (ptr->next != NULL) {
        unloadLinkedList(ptr->next);
    }
    free(ptr);
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
    FILE* test_file;
    char* filename = "test_lines.txt";
    node* start;
    node* next;
    node* prev;
    node* current;
    int i = 0;

    int size = 5;
    char* buffer = (char*)malloc(size * sizeof(char));
    if (buffer == NULL) {
        fprintf(stderr, "ERROR\n");
        exit(EXIT_FAILURE);
    }

    test_file = fopen(filename, "r");
    if (test_file == NULL) {
        fprintf(stderr, "Cannot open \"%s\"\n", filename);
        exit(EXIT_FAILURE);
    }

    assert(getLine(&buffer, &size, test_file) == 10);
    assert(strcmp(buffer, "farandoles") == 0);
    assert(getLine(&buffer, &size, test_file) == 8);
    assert(strcmp(buffer, "bronzine") == 0);
    assert(getLine(&buffer, &size, test_file) == 12);
    assert(strcmp(buffer, "auscultatory") == 0);
    assert(getLine(&buffer, &size, test_file) == 5);
    assert(strcmp(buffer, "bifer") == 0);
    assert(getLine(&buffer, &size, test_file) == 10);
    assert(strcmp(buffer, "steepgrass") == 0);
    assert(getLine(&buffer, &size, test_file) == 0);

    fclose(test_file);

    start = loadDictionary("./eng_370k_shuffle.txt");
    printf("\n");

    printf("%s\n", start->str);
    current = start->next;
    prev = start->prev;
    printf("%s\n", prev->str);

    pruneNode(start->next);
    next = start->next;
    printf("%s\n", next->str);
    printf("\n");

    while (start->next != NULL) {
        printf("%d  ", i++);
        printf("words %d ", words);
        selfAnagram(start, start->count, start->len);
        next = start->next;
        free(start->str);
        free(start);
        start = start->next;
        start->prev = start;
        printf("\n");
        printf("\n");
    }

    free(buffer);
    unloadLinkedList(start);
}
