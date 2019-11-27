#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./fileio.c"
#include "./pqueue.c"

/* TODO this is currently horrible. Might need to be rewritten with proper range
 * as currently it includes characters that aren't going to appear in text e.g.
 * NULL
 */
#define ASCII_SIZE 127
#define MAX_ENCODE 30

typedef struct data_t {
    char c;
    unsigned long freq;
    char encoding[MAX_ENCODE + 1];
    int bit;
} data_t;

void countFileChars(data_t letters[], char* filename);
void frequencyCount(data_t letters[], char* s);
void getInitialFreqs(queue_t* p_queue, data_t letters[], char* filename);
int cmpFunc(const void* a, const void* b);
void initCntArr(data_t letters[]);
node_t* createNode(char c, int freq);
node_t* buildHuffmanTree(queue_t* p_queue);

void test(void);

int main(void) {
    test();
    return 0;
}

void unloadHuffman(node_t* node) {
    if (node->l) {
        unloadHuffman(node->l);
    }
    if (node->r) {
        unloadHuffman(node->r);
    }
    free(node);
}

void returnHuffmanEncodings(node_t* node, data_t letters[], char* encoding, int bit) {
    bit++;
    if (node->l) {
        encoding[bit] = '0';
        returnHuffmanEncodings(node->l, letters, encoding, bit);
    }
    if (node->r) {
        encoding[bit] = '1';
        returnHuffmanEncodings(node->r, letters, encoding, bit);
    }
    if (!(node->l && node->r)) {
        encoding[bit] = '\0';
        strcpy(letters[(int)node->c].encoding, encoding);

        letters[(int)node->c].freq = node->freq;
        letters[(int)node->c].bit = bit;
    }
}

node_t* buildHuffmanTree(queue_t* p_queue) {
    node_t* parent = getMin(p_queue);
    node_t *child1, *child2;

    while (p_queue->end > 1) {
        child1 = getMin(p_queue);
        delMin(p_queue);
        child2 = getMin(p_queue);
        delMin(p_queue);

        parent = createNode(-1, child1->freq + child2->freq);

        parent->l = child1;
        parent->r = child2;

        insertPQueue(p_queue, parent);
    }
    return parent;
}

void getInitialFreqs(queue_t* p_queue, data_t letters[], char* filename) {
    size_t i;
    node_t* tmp;

    initCntArr(letters);
    initPQueue(p_queue);

    countFileChars(letters, filename);

    for (i = 0; i < ASCII_SIZE; i++) {
        if (letters[i].freq) {
            tmp = createNode(letters[i].c, letters[i].freq);

            insertPQueue(p_queue, tmp);
        }
    }

    /*qsort(letters, ASCII_SIZE, sizeof(data_t), cmpFunc);*/
}

node_t* createNode(char c, int freq) {
    node_t* tmp = (node_t*)malloc(sizeof(node_t));
    if (!tmp) {
        fprintf(stderr, "Memory allocation error\n");
        exit(EXIT_FAILURE);
    }

    tmp->c = c;
    tmp->freq = freq;
    tmp->l = tmp->r = NULL;

    return tmp;
}

int cmpFunc(const void* a, const void* b) {
    return ((data_t*)a)->freq - ((data_t*)b)->freq;
}

/* Count all characters in file */
void countFileChars(data_t letters[], char* filename) {
    FILE* file = openFile(filename);

    char* buffer = NULL;
    size_t size;

    while (getLine(&buffer, &size, file)) {
        frequencyCount(letters, buffer);
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
void frequencyCount(data_t letters[], char* s) {
    size_t i = 0;

    for (i = 0; s[i] != '\0'; i++) {
        letters[(int)s[i]].freq++;
    }
}

void initCntArr(data_t letters[]) {
    size_t i;
    for (i = 0; i < ASCII_SIZE; i++) {
        letters[i].c = i;
        letters[i].freq = 0;
    }
}

void test(void) {
    size_t i;
    data_t letters[ASCII_SIZE];
    queue_t p_queue;
    node_t* huffman = NULL;
    char encoding[MAX_ENCODE + 1];
    size_t bytes = 0;

    initCntArr(letters);

    frequencyCount(letters, "aaa");
    assert(letters['a'].freq == 3);

    frequencyCount(letters, "Hello you are a cat");
    assert(letters['a'].freq == 6);
    assert(letters['o'].freq == 2);
    assert(letters['l'].freq == 2);

    initCntArr(letters);

    countFileChars(letters, "./test.txt");
    assert(letters['H'].freq == 2);
    assert(letters['B'].freq == 1);
    assert(letters['h'].freq == 4);
    assert(letters['o'].freq == 5);
    assert(letters['\n'].freq == 4);

    getInitialFreqs(&p_queue, letters, "./test.txt");
    for (i = 1; i <= p_queue.end; i++) {
        free(p_queue.node[i]);
    }

    unloadPQueue(&p_queue);

    /* for (i = 0; i < ASCII_SIZE; i++) {
        if (letters[i].c == '\n') {
            printf("Nl %li\n", letters[i].freq);
        } else {
            printf("%c %li\n", letters[i].c, letters[i].freq);
        }
    }
    printf("\n\n");

    for (i = 1; p_queue.node[i] != NULL; i++) {
        if (p_queue.node[i]->c == '\n') {
            printf("Nl %li\n", p_queue.node[i]->freq);
        } else {
            printf("%c %li\n", p_queue.node[i]->c, p_queue.node[i]->freq);
        }
    }*/

    getInitialFreqs(&p_queue, letters, "./aliceinwonderland.txt");
    huffman = buildHuffmanTree(&p_queue);
    returnHuffmanEncodings(huffman, letters, encoding, -1);

    for (i = 0; i < ASCII_SIZE; i++) {
        if (letters[i].freq) {
            if (isalnum(letters[i].c) || ispunct(letters[i].c)) {
                printf("%3c", letters[i].c);
            } else {
                printf("%3i", letters[i].c);
            }
            printf(" : %*s (%3d * %5li)\n", MAX_ENCODE + 1, letters[i].encoding, letters[i].bit, letters[i].freq);
            bytes += letters[i].freq * letters[i].bit;
        }
    }
    bytes /= 8;
    printf("%li bytes\n", bytes);

    unloadPQueue(&p_queue);
    unloadHuffman(huffman);
}
