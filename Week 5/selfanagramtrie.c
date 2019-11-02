#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ALPHABET 26
#define BUFFER_SIZE 10

typedef struct trie_node {
    struct list_node* start;
    struct list_node* current;
    struct trie_node* children[ALPHABET];
} trie_node;

typedef struct list_node {
    char* str;
    struct list_node* next;
} list_node;

list_node* createNode(char* str, int size);
trie_node* loadDictionary(char* filename);
void charCounts(int counts[ALPHABET], char* str);
int compareCounts(int count1[ALPHABET], int count2[ALPHABET]);

int getLine(char** buffer, int* size, FILE* file);

void unloadLinkedList(list_node* ptr);
void traverseLinkedList(list_node* head);
void traverseTrie(trie_node* root);

void test(void);
int words = 0;

int main(void) {
    test();
    return 0;
}

list_node* createNode(char* str, int size) {
    list_node* ptr = (list_node*)malloc(sizeof(list_node));
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
    return ptr;
}

trie_node* createTrieNode(void) {
    int i;
    trie_node* ptr = (trie_node*)malloc(sizeof(trie_node));
    if (ptr == NULL) {
        fprintf(stderr, "Error building trie\n");
        exit(EXIT_FAILURE);
    }

    ptr->current = NULL;
    ptr->start = NULL;
    for (i = 0; i < ALPHABET; i++) {
        ptr->children[i] = NULL;
    }

    return ptr;
}

void traverseTrie(trie_node* root) {
    int i;
    if (root->start) {
        traverseLinkedList(root->start);
    }
    for (i = 0; i < ALPHABET; i++) {
        if (root->children[i]) {
            traverseTrie(root->children[i]);
        }
    }
}

void traverseLinkedList(list_node* head) {
    printf("%s ", head->str);
    if (head->next != NULL) {
        traverseLinkedList(head->next);
    } else {
        printf("\n\n");
    }
}

trie_node* loadDictionary(char* filename) {
    trie_node* root = NULL;
    trie_node* curr = NULL;
    int word_len;
    int counts[ALPHABET] = {0};
    int i;

    char* line_buff = NULL;
    int buff_len;

    FILE* file;
    file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Cannot open \"%s\"\n", filename);
        return 0;
    }

    root = createTrieNode();
    printf("Loading dictionary...\n");

    while ((word_len = getLine(&line_buff, &buff_len, file))) {
        curr = root;

        charCounts(counts, line_buff);

        /* Index to hash entry in trie for anagram */
        for (i = 0; i < ALPHABET; i++) {
            while (counts[i] > 0) {
                if (curr->children[i] == NULL) {
                    curr->children[i] = createTrieNode();
                }
                curr = curr->children[i];
                counts[i]--;
            }
        }
        /* QUESTION is it possible to make tries space efficient? */
        /* Add word to trie location at end of linked list */
        if (curr->start == NULL) {
            curr->start = curr->current = createNode(line_buff, word_len);
        } else {
            curr->current->next = createNode(line_buff, word_len);
            curr->current = curr->current->next;
        }
    }

    free(line_buff);
    fclose(file);

    return root;
}

void charCounts(int counts[ALPHABET], char* str) {
    int i;
    for (i = 0; str[i] != '\0'; i++) {
        counts[tolower(str[i]) - 'a']++;
    }
}

void unloadLinkedList(list_node* ptr) {
    free(ptr->str);
    if (ptr->next != NULL) {
        unloadLinkedList(ptr->next);
    }
    free(ptr);
}

void unloadTrie(trie_node* ptr) {
    int i;
    if (ptr->start) {
        unloadLinkedList(ptr->start);
    }
    for (i = 0; i < ALPHABET; i++) {
        if (ptr->children[i]) {
            unloadTrie(ptr->children[i]);
        }
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
    char* filename = "./Text Files/test_lines.txt";
    trie_node* start;

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

    fclose(test_file);

    start = loadDictionary("./Text Files/eng_370k_shuffle.txt");
    printf("\n");
    traverseTrie(start);
    unloadTrie(start);

    free(buffer);
}
