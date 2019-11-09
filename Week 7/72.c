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
    int board[SIZE][SIZE];
    size_t x;
    size_t y;
    size_t parent;
    size_t step;
} grid_t;

typedef struct queue_t {
    grid_t children[QUEUE];
    size_t index;
} queue_t;

static int target[SIZE][SIZE] = {{1, 2, 3},
                                 {4, 5, 6},
                                 {7, 8, 0}};

void solvePuzzle(queue_t* queue, char* start);

/* BUILDING CHILDREN */
int createChildren(queue_t* queue, size_t parent);
void buildChild(queue_t* queue, size_t parent, size_t x_swap, size_t y_swap);
void createChild(grid_t* parent, grid_t* child, size_t x_swap, size_t y_swap, size_t index);
int checkUnique(queue_t* queue, int grid[SIZE][SIZE]);
int checkTarget(int grid[SIZE][SIZE]);
void enqueue(queue_t* queue, grid_t* grid);

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
    size_t index;
    initQueue(queue, start);

    while (!createChildren(queue, parent)) {
        parent++;
    }

    printf("Iterations: %li\n", parent);
    printf("Steps: %li\n", queue->children[queue->index].step);

    index = queue->index;
    while (queue->children[index].parent != 0) {
        printBoard(queue->children[index].board);
        index = queue->children[index].parent;
    }
    printBoard(queue->children[1].board);
    printBoard(queue->children[0].board);
}

int createChildren(queue_t* queue, size_t parent) {
    grid_t tmp_grid;

    size_t x = queue->children[parent].x;
    size_t y = queue->children[parent].y;

    if (x < SIZE - 1) {
        createChild(&queue->children[parent], &tmp_grid, x + 1, y, parent);
        if (checkTarget(tmp_grid.board)) {
            enqueue(queue, &tmp_grid);
            return 1;
        }
        if (checkUnique(queue, tmp_grid.board)) {
            enqueue(queue, &tmp_grid);
        }
    }
    if (x > 0) {
        createChild(&queue->children[parent], &tmp_grid, x - 1, y, parent);
        if (checkTarget(tmp_grid.board)) {
            enqueue(queue, &tmp_grid);
            return 1;
        }
        if (checkUnique(queue, tmp_grid.board)) {
            enqueue(queue, &tmp_grid);
        }
    }
    if (y < SIZE - 1) {
        createChild(&queue->children[parent], &tmp_grid, x, y + 1, parent);
        if (checkTarget(tmp_grid.board)) {
            enqueue(queue, &tmp_grid);
            return 1;
        }
        if (checkUnique(queue, tmp_grid.board)) {
            enqueue(queue, &tmp_grid);
        }
    }
    if (y > 0) {
        createChild(&queue->children[parent], &tmp_grid, x, y - 1, parent);
        if (checkTarget(tmp_grid.board)) {
            enqueue(queue, &tmp_grid);
            return 1;
        }
        if (checkUnique(queue, tmp_grid.board)) {
            enqueue(queue, &tmp_grid);
        }
        /*buildChild(queue, parent, x, y - 1);
        if (compareBoards(queue->children[queue->index].board, target)) {
            printf("Board found\n");
            return 1;
        }*/
    }
    return 0;
}

int move(swap_t dir, queue_t* queue, size_t parent) {
    size_t x = queue->children[parent].x;
    size_t y = queue->children[parent].y;
    size_t x2 = x + (dir == RIGHT) - (dir == LEFT);
    size_t y2 = y + (dir == DOWN) = (dir == UP);

    grid_t tmp;

    memcpy(tmp.board, &queue->children[parent], SIZE * SIZE * sizeof(int));
    swap(tmp.board[x][y], tmp.board[x2][y2]);

    /* switch (dir) {
        case UP:
            swap(tmp.board[x][y], tmp.board[x][y + 1]);
            break;
        case DOWN:
            swap(tmp.board[x][y], tmp.board[x][y - 1]);
            break;
        case LEFT:
            swap(tmp.board[x][y], tmp.board[x + 1][y]);
            break;
        case RIGHT:
            swap(tmp.board[x][y], tmp.board[x - 1][y]);
            break;
    }*/

    if (checkUnique(queue, tmp.board)) {
        enqueue(queue, &tmp);
    }
}

void createChild(grid_t* parent, grid_t* child, size_t x_swap, size_t y_swap, size_t parent_index) {
    size_t x_free = parent->x;
    size_t y_free = parent->y;

    /* Copy original grid */
    memcpy(child->board, parent->board, SIZE * SIZE * sizeof(int));

    /* Swap cells */
    swap(&child->board[y_free][x_free], &child->board[y_swap][x_swap]);

    child->x = x_swap;
    child->y = y_swap;
    child->parent = parent_index;
    child->step = parent->step + 1;
}

/* TODO is this the best way to check for repetition */
/* TODO restructure as binary tree? */
int checkUnique(queue_t* queue, int grid[SIZE][SIZE]) {
    size_t i;
    for (i = 0; i <= queue->index; i++) {
        if (compareBoards(queue->children[i].board, grid)) {
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
    size_t index = ++queue->index;

    memcpy(&queue->children[index], grid, sizeof(grid_t));
}

/* DEPRECATED */
void buildChild(queue_t* queue, size_t parent, size_t x_swap, size_t y_swap) {
    size_t i;
    size_t index = ++queue->index;
    size_t x_zero = queue->children[parent].x;
    size_t y_zero = queue->children[parent].y;

    int(*child_board)[SIZE] = queue->children[index].board;
    int(*parent_board)[SIZE] = queue->children[parent].board;

    memcpy(child_board, parent_board, SIZE * SIZE * sizeof(int));
    swap(&child_board[y_zero][x_zero], &child_board[y_swap][x_swap]);

    for (i = 0; i < queue->index; i++) {
        if (compareBoards(child_board, queue->children[i].board)) {
            queue->index--;
            return;
        }
    }
    queue->children[index].x = x_swap;
    queue->children[index].y = y_swap;
    queue->children[index].parent = parent;
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
    loadBoard(queue->children[index].board, s);

    findEmptyCell(&queue->children[index]);
    queue->children[index].step = 0;
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
    assert(test_queue.children[test_queue.index].board[0][0] == 1);
    assert(test_queue.children[test_queue.index].board[2][2] == 8);
    assert(test_queue.children[test_queue.index].x == 1);
    assert(test_queue.children[test_queue.index].y == 1);

    printBoard(test_queue.children[0].board);

    /* Check that single child is generated properly */
    buildChild(&test_queue, 0, 1, 2);
    assert(test_queue.index == 1);
    assert(test_queue.children[test_queue.index].board[0][0] == 1);
    assert(test_queue.children[test_queue.index].board[2][2] == 8);
    assert(test_queue.children[test_queue.index].x == 1);
    assert(test_queue.children[test_queue.index].y == 2);
    assert(test_queue.children[test_queue.index].parent == 0);

    printBoard(test_queue.children[1].board);

    /* Check that whole set of children are generated properly */
    initQueue(&test_queue, "1234 5678");
    createChildren(&test_queue, 0);

    /* TODO more explicit testing here. On visual inspection it work */
    printBoard(test_queue.children[0].board);
    printBoard(test_queue.children[1].board);
    printBoard(test_queue.children[2].board);
    printBoard(test_queue.children[3].board);
    printBoard(test_queue.children[4].board);

    /* Testing of board duplication avoidance */
    /* TODO more explicit testing required */
    createChildren(&test_queue, 0);
    for (i = 0; i <= test_queue.index; i++) {
        printf("Board: %li\n", i);
        assert(compareBoards(test_queue.children[0].board, test_queue.children[i + 1].board) == 0);
        printBoard(test_queue.children[i].board);
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
    createChild(&test_queue.children[0], &test_board, 1, 1 + 1, 0);
    assert(test_board.x == 1 && test_board.y == 2);
    printBoard(test_board.board);

    solvePuzzle(&test_queue, "1234 5678");
}
