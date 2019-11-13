#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define QUEUE_SIZE 500
#define BUFF_FACTOR 4
#define SIZE 3
/* http://w01fe.com/blog/2009/01/the-hardest-eight-puzzle-instances-take-31-moves-to-solve/ */
#define MAX_STEPS 32

typedef enum bool { false = 0,
                    true = 1 } bool;

/* Encodes direction that tiles on the board are shifted */
typedef enum swap_t { UP,
                      DOWN,
                      LEFT,
                      RIGHT } swap_t;

typedef struct node_t {
    int grid[SIZE][SIZE];
    int f, step;
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

/* Stores list of solution steps, and variable for number
   of steps */
typedef struct sol_t {
    node_t* node[MAX_STEPS];
    int steps;
} sol_t;

/* ------ SOLVER FUNCTIONS ------ */
void solve8Tile(queue_t* p_queue, tree_t** tree, char* s);
bool expandNode(queue_t* p_queue, tree_t* tree);
bool shiftTile(swap_t dir, queue_t* p_queue, tree_t* tree, node_t* parent);

bool checkTarget(int grid[SIZE][SIZE]);
bool compareBoards(int grid1[SIZE][SIZE], int grid2[SIZE][SIZE]);

/* ------ LOADING FUNCTIONS ------ */
node_t* initNode(char* s);
void loadBoard(int grid[SIZE][SIZE], char* s);
void findFreeTile(node_t* node);

/* ------ PRIORITY QUEUE FUNCTIONS ------ */
void initPQueue(queue_t* p_queue, node_t* node);
void insertPQueue(queue_t* p_queue, node_t* node);
node_t* getMin(queue_t* p_queue);
void delMin(queue_t* p_queue);
bool isEmpty(queue_t* p_queue);

void expandPQueue(queue_t* p_queue);
void percolateUp(queue_t* p_queue);
void percolateDown(queue_t* p_queue);
size_t minChildIndex(queue_t* p_queue, size_t i);
void swapNodePtr(node_t** n1, node_t** n2);
void unloadPQueue(queue_t* p_queue);

/* ------ SEARCH TREE AND VISITED FUNCTIONS ------ */
tree_t* initTree(node_t* node);
void insertTree(tree_t* tree, node_t* node);
bool searchInTree(tree_t* tree, int grid[SIZE][SIZE]);

tree_t* createTreeNode(void);
void unloadTree(tree_t* tree);

/* ------ PRIORITY FUNCTION CALCULATIONS ------ */
int fPriority(int grid[SIZE][SIZE], int g);
int manhattanDistance(int grid[SIZE][SIZE]);
int hammingDistance(int grid[SIZE][SIZE]);

/* ------ UTILITY & INPUT FUNCTIONS ------ */
void loadSolution(queue_t* p_queue, sol_t* solution);
void printSolution(sol_t* solution);
void printBoard(int grid[SIZE][SIZE]);
bool checkInputString(char* s);
bool isSolvable(char* s);
void swap(int* n1, int* n2);
void test(void);

int main(void) {
    queue_t p_queue;
    tree_t* search_tree = NULL;
    sol_t solution;

    /*test();*/
    solve8Tile(&p_queue, &search_tree, "12345 678");
    loadSolution(&p_queue, &solution);
    printSolution(&solution);

    /*unloadPQueue(&p_queue);
    unloadTree(&search_tree);*/

    return 0;
}

/* ------ SOLVER FUNCTIONS ------ */
/* Solver initailises the queue with the starting grid and then expand nodes
 * onto the queue until a solution is found. Assumes a valid string. Uses
 * breadth first search
 */
void solve8Tile(queue_t* p_queue, tree_t** tree, char* s) {
    node_t* node = initNode(s);
    initPQueue(p_queue, node);
    *tree = initTree(node);
    while (!expandNode(p_queue, *tree)) {
        /* TODO could add isEMpyt function */
    }
}

/* Calls function shiftTile to expand possible moves of current node. Will 
 * return true if the solution is found
 */
bool expandNode(queue_t* p_queue, tree_t* tree) {
    node_t* parent = getMin(p_queue);

    int x = parent->x;
    int y = parent->y;
    if (x < SIZE - 1) {
        if (shiftTile(LEFT, p_queue, tree, parent)) {
            return true;
        }
    }
    if (x > 0) {
        if (shiftTile(RIGHT, p_queue, tree, parent)) {
            return true;
        }
    }
    if (y < SIZE - 1) {
        if (shiftTile(UP, p_queue, tree, parent)) {
            return true;
        }
    }
    if (y > 0) {
        if (shiftTile(DOWN, p_queue, tree, parent)) {
            return true;
        }
    }
    /* Moving to next list element is effectively dequeuing the current node,
       without having to move it somewhere else for later duplicate checking */
    delMin(p_queue);
    return false;
}

/* Generates next board state based on direction of shift. Shift direction 
 * refers to direction of tile being moved into the free space. Function assumes
 * that a valid shiftdirection is given. Copies board into queue, but queue
 * current index is only incremented if it's a valid board. This avoid copying
 * to a tmp and then copying to the queue
 */
bool shiftTile(swap_t dir, queue_t* p_queue, tree_t* tree, node_t* parent) {
    int x1, y1, x2, y2;
    static node_t* tmp = NULL;

    /* tmp will only be malloced if the previous one has been added to the
       queue */
    if (tmp == NULL) {
        tmp = (node_t*)malloc(sizeof(node_t));
        if (tmp == NULL) {
            fprintf(stderr, "Memory allocation error\n");
            exit(EXIT_FAILURE);
        }
    }

    /* Conditional evaluation will determine swap coordinates. "dir" will only
     * ever be one of these values, and as such the shift can only go, UP,
     * DOWN, LEFT or RIGHT
     */
    x1 = parent->x;
    y1 = parent->y;
    x2 = x1 + (dir == LEFT) - (dir == RIGHT);
    y2 = y1 + (dir == UP) - (dir == DOWN);

    memcpy(tmp->grid, parent->grid, SIZE * SIZE * sizeof(int));
    swap(&tmp->grid[y1][x1], &tmp->grid[y2][x2]);

    tmp->x = x2;
    tmp->y = y2;
    tmp->step = parent->step + 1;
    tmp->parent = parent;
    tmp->f = fPriority(tmp->grid, tmp->step);

    if (checkTarget(tmp->grid)) {
        insertTree(tree, tmp);
        insertPQueue(p_queue, tmp);
        return true;
    } else if (!searchInTree(tree, tmp->grid)) {
        insertTree(tree, tmp);
        insertPQueue(p_queue, tmp);
        /* reset tmp so it is malloced again on next loop */
        tmp = NULL;
    }

    return false;
}

/* This just calls the compare boards function, but function adds to readability
 * above
 */
bool checkTarget(int grid[SIZE][SIZE]) {
    static int target[SIZE][SIZE] = {{1, 2, 3},
                                     {4, 5, 6},
                                     {7, 8, 0}};

    return compareBoards(target, grid);
}

/* Compares whether two 8-tile boards are the same. Have used memcmp() for
 * improved speed. Did compare against storing a value key for each grid and 
 * using that as the comparator, but the speed seemed basically the same. 
 */
bool compareBoards(int grid1[SIZE][SIZE], int grid2[SIZE][SIZE]) {
    return !(memcmp(grid1, grid2, SIZE * SIZE * sizeof(int)));
}

/* ------- LOADING FUNCTIONS ------ */
node_t* initNode(char* s) {
    node_t* node = (node_t*)malloc(sizeof(node_t));

    loadBoard(node->grid, s);
    findFreeTile(node);
    node->step = 0;
    node->parent = NULL;
    /* FIXME not really required */
    node->f = fPriority(node->grid, node->step);
    return node;
}

/* Loads string representation of the board into an array. Expects valid input
 * which has been checked with checkInputString()
 */
void loadBoard(int grid[SIZE][SIZE], char* s) {
    int i, j;
    char val;

    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            val = s[SIZE * i + j];

            if (val == ' ') {
                grid[i][j] = 0;
            } else {
                grid[i][j] = (int)(val - '0');
            }
        }
    }
}

/* Finds coordinates of free tile and stores them in grid_t* struct. Only used
 * for initilisation as free tiles for expanded nodes can be inferred directly
 * from the swap direction.
 */
void findFreeTile(node_t* node) {
    int i, j;

    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            if (node->grid[i][j] == 0) {
                node->x = j;
                node->y = i;
                return;
            }
        }
    }
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

    /* Set sentinel node value and initialise first node*/
    p_queue->node[0] = NULL;
    p_queue->node[1] = node;

    p_queue->size = QUEUE_SIZE;
    p_queue->elem = 1;
    p_queue->back = 1;
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

node_t* getMin(queue_t* p_queue) {
    return p_queue->node[1];
}

void delMin(queue_t* p_queue) {
    p_queue->node[1] = p_queue->node[p_queue->back];
    percolateDown(p_queue);

    p_queue->elem--;
    p_queue->back--;
}

bool isEmpty(queue_t* p_queue) {
    if (p_queue->elem == 0) {
        return true;
    }
    return false;
}

void unloadPQueue(queue_t* p_queue) {
    free(p_queue->node);
}

void expandPQueue(queue_t* p_queue) {
    p_queue->size = p_queue->size * BUFF_FACTOR + 1;

    p_queue->node = (node_t**)realloc(p_queue->node, p_queue->size * sizeof(node_t*));
    if (p_queue->node == NULL) {
        fprintf(stderr, "Memory allocation error\n");
        /* QUESTION should i free previously allced memory?*/
        exit(EXIT_FAILURE);
    }
}

void percolateUp(queue_t* p_queue) {
    node_t *child, *parent;
    size_t i = p_queue->back;

    while (i / 2 != 0) {
        child = p_queue->node[i];
        parent = p_queue->node[i / 2];
        if (child->f < parent->f) {
            swapNodePtr(&child, &parent);
        }
        i /= 2;
    }
}

void percolateDown(queue_t* p_queue) {
    node_t *child, *parent;
    size_t i = 1;

    while (i * 2 <= p_queue->elem) {
        parent = p_queue->node[i];

        i = minChildIndex(p_queue, i);
        child = p_queue->node[i];

        if (child->f < parent->f) {
            swapNodePtr(&child, &parent);
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

void swapNodePtr(node_t** n1, node_t** n2) {
    node_t* tmp = *n1;
    *n1 = *n2;
    *n2 = tmp;
}

/* ------ SEARCH TREE FOR VISITED LIST ------ */
tree_t* initTree(node_t* node) {
    tree_t* tree = (tree_t*)calloc(1, sizeof(tree_t));
    if (tree == NULL) {
        fprintf(stderr, "Memory allocation error\n");
        exit(EXIT_FAILURE);
    }

    insertTree(tree, node);
    return tree;
}

void insertTree(tree_t* tree, node_t* node) {
    int i, j, leaf;
    tree_t* branch = tree;

    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            leaf = node->grid[i][j];
            if (branch->children[leaf] == NULL) {
                branch->children[leaf] = createTreeNode();
            }
            branch = branch->children[leaf];
        }
    }
    /* Insert pointer to node at end of branches */
    branch->node = node;
}

/* FIXME combine search and insert? */
bool searchInTree(tree_t* tree, int grid[SIZE][SIZE]) {
    int i, j, leaf;
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

tree_t* createTreeNode(void) {
    int i;
    /* FIXME just calloc */
    tree_t* ptr = (tree_t*)calloc(1, sizeof(tree_t));
    if (ptr == NULL) {
        /* QUESTION should i free previously allced memory?*/
        fprintf(stderr, "Memory allocation error\n");
        exit(EXIT_FAILURE);
    }

    /*   for (i = 0; i < SIZE * SIZE; i++) {
        ptr->children[i] = NULL;
    }

    /* Doesn't set ptr to queue node until very end externally */
    /* ptr->node = NULL;*/
    return ptr;
}

/*FIXME not totally sure on this */
void unloadTree(tree_t* tree) {
    int i;
    for (i = 0; i < SIZE * SIZE; i++) {
        if (tree->children[i]) {
            unloadTree(tree->children[i]);
        } else {
            free(tree->node);
        }
    }
    free(tree);
}

/* ------- PRIORIITY FUNCTION CALCULATION ------- */
int fPriority(int grid[SIZE][SIZE], int step) {
    return manhattanDistance(grid) + step;
}

int manhattanDistance(int grid[SIZE][SIZE]) {
    int i, j;
    int manhattan = 0;
    int num;
    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            num = grid[i][j] - 1;
            /* FIXME definition doesn't ignore 0 */
            manhattan += abs(num / 3 - i) + abs(num % 3 - j);
        }
    }
    return manhattan;
}

int hammingDistance(int grid[SIZE][SIZE]) {
    int i, j;
    int hamming = 0;

    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            /* FIXME definition doesn't ignore 0 */
            hamming += (grid[i][j] != (3 * i + j + 1) % (SIZE * SIZE));
        }
    }
    return hamming;
}

/* ------ UTILITY & INPUT FUNCTIONS ------ */
/* Loads solution by going through parent nodes back to start grid
 */
void loadSolution(queue_t* p_queue, sol_t* solution) {
    int i;
    node_t* node = p_queue->node[1];

    /* FIXME do I need the step counter? */
    i = solution->steps = node->step;

    while (node != NULL) {
        solution->node[i] = node;
        node = node->parent;
        i--;
    }
}

void printSolution(sol_t* solution) {
    int i;

    printf("Starting board:\n");
    printBoard(solution->node[0]->grid);
    for (i = 1; i <= solution->steps; i++) {
        printf("Step %i:\n", i);
        printBoard(solution->node[i]->grid);
    }
}

/* Print a single 8-tile board
 */
void printBoard(int grid[SIZE][SIZE]) {
    int i, j;
    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            printf("%d", grid[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

/* FIXME can this be made more concise */
/* Checks that a valid string has been inputted 
 */
bool checkInputString(char* s) {
    size_t len, i;
    int count[SIZE * SIZE] = {0};

    /* Check string length */
    if ((len = strlen(s)) != SIZE * SIZE) {
        if (len < SIZE * SIZE) {
            fprintf(stderr, "String is shorter than expected..\n");
        } else {
            fprintf(stderr, "String is longer than expected..\n");
        }
        return false;
    }

    /* Check for invalid chars and check valid ones are unique */
    for (i = 0; i < SIZE * SIZE; i++) {
        if (s[i] == ' ') {
            count[0]++;
        } else if ('1' <= s[i] && s[i] <= '8') {
            count[s[i] - '0']++;
        } else {
            fprintf(stderr, "Invalid character \"%c\" in input...\n", s[i]);
            return false;
        }
    }

    for (i = 0; i < SIZE * SIZE; i++) {
        if (count[i] > 1) {
            fprintf(stderr, "Each tile must have a unique value...\n");
            return false;
        }
    }

    return true;
}

/* Reference: https://www.geeksforgeeks.org/check-instance-8-puzzle-solvable/ 
 * Checks whether the input string is acutally solvable before attempting to 
 * find a solution
 */
/* FIXME readability */
bool isSolvable(char* s) {
    int i;
    int inversions = 0;
    int grid[SIZE][SIZE];

    loadBoard(grid, s);

    for (i = 0; i < SIZE * SIZE - 1; i++) {
        if (*(grid + i + 1) && *(grid + i) && *(grid + i + 1) > *(grid + i)) {
            inversions++;
        }
    }

    return inversions % 2 == 0;
}

void swap(int* n1, int* n2) {
    int tmp = *n1;
    *n1 = *n2;
    *n2 = tmp;
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
    assert(manhattanDistance(test_grid) == 12);
}