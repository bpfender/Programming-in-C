#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define QUEUE_SIZE 500
#define BUFF_FACTOR 4
#define SIZE 3

typedef enum bool { false = 0,
                    true = 1 } bool;

typedef struct node_t {
    int grid[SIZE][SIZE];
    int f, g, h;
    int x, y;
    struct node_t* parent;
} node_t;

typedef struct queue_t {
    node_t** node;
    size_t elem;
    size_t front;
    size_t back;
    size_t size;
} queue_t;

typedef struct tree_t {
    node_t* node;
    struct tree_t* children[SIZE * SIZE];
} tree_t;

/* ------ PRIORITY QUEUE FUNCTIONS ------ */
void initPQueue(queue_t* p_queue, node_t* node);
void insertPQueue(queue_t* p_queue, node_t* node);
node_t* getMin(queue_t* p_queue);
void delMin(queue_t* p_queue);
bool isEmpty(queue_t* p_queue);
void unloadPQueue(queue_t* p_queue);

void expandPQueue(queue_t* p_queue);
void percolateUp(queue_t* p_queue);
void percolateDown(queue_t* p_queue);
size_t minChildIndex(queue_t* p_queue, size_t i);
void swapNode(node_t* n1, node_t* n2);
void unloadPQueue(queue_t* p_queue);

/* ------ SEARCH TREE AND VISITED FUNCTIONS ------ */
tree_t* initSearchTree();
tree_t* createTreeNode(void);
void insertSearchTree(tree_t* tree, node_t* node);
bool searchTree();
void unloadSearchTree(tree_t* tree);

/* ------ PRIORITY FUNCTION CALCULATIONS ------ */
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
/* Initiliases priority queue to array of size QUEUE_SIZE. Adds starting grid to
 * queue
 */
void initPQueue(queue_t* p_queue, node_t* node) {
    /* +1 for permanent sentinel value */
    p_queue->node = (node_t**)malloc((QUEUE_SIZE + 1) * sizeof(node_t*));
    if (p_queue->node == NULL) {
        fprintf(stderr, "Memory allocation error\n");
        exit(EXIT_FAILURE);
    }

    /* Set sentinel node value and add starting grid*/
    p_queue->node[0] = NULL;
    p_queue->node[1] = node;

    p_queue->size = QUEUE_SIZE;
    p_queue->elem = 1;
    p_queue->back = 1;
}

void unloadPQueue(queue_t* p_queue) {
    free(p_queue->node);
}

void insertPQueue(queue_t* p_queue, node_t* node) {
    if (p_queue->elem == p_queue->size) {
        expandPQueue(p_queue);
    }

    p_queue->elem++;
    p_queue->back++;

    p_queue->node[p_queue->back] = node;
    percolateUp(p_queue);
}

void percolateUp(queue_t* p_queue) {
    node_t *child, *parent;
    size_t i = p_queue->back;

    while (i / 2 != 0) {
        child = p_queue->node[i];
        parent = p_queue->node[i / 2];
        if (child->f < parent->f) {
            swapNode(child, parent);
        }
        i /= 2;
    }
}

void delMin(queue_t* p_queue) {
    p_queue->elem--;
    p_queue->back--;

    p_queue->node[1] = p_queue->node[p_queue->back];
    percolateDown(p_queue);
}

void percolateDown(queue_t* p_queue) {
    node_t *child, *parent;
    size_t i = 1;

    while (i * 2 <= p_queue->elem) {
        parent = p_queue->node[i];

        i = minChildIndex(p_queue, i);
        child = p_queue->node[i];

        if (child->f < parent->f) {
            swapNode(child, parent);
        }
    }
}

size_t minChildIndex(queue_t* p_queue, size_t i) {
    int f1, f2;
    size_t child_index = i * 2;

    /* If final node only has one branch, can only return this */
    if (child_index == p_queue->back) {
        return child_index;
    }

    /* Get f-value of each child */
    f1 = p_queue->node[child_index]->f;
    f2 = p_queue->node[child_index + 1]->f;

    if (f1 < f2) {
        return child_index;
    } else {
        return child_index + 1;
    }
}

void expandPQueue(queue_t* p_queue) {
    p_queue->size = p_queue->size * BUFF_FACTOR + 1;

    p_queue->node = realloc(p_queue->node, p_queue->size * sizeof(node_t*));
    if (p_queue->node == NULL) {
        fprintf(stderr, "Memory allocation error\n");
        exit(EXIT_FAILURE);
    }
}

node_t* getMin(queue_t* p_queue) {
    return p_queue->node[1];
}

bool isEmpty(queue_t* p_queue) {
    if (p_queue->elem == 0) {
        return true;
    }
    return false;
}

/* ------ SEARCH TREE and VISITED ------ */
tree_t* initSearchTree() {
    return createTreeNode();
}

tree_t* createTreeNode(void) {
    size_t i;
    tree_t* ptr = (tree_t*)malloc(sizeof(tree_t));
    if (ptr == NULL) {
        fprintf(stderr, "ERROR\n");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < SIZE * SIZE; i++) {
        ptr->children[i] = NULL;
    }
    /* Doesn't set ptr to queue node until very end externally */
    ptr->node = NULL;
    return ptr;
}

void insertSearchTree(tree_t* tree, node_t* node) {
    int i, j;
    int leaf;
    tree_t* tmp;
    tree_t* tree_node = tree;

    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            leaf = node->grid[i][j];
            if (tree_node->children[leaf] == NULL) {
                tmp = createTreeNode();
                tree_node->children[leaf] = tmp;
                tree_node = tmp;
            } else {
                tree_node = tree_node->children[leaf];
            }
        }
    }
    /* Insert pointer to node at end of branches */
    tree_node->node = node;
}

bool searchTree(tree_t* tree, int grid[SIZE][SIZE]) {
    int i, j;
    int leaf;
    tree_t* tree_node = tree;

    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            leaf = grid[i][j];
            if (tree_node->children[leaf] == NULL) {
                return false;
            }
            tree_node = tree_node->children[leaf];
        }
    }
    return true;
}

/*FIXME not totally sure on this */
void unloadSearchTree(tree_t* tree) {
    int i;
    for (i = 0; i < SIZE * SIZE; i++) {
        if (tree->children[i]) {
            unloadSearchTree(tree->children[i]);
        }
        if (tree->children[i]->node) {
            free(tree->children[i]->node);
        }
    }
    free(tree);
}

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