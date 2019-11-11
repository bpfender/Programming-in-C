#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 3

typedef enum swap_t { UP,
                      DOWN,
                      LEFT,
                      RIGHT } swap_t;

typedef struct node_t {
    int grid[SIZE][SIZE];
    size_t x;
    size_t y;
    size_t step;
    struct node_t* parent;
    struct node_t* next;
} node_t;

typedef struct queue_t {
    node_t* start;
    node_t* curr;
    node_t* end;
} queue_t;

void solvePuzzle(queue_t* queue, char* s);

/* BUILDING CHILDREN */
int getChildren(queue_t* queue);
int swapTile(swap_t dir, queue_t* queue);
int checkUnique(queue_t* queue, int grid[SIZE][SIZE]);
int checkTarget(int grid[SIZE][SIZE]);
void enqueue(queue_t* queue, node_t* grid);

int compareBoards(int grid1[SIZE][SIZE], int grid2[SIZE][SIZE]);
void swap(int* n1, int* n2);

/* INITIALISATION & UTILITY FUNCTIONS */
void initQueue(queue_t* queue, char* s);
void loadBoard(int grid[SIZE][SIZE], char* s);
void markXY(node_t* grid);
void printBoard(int grid[SIZE][SIZE]);

void unloadQueue(node_t* node);

void test(void);

int main(void) {
    test();
    return 0;
}

void solvePuzzle(queue_t* queue, char* s) {
    node_t* node;
    initQueue(queue, s);

    while (!getChildren(queue)) {
    }

    printf("Steps: %li\n", queue->end->step);

    node = queue->end;
    while (node != NULL) {
        printBoard(node->grid);
        node = node->parent;
    }
}

int getChildren(queue_t* queue) {
    node_t* parent = queue->curr;
    size_t x = parent->x;
    size_t y = parent->y;

    if (x < SIZE - 1) {
        if (swapTile(RIGHT, queue)) {
            return 1;
        }
    }
    if (x > 0) {
        if (swapTile(LEFT, queue)) {
            return 1;
        }
    }
    if (y < SIZE - 1) {
        if (swapTile(DOWN, queue)) {
            return 1;
        }
    }
    if (y > 0) {
        if (swapTile(UP, queue)) {
            return 1;
        }
    }
    queue->curr = parent->next;
    return 0;
}

/* QUESTION Can i do memory allocation earlier to save having to build node_t every time */
int swapTile(swap_t dir, queue_t* queue) {
    node_t tmp;
    node_t* parent = queue->curr;

    size_t x1 = parent->x;
    size_t y1 = parent->y;
    /* QUESTION is this naughty? can i rely on conditional evalutation */
    size_t x2 = x1 + (dir == RIGHT) - (dir == LEFT);
    size_t y2 = y1 + (dir == DOWN) - (dir == UP);

    memcpy(tmp.grid, parent->grid, SIZE * SIZE * sizeof(int));
    swap(&tmp.grid[y1][x1], &tmp.grid[y2][x2]);

    tmp.x = x2;
    tmp.y = y2;
    tmp.step = parent->step + 1;
    tmp.parent = parent;
    tmp.next = NULL;

    if (checkUnique(queue, tmp.grid)) {
        enqueue(queue, &tmp);
    }

    return checkTarget(tmp.grid);
}

/* TODO is this the best way to check for repetition */
/* TODO restructure as binary tree? */
int checkUnique(queue_t* queue, int grid[SIZE][SIZE]) {
    node_t* node = queue->start;

    /* FIXME should this be do while? */
    while (node != NULL) {
        if (compareBoards(node->grid, grid)) {
            return 0;
        }
        node = node->next;
    }
    return 1;
}

int checkTarget(int grid[SIZE][SIZE]) {
    /* QUESTION in termsof notation better to have a const */
    int target[SIZE][SIZE] = {{1, 2, 3},
                              {4, 5, 6},
                              {7, 8, 0}};

    return compareBoards(target, grid);
}

void enqueue(queue_t* queue, node_t* node) {
    node_t* ptr = (node_t*)malloc(sizeof(node_t));
    if (ptr == NULL) {
        /*TODO, free array */
        fprintf(stderr, "Memory allocation failed\n");
    }

    memcpy(ptr, node, sizeof(node_t));

    queue->end->next = ptr;
    queue->end = ptr;
}

int compareBoards(int grid1[SIZE][SIZE], int grid2[SIZE][SIZE]) {
    /* memcmp returns 0 when strs are the same */
    return !(memcmp(grid1, grid2, SIZE * SIZE * sizeof(int)));
}

void swap(int* n1, int* n2) {
    int tmp = *n1;
    *n1 = *n2;
    *n2 = tmp;
}

/* ------ UTILITY FUNCTIONS ------ */
void initQueue(queue_t* queue, char* s) {
    node_t* ptr = (node_t*)malloc(sizeof(node_t));
    if (ptr == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    queue->start = queue->curr = queue->end = ptr;

    loadBoard(queue->start->grid, s);
    markXY(queue->start);

    queue->start->step = 0;
    queue->start->parent = NULL;
    queue->start->next = NULL;
}

/* TODO requires error checking on input string */
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

void markXY(node_t* node) {
    size_t i, j;

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

/* TODO still needs to be tested */
int checkInput(char* s) {
    size_t i;

    if (strlen(s) != SIZE * SIZE) {
        fprintf(stderr, "String is longer than expected..\n");
        return 0;
    }

    for (i = 0; i < SIZE * SIZE; i++) {
        if (!(isdigit(s[i]) || s[i] == ' ')) {
            fprintf(stderr, "Invalid character in input...\n");
            return 0;
        }
    }

    return 1;
}

void unloadQueue(node_t* node) {
    if (node->next != NULL) {
        unloadQueue(node->next);
    }
    free(node);
}

void test(void) {
    size_t i, j;

    int test_grid[SIZE][SIZE];
    node_t test_board;
    static queue_t test_queue;

    /* Testing that grid gets loaded up properly with string */
    loadBoard(test_grid, " 12345678");
    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            assert(test_grid[i][j] == (int)(SIZE * i + j));
        }
    }

    loadBoard(test_grid, "1234 5678");
    assert(test_grid[0][0] == 1);
    assert(test_grid[1][1] == 0);
    assert(test_grid[2][2] == 8);

    /* Testing swap of two cells */
    swap(&test_grid[0][0], &test_grid[2][2]);
    assert(test_grid[0][0] == 8);
    assert(test_grid[2][2] == 1);

    printBoard(test_grid);

    /* Put grid into grid_t type and check empy cell finding  */
    loadBoard(test_grid, " 23415678");
    memcpy(test_board.grid, test_grid, SIZE * SIZE * sizeof(int));

    markXY(&test_board);
    assert(test_board.x == 0);
    assert(test_board.y == 0);

    printBoard(test_board.grid);

    /* Check that queue initialisation works properly */
    initQueue(&test_queue, "1234 5678");

    assert(test_queue.end->grid[0][0] == 1);
    assert(test_queue.end->grid[2][2] == 8);
    assert(test_queue.end->x == 1);
    assert(test_queue.end->y == 1);

    printBoard(test_queue.start->grid);
    unloadQueue(test_queue.start);

    /* Check that whole set of children are generated properly */
    initQueue(&test_queue, "1234 5678");
    getChildren(&test_queue);

    /* TODO more explicit testing here. On visual inspection it work */

    /* Testing of grid duplication avoidance */
    /* TODO more explicit testing required */
    getChildren(&test_queue);

    /* Testing of checkTarget() */
    loadBoard(test_grid, "12345678 ");
    assert(checkTarget(test_grid) == 1);

    unloadQueue(test_queue.start);

    /* Testing of checkUnique */
    /* TODO requires more extensive testing */
    initQueue(&test_queue, "12345678 ");
    loadBoard(test_grid, "12345678 ");
    assert(checkUnique(&test_queue, test_grid) == 0);

    loadBoard(test_grid, "123 45678");
    assert(checkUnique(&test_queue, test_grid) == 1);

    unloadQueue(test_queue.start);

    /* Testing of createChild() */
    initQueue(&test_queue, "1234 5678");
    swapTile(DOWN, &test_queue);
    assert(test_queue.start->next->x == 1 && test_queue.start->next->y == 2);
    printBoard(test_board.grid);

    unloadQueue(test_queue.start);

    printf("Solve START\n");
    solvePuzzle(&test_queue, "123 45678");
    unloadQueue(test_queue.start);
}
