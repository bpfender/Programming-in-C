#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define QUEUE_SIZE 500
#define BUFF_FACTOR 4
#define SIZE 3

typedef struct node_t {
    int grid[SIZE][SIZE];
    int f, g, h;
    size_t x, y;
    struct node_t* parent;
} node_t;

typedef struct queue_t {
    node_t** list;
    size_t elem;
    size_t front;
    size_t back;
    size_t size;
} queue_t;

typedef struct list_t {
    node_t** node;
    struct list_t* next;
} list_t;

typedef struct bin_tree_t {
    unsigned long key;
    node_t* node;
    struct bin_tree_t* children[2];
} bin_tree_t;

int fPriority(int grid[SIZE][SIZE], size_t step);
int manhattanDistance(int grid[SIZE][SIZE]);
int hammingDistance(int grid[SIZE][SIZE]);

void test(void);

int main(void) {
    test();
    return 0;
}

void swapNode(node_t* n1, node_t* n2) {
    node_t tmp = *n1;
    *n1 = *n2;
    *n2 = tmp;
}

/* ------- PRIORITY QUEUE FUNCTIONS ------- */
void initPQueue(queue_t* p_queue) {
    p_queue->list = (queue_t**)malloc(QUEUE_SIZE * sizeof(queue_t*));
    if (p_queue->list == NULL) {
        fprintf(stderr, "ERROR\n");
        exit(EXIT_FAILURE);
    }

    p_queue->size = QUEUE_SIZE;
    p_queue->elem = 0;
    p_queue->front = 0;
    p_queue->back = 0;
}

void insertPQueue(queue_t* queue, node_t* node) {
    if (queue->elem == queue->size) {
        expandPQueue(queue);
    }

    queue->list[queue->back % queue->size] = node;
    queue->elem++;
    percolateUp(queue);

    queue->back = (queue->back + 1) % queue->size;
}

void delMin(queue_t* queue) {
    queue->list[queue->front] = queue->list[queue->back];
    queue->elem--;

    percolateDown(queue);

    if (queue->back == 0) {
        queue->back = queue->elem - 1;
    } else {
        queue->back--;
    }
}

void percolateUp(queue_t* queue) {
    size_t i = queue->elem;
    size_t front = queue->front;
    size_t len = queue->size;
    node_t *child, *parent;

    /* FIXME, THIS DOESN't return when done swapping yet */
    while (i / 2 != 0) {
        child = queue->list[(front + i) % len];
        parent = queue->list[(front + i / 2) % len];
        if (child->f < parent->f) {
            swapNode(child, parent);
        }
        i /= 2;
    }
}

void percolateDown(queue_t* queue) {
    size_t i = 0;
    size_t front = queue->front;
    size_t len = queue->elem;
    node_t *child, *parent;
    size_t child_index;

    /*FIXME what about child nodes at end of list? */
    while ((i + 1) * 2 <= queue->elem) {
        child_index = minChildIndex(queue, i);
        parent = queue->list[(front + i) % len];
        child = queue->list[(front + child_index) % len];

        if (child->f < parent->f) {
            swap(child, parent);
        }
        i = child_index;
    }
}

int minChildIndex(queue_t* queue, size_t i) {
    size_t front = queue->front;
    size_t len = queue->size;
    size_t next_index = (i + 1) * 2;

    int f1 = queue->list[(front + next_index) % len];
    int f2 = queue->list[(front + next_index + 1) % len];

    /* FIXME this indexing will likely break */
    if (f1 < f2) {
        return next_index;
    } else {
        return next_index + 1;
    }
}

void expandPQueue(queue_t* queue) {
    size_t i;

    node_t** tmp = (node_t**)malloc(queue->size * BUFF_FACTOR * sizeof(node_t*));
    if (tmp == NULL) {
        fprintf(stderr, "ERROR\n");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < queue->elem; i++) {
        tmp[i] = queue->list[(queue->front + i)];
    }
    free(queue->list);
    queue->list = tmp;
    queue->size *= BUFF_FACTOR;
    queue->front = 0;
    queue->back = queue->elem - 1;
}

node_t* getMin(queue_t* queue) {
    return queue->list[queue->front];
}

void isEmpty();

/* ------- CLOSED LIST ------- */
void initList();
void insertList();

/* ------ SEARCH TREE and CLOSED LIST ------ */
unsigned long keyValue(int grid[SIZE][SIZE]) {
}

void initBinTree();
void insertBinTree();
void searchBinTree();

/* ------- PRIORIITY FUNCTION CALCULATION ------- */
/* FIXME size_t vs int */
int fPriority(int grid[SIZE][SIZE], size_t step) {
    return manhattanDistance(grid) + (int)step;
}

int manhattanDistance(int grid[SIZE][SIZE]) {
    int i, j;
    int manhattan = 0;
    int num;
    printf("MANHATTAN\n");
    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            num = grid[i][j] - 1;
            /* FIXME definition doesn't ignore 0 */
            manhattan += abs(num / 3 - i) + abs(num % 3 - j);
            printf("%d\n", manhattan);
        }
    }

    return manhattan;
}

int hammingDistance(int grid[SIZE][SIZE]) {
    int i, j;
    int hamming = 0;

    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            /* FIXME different definition of hamming distance elsewhere */
            hamming += (grid[i][j] != (int)(3 * i + j + 1) % (SIZE * SIZE));
            printf("%d\n", hamming);
        }
    }
    return hamming;
}

void test(void) {
    int i, j;
    int test_grid[SIZE][SIZE] = {{8, 1, 3}, {4, 0, 2}, {7, 6, 5}};

    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            printf("%d", test_grid[i][j]);
        }
        printf("\n");
    }

    assert(hammingDistance(test_grid) == 6);
    assert(manhattanDistance(test_grid) == 10);
}