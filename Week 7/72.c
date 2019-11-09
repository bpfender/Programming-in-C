#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 3
#define QUEUE 362880

typedef struct grid_t {
    int board[SIZE][SIZE];
    size_t x;
    size_t y;
    size_t parent;
} grid_t;

typedef struct queue_t {
    grid_t queue[QUEUE];
    size_t index;
} queue_t;

static int target[SIZE][SIZE] = {{1, 2, 3},
                                 {4, 5, 6},
                                 {7, 8, 0}};

void solvePuzzle(queue_t* queue, char* start);

/* BUILDING CHILDREN */
int createChildren(queue_t* queue, size_t parent);
void buildChild(queue_t* queue, size_t parent, size_t x_swap, size_t y_swap);
int compareBoards(int grid1[SIZE][SIZE], int grid2[SIZE][SIZE]);
void swap(int* n1, int* n2);

/* INITIALISATION & UTILITY FUNCTIONS */
void initQueue(queue_t* queue, char* s);
void loadBoard(int board[SIZE][SIZE], char* s);
void findEmptyCell(grid_t* board);
void printBoard(int board[SIZE][SIZE]);

void test(void);

int main(void) {
    test();
    return 0;
}

void solvePuzzle(queue_t* queue, char* start) {
    size_t parent = 0;
    initQueue(queue, start);

    while (!createChildren(queue, parent)) {
        parent++;
    }

    printf("%li\n", parent);
}

int createChildren(queue_t* queue, size_t parent) {
    size_t x = queue->queue[parent].x;
    size_t y = queue->queue[parent].y;

    if (x < SIZE - 1) {
        buildChild(queue, parent, x + 1, y);
        if (compareBoards(queue->queue[queue->index].board, target)) {
            printf("Board found\n");
            return 1;
        }
    }
    if (x > 0) {
        buildChild(queue, parent, x - 1, y);
        if (compareBoards(queue->queue[queue->index].board, target)) {
            printf("Board found\n");
            return 1;
        }
    }
    if (y < SIZE - 1) {
        buildChild(queue, parent, x, y + 1);
        if (compareBoards(queue->queue[queue->index].board, target)) {
            printf("Board found\n");
            return 1;
        }
    }
    if (y > 0) {
        buildChild(queue, parent, x, y - 1);
        if (compareBoards(queue->queue[queue->index].board, target)) {
            printf("Board found\n");
            return 1;
        }
    }
    parent++;
    return 0;
}

void buildChild(queue_t* queue, size_t parent, size_t x_swap, size_t y_swap) {
    size_t i;
    size_t index = ++queue->index;
    size_t x_zero = queue->queue[parent].x;
    size_t y_zero = queue->queue[parent].y;

    int(*child_board)[SIZE] = queue->queue[index].board;
    int(*parent_board)[SIZE] = queue->queue[parent].board;

    memcpy(child_board, parent_board, SIZE * SIZE * sizeof(int));
    swap(&child_board[y_zero][x_zero], &child_board[y_swap][x_swap]);

    /* FIXME is there a better way of checking for duplicates? */
    for (i = 0; i < queue->index; i++) {
        if (compareBoards(child_board, queue->queue[i].board)) {
            queue->index--;
            return;
        }
    }
    queue->queue[index].x = x_swap;
    queue->queue[index].y = y_swap;
    queue->queue[index].parent = parent;
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
    size_t index = queue->index = 0;
    loadBoard(queue->queue[index].board, s);
    findEmptyCell(&queue->queue[index]);
}

/* TODO requires error checking on input string */
void loadBoard(int board[SIZE][SIZE], char* s) {
    int i, j;
    char val;

    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            val = s[SIZE * i + j];

            if (val == ' ') {
                board[i][j] = 0;
            } else {
                board[i][j] = (int)(val - '0');
            }
        }
    }
}

void findEmptyCell(grid_t* board) {
    size_t i, j;

    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            if (board->board[i][j] == 0) {
                board->x = j;
                board->y = i;
                return;
            }
        }
    }
}

void printBoard(int board[SIZE][SIZE]) {
    int i, j;
    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            printf("%d", board[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

/* TODO still needs to be tested */
int validInput(char* s) {
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

    /* Testing that board gets loaded up properly with string */
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

    /* Put board into grid_t type and check empy cell finding  */
    loadBoard(test_grid, " 23415678");
    memcpy(test_board.board, test_grid, SIZE * SIZE * sizeof(int));

    findEmptyCell(&test_board);
    assert(test_board.x == 0);
    assert(test_board.y == 0);

    printBoard(test_board.board);

    /* Check that queue initialisation works properly */
    initQueue(&test_queue, "1234 5678");
    assert(test_queue.index == 0);
    assert(test_queue.queue[test_queue.index].board[0][0] == 1);
    assert(test_queue.queue[test_queue.index].board[2][2] == 8);
    assert(test_queue.queue[test_queue.index].x == 1);
    assert(test_queue.queue[test_queue.index].y == 1);

    printBoard(test_queue.queue[0].board);

    /* Check that single child is generated properly */
    buildChild(&test_queue, 0, 1, 2);
    assert(test_queue.index == 1);
    assert(test_queue.queue[test_queue.index].board[0][0] == 1);
    assert(test_queue.queue[test_queue.index].board[2][2] == 8);
    assert(test_queue.queue[test_queue.index].x == 1);
    assert(test_queue.queue[test_queue.index].y == 2);
    assert(test_queue.queue[test_queue.index].parent == 0);

    printBoard(test_queue.queue[1].board);

    /* Check that whole set of children are generated properly */
    initQueue(&test_queue, "1234 5678");
    createChildren(&test_queue, 0);

    /* TODO more explicit testing here. On visual inspection it work */
    printBoard(test_queue.queue[0].board);
    printBoard(test_queue.queue[1].board);
    printBoard(test_queue.queue[2].board);
    printBoard(test_queue.queue[3].board);
    printBoard(test_queue.queue[4].board);

    /* Testing of board duplication avoidance */
    /* TODO more explicit testing required */
    createChildren(&test_queue, 0);
    for (i = 0; i <= test_queue.index; i++) {
        printf("Board: %li\n", i);
        assert(compareBoards(test_queue.queue[0].board, test_queue.queue[i + 1].board) == 0);
        printBoard(test_queue.queue[i].board);
    }

    /* Test solver */
    printf("Solve start\n\n");
    solvePuzzle(&test_queue, "1234 5678");
}
