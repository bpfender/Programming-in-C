#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ALPHABET 26
#define BUFFER_SIZE 10

/* TODO malloc of word? */
typedef struct node {
    char* str;
    struct node* next;
    struct node* prev;
} node;

int isAnagram(char* str1, char* str2);
node* createNode(node* prev, char* str, int size);
node* loadDictionary(char* filename);
void pruneNode(node* ptr);

void traverseLinkedList(node* ptr);
void destroyLinkedList(node* ptr);
int getLine(char** buffer, int* size, FILE* file);

void test(void);
int words = 0;

int main(void) {
    test();
    return 0;
}

void selfAnagram(node* ptr, char* str) {
    if (ptr->prev == ptr) {
        printf("%s ", str);
    } else {
        if (isAnagram(str, ptr->str)) {
            printf("%s ", ptr->str);
            i++;
            pruneNode(ptr);
        }
    }
    if (ptr->next != NULL) {
        selfAnagram(ptr->next, str);
    }
}

void anagramsLinkedList(node* ptr, char* str) {
    if (isAnagram(str, ptr->str)) {
        printf("%s ", ptr->str);
    }
    if (ptr->next != NULL) {
        anagramsLinkedList(ptr->next, str);
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

/* TODO is this right? */
void destroyLinkedList(node* ptr) {
    free(ptr->str);
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

    assert(isAnagram("aa", "bb") == 0);
    assert(isAnagram("aa", "aa") == 1);
    assert(isAnagram("astringe", "gantries") == 1);

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
    anagramsLinkedList(start, "misconstrual");
    printf("\n");

    printf("%s\n", start->str);
    current = start->next;
    prev = start->prev;
    printf("%s\n", prev->str);

    pruneNode(start->next);
    next = start->next;
    printf("%s\n", next->str);
    printf("\n");

    /*while (start->next != NULL) {
        printf("%d  ", i++);
        printf("words %d ", )
        selfAnagram(start, start->str);
        next = start->next;
        free(start->str);
        free(start);
        start = start->next;
        start->prev = start;
        printf("\n");
        printf("\n");
    }*/

    free(buffer);
    destroyLinkedList(start);
}
