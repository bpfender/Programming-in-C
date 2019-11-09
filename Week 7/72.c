#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 3
#define QUEUE 362880

typedef struct grid_t {
    int board[SIZE][SIZE];
    size_t x;
    size_t y;
} grid_t;

typedef struct queue_t {
    grid_t queue[QUEUE];
    size_t index;
} queue_t;

void createChildren(queue_t* queue);
void buildChild(queue_t* queue, size_t parent, size_t x_swap, size_t y_swap);
void swap(int* n1, int* n2);

void initQueue(queue_t* queue, char* s);
void loadBoard(int board[SIZE][SIZE], char* s);
void findEmptyCell(grid_t* board);
void printBoard(int board[SIZE][SIZE]);

void test(void);

int main(void) {
    test();
    return 0;
}

void createChildren(queue_t* queue) {
    size_t parent = queue->index;

    size_t x = queue->queue[parent].x;
    size_t y = queue->queue[parent].y;

    if (x < SIZE - 1) {
        buildChild(queue, parent, x + 1, y);
    }
    if (x > 0) {
        buildChild(queue, parent, x - 1, y);
    }
    if (y < SIZE - 1) {
        buildChild(queue, parent, x, y + 1);
    }
    if (y > 0) {
        buildChild(queue, parent, x, y - 1);
    }
}

void buildChild(queue_t* queue, size_t parent, size_t x_swap, size_t y_swap) {
    size_t index = ++queue->index;
    size_t x_zero = queue->queue[parent].x;
    size_t y_zero = queue->queue[parent].y;

    int(*child_board)[SIZE] = queue->queue[index].board;
    int(*parent_board)[SIZE] = queue->queue[parent].board;

    memcpy(child_board, parent_board, SIZE * SIZE * sizeof(int));
    swap(&child_board[y_zero][x_zero], &child_board[y_swap][x_swap]);

    queue->queue[index].x = x_swap;
    queue->queue[index].y = y_swap;
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

void test(void) {
    int i, j;

    int test_grid[SIZE][SIZE];
    grid_t test_board;
    static queue_t test_queue;

    /* Testing that board gets loaded up properly with string */
    loadBoard(test_grid, " 12345678");
    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            assert(test_grid[i][j] == SIZE * i + j);
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

    printBoard(test_queue.queue[1].board);

    /* Check that whole set of children are generated properly */
    initQueue(&test_queue, "1234567 8");
    createChildren(&test_queue);

    /* TODO more explicit testing here. On visual inspection it work */
    printBoard(test_queue.queue[0].board);
    printBoard(test_queue.queue[1].board);
    printBoard(test_queue.queue[2].board);
    printBoard(test_queue.queue[3].board);
    printBoard(test_queue.queue[4].board);
}
