#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 3
#define QUEUE 362880

typedef enum swap_t { UP,
                      DOWN,
                      LEFT,
                      RIGHT } swap_t;

typedef struct grid_t {
    int grid[SIZE][SIZE];
    size_t x;
    size_t y;
    size_t parent;
    size_t step;
} grid_t;

typedef struct queue_t {
    grid_t children[QUEUE];
    size_t curr;
    size_t end;
} queue_t;

void solvePuzzle(queue_t* queue, char* s);

/* BUILDING CHILDREN */
int getChildren(queue_t* queue);
int swapTile(swap_t dir, queue_t* queue);
int checkUnique(queue_t* queue, int grid[SIZE][SIZE]);
int checkTarget(int grid[SIZE][SIZE]);
void enqueue(queue_t* queue, grid_t* grid);

int compareBoards(int grid1[SIZE][SIZE], int grid2[SIZE][SIZE]);
void swap(int* n1, int* n2);

/* INITIALISATION & UTILITY FUNCTIONS */
void initQueue(queue_t* queue, char* s);
void loadBoard(int grid[SIZE][SIZE], char* s);
void markFreeCell(grid_t* grid);
void printBoard(int grid[SIZE][SIZE]);

void test(void);

int main(void) {
    test();
    return 0;
}

void solvePuzzle(queue_t* queue, char* s) {
    size_t index;
    initQueue(queue, s);

    while (!getChildren(queue)) {
    }

    printf("Iterations: %li\n", queue->curr);
    printf("Steps: %li\n", queue->children[queue->end].step);

    index = queue->end;
    while (queue->children[index].step != 0) {
        printBoard(queue->children[index].grid);
        index = queue->children[index].parent;
    }
    printBoard(queue->children[0].grid);
}

int getChildren(queue_t* queue) {
    size_t parent = queue->curr;
    size_t x = queue->children[parent].x;
    size_t y = queue->children[parent].y;

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
    queue->curr++;
    return 0;
}

int swapTile(swap_t dir, queue_t* queue) {
    grid_t tmp;
    size_t parent = queue->curr;

    size_t x1 = queue->children[parent].x;
    size_t y1 = queue->children[parent].y;
    /* QUESTION is this naughty? can i rely on conditional evalutation */
    size_t x2 = x1 + (dir == RIGHT) - (dir == LEFT);
    size_t y2 = y1 + (dir == DOWN) - (dir == UP);

    memcpy(tmp.grid, queue->children[parent].grid, SIZE * SIZE * sizeof(int));
    swap(&tmp.grid[y1][x1], &tmp.grid[y2][x2]);

    tmp.x = x2;
    tmp.y = y2;
    tmp.parent = parent;
    tmp.step = queue->children[parent].step + 1;

    if (checkUnique(queue, tmp.grid)) {
        enqueue(queue, &tmp);
    }

    return checkTarget(tmp.grid);
}

/* TODO is this the best way to check for repetition */
/* TODO restructure as binary tree? */
int checkUnique(queue_t* queue, int grid[SIZE][SIZE]) {
    size_t i;
    for (i = 0; i <= queue->end; i++) {
        if (compareBoards(queue->children[i].grid, grid)) {
            return 0;
        }
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

void enqueue(queue_t* queue, grid_t* grid) {
    size_t end = ++queue->end;

    memcpy(&queue->children[end], grid, sizeof(grid_t));
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
    size_t end = queue->end = 0;
    queue->curr = 0;
    loadBoard(queue->children[end].grid, s);

    markFreeCell(&queue->children[end]);
    queue->children[end].step = 0;
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

void markFreeCell(grid_t* grid) {
    size_t i, j;

    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            if (grid->grid[i][j] == 0) {
                grid->x = j;
                grid->y = i;
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

void test(void) {
    size_t i, j;

    int test_grid[SIZE][SIZE];
    grid_t test_board;
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

    markFreeCell(&test_board);
    assert(test_board.x == 0);
    assert(test_board.y == 0);

    printBoard(test_board.grid);

    /* Check that queue initialisation works properly */
    initQueue(&test_queue, "1234 5678");
    assert(test_queue.end == 0);
    assert(test_queue.children[test_queue.end].grid[0][0] == 1);
    assert(test_queue.children[test_queue.end].grid[2][2] == 8);
    assert(test_queue.children[test_queue.end].x == 1);
    assert(test_queue.children[test_queue.end].y == 1);

    printBoard(test_queue.children[0].grid);

    /* Check that whole set of children are generated properly */
    initQueue(&test_queue, "1234 5678");
    getChildren(&test_queue);

    /* TODO more explicit testing here. On visual inspection it work */
    printBoard(test_queue.children[0].grid);
    printBoard(test_queue.children[1].grid);
    printBoard(test_queue.children[2].grid);
    printBoard(test_queue.children[3].grid);
    printBoard(test_queue.children[4].grid);

    /* Testing of grid duplication avoidance */
    /* TODO more explicit testing required */
    getChildren(&test_queue);
    for (i = 0; i <= test_queue.end; i++) {
        printf("Board: %li\n", i);
        assert(compareBoards(test_queue.children[0].grid, test_queue.children[i + 1].grid) == 0);
        printBoard(test_queue.children[i].grid);
    }

    /* Testing of checkTarget() */
    loadBoard(test_grid, "12345678 ");
    assert(checkTarget(test_grid) == 1);

    /* Testing of checkUnique */
    /* TODO requires more extensive testing */
    initQueue(&test_queue, "12345678 ");
    loadBoard(test_grid, "12345678 ");
    assert(checkUnique(&test_queue, test_grid) == 0);

    loadBoard(test_grid, "123 45678");
    assert(checkUnique(&test_queue, test_grid) == 1);

    /* Testing of createChild() */
    initQueue(&test_queue, "1234 5678");
    swapTile(DOWN, &test_queue);
    assert(test_queue.children[1].x == 1 && test_queue.children[1].y == 2);
    printBoard(test_board.grid);

    printf("Solve START\n");
    solvePuzzle(&test_queue, "123 45678");
}
