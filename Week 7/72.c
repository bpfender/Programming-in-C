#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*FIXME types */

#define SIZE 3
/* FIXME queue length? */
#define QUEUE 362880

typedef enum bool { false = 0,
                    true = 1 } bool;

/* Encodes direction that tiles on the board are shifted */
typedef enum swap_t { UP,
                      DOWN,
                      LEFT,
                      RIGHT } swap_t;

/* Stores 8tile grid, location of free tile, the parent node and the number of
   steps from the starting grid */
typedef struct grid_t {
    int grid[SIZE][SIZE];
    size_t x;
    size_t y;
    size_t parent;
    size_t step;
} grid_t;

/* Stores array of grid_t elements, with index of the current node and 
   last node added to the array */
typedef struct queue_t {
    grid_t node[QUEUE];
    size_t curr;
    size_t end;
} queue_t;

/* Stores dynamically allocated list of solution steps, and variable for number
   of steps */
typedef struct sol_t {
    grid_t** grid;
    int steps;
} sol_t;

/* ------ SOLVER FUNCTIONS ------ */
void solve8Tile(queue_t* queue, char* s);
bool expandNode(queue_t* queue);
bool shiftTile(swap_t dir, queue_t* queue);

bool checkTarget(int grid[SIZE][SIZE]);
bool checkUnique(queue_t* queue, int grid[SIZE][SIZE]);
bool compareBoards(int grid1[SIZE][SIZE], int grid2[SIZE][SIZE]);

/* ------ QUEUE FUNCTIONS ------ */
void enqueue(queue_t* queue, grid_t* grid);
void initQueue(queue_t* queue, char* s);
void loadBoard(int grid[SIZE][SIZE], char* s);
void findFreeTile(grid_t* grid);

/* ------ UTILITY & INPUT FUNCTIONS ------ */
void printSolution(queue_t* queue);
void printBoard(int grid[SIZE][SIZE]);
bool checkInputString(char* s);
bool isSolvable(char* s);
void swap(int* n1, int* n2);
void test(void);

/* REQUIRE LOAD SOLUTION */

int main(int argc, char* argv[]) {
    static queue_t queue;
    test();
    return 0;
    /* if (argc != 2) {
        fprintf(stderr,
                "ERROR: Incorrect usage, try e.g. %s \"12345 678\"\n", argv[0]);
        return 1;
    }

    if (!checkInputString(argv[1])) {
        return 1;
    }

    if (!isSolvable(argv[1])) {
        printf("This 8-tile board cannot be solved...\n");
        return 0;
    }

    printf("Solving puzzle... please wait...\n");
    solve8Tile(&queue, argv[1]);

    printf("\nPuzzle Solved in XXXXXX steps:\n");
    printSolution(&queue);

    return 0;*/
}

void printSolution(queue_t* queue) {
    size_t i;
    size_t list_index = queue->end;
    size_t len = queue->node[queue->end].step;

    grid_t* list = (grid_t*)malloc((len + 1) * sizeof(grid_t));

    i = len;
    while (queue->node[list_index].step != 0) {
        printf("%li\n", queue->node[list_index].step);
        list[i] = queue->node[list_index];
        list_index = queue->node[list_index].parent;
        i--;
    }
    list[0] = queue->node[0];

    for (i = 0; i <= len; i++) {
        printBoard(list[i].grid);
    }

    free(list);
}

/* ------- SOLVER FUNCTIONS ------ */
/* Solver initailises the queue with the starting grid and then expand nodes
 * onto the queue until a solution is found. Assumes a valid string.
 */
void solve8Tile(queue_t* queue, char* s) {
    initQueue(queue, s);
    while (!expandNode(queue)) {
    }
}

/* Calls function shiftTile to expand possible moves of current node. Will 
 * return true if the solution is found
 */
bool expandNode(queue_t* queue) {
    size_t parent = queue->curr;
    /* QUESTION is duplication of x call bad? c.f. shiftTile*/
    size_t x = queue->node[parent].x;
    size_t y = queue->node[parent].y;

    if (x < SIZE - 1) {
        if (shiftTile(LEFT, queue)) {
            return true;
        }
    }
    if (x > 0) {
        if (shiftTile(RIGHT, queue)) {
            return true;
        }
    }
    if (y < SIZE - 1) {
        if (shiftTile(UP, queue)) {
            return true;
        }
    }
    if (y > 0) {
        if (shiftTile(DOWN, queue)) {
            return true;
        }
    }

    /* Incrementing current index is effectively dequeuing the current node,
       without having to move it somewhere else for later duplicate checking */
    queue->curr++;
    return false;
}

/* Generates next board state based on direction of shift. Shift direction 
 * refers to direction of tile being moved into the free space. Function assumes
 * that a valid shiftdirection is given.
 */
bool shiftTile(swap_t dir, queue_t* queue) {
    /* QUESTION does it make sense to declare this as static given that it is 
       called again and again */
    grid_t tmp;
    grid_t* parent = &queue->node[queue->curr];

    /* Conditional evaluation will determine swap coordinates. "dir" will only
     * ever be one of these values, and as such the shift can only go, UP,
     * DOWN, LEFT or RIGHT
     */
    size_t x1 = parent->x;
    size_t y1 = parent->y;
    size_t x2 = x1 + (dir == LEFT) - (dir == RIGHT);
    size_t y2 = y1 + (dir == UP) - (dir == DOWN);

    /* Create child node in tmp struct */
    memcpy(tmp.grid, parent->grid, SIZE * SIZE * sizeof(int));
    swap(&tmp.grid[y1][x1], &tmp.grid[y2][x2]);
    tmp.x = x2;
    tmp.y = y2;
    tmp.parent = queue->curr;
    tmp.step = parent->step + 1;

    /* FIXME can this be more concise? */
    if (checkTarget(tmp.grid)) {
        enqueue(queue, &tmp);
        return true;
    } else if (checkUnique(queue, tmp.grid)) {
        enqueue(queue, &tmp);
    }

    return false;
}

/* QUESTION do i need a seperate function for this? */
bool checkTarget(int grid[SIZE][SIZE]) {
    /* QUESTION in termsof notation better to have a static? */
    static int target[SIZE][SIZE] = {{1, 2, 3},
                                     {4, 5, 6},
                                     {7, 8, 0}};

    return compareBoards(target, grid);
}

/* Iterates through complete list of explored nodes to check for duplicates
 */
bool checkUnique(queue_t* queue, int grid[SIZE][SIZE]) {
    size_t i;
    for (i = 0; i <= queue->end; i++) {
        if (compareBoards(queue->node[i].grid, grid)) {
            return false;
        }
    }
    return true;
}

/* Compares whether two 8-tile boards are the same. Have used memcmp() for
 * improved speed. Did compare against storing a value key for each grid and 
 * using that as the comparator, but the speed seemed basically the same. 
 */
bool compareBoards(int grid1[SIZE][SIZE], int grid2[SIZE][SIZE]) {
    /* QUESTION enum and return value? */
    return !memcmp(grid1, grid2, SIZE * SIZE * sizeof(int));
}

/* ------ QUEUE FUNCTIONS ------ */
/* Add grid_t to the end of the queue and increment end of queue index
 */
void enqueue(queue_t* queue, grid_t* grid) {
    /* Incrementing end keeps track of where to add future nodes */
    size_t end = ++queue->end;
    memcpy(&queue->node[end], grid, sizeof(grid_t));
}

/* Initiliase a queue by Loading the starting board and setting indeces to zero.
 * Expects a pointer to a queue, and valid string input checked by 
 * checkInputString()
 */
void initQueue(queue_t* queue, char* s) {
    size_t start = queue->end = queue->curr = 0;

    loadBoard(queue->node[start].grid, s);
    findFreeTile(&queue->node[start]);
    queue->node[start].step = 0;
    queue->node[start].parent = 0;
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
void findFreeTile(grid_t* grid) {
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

/* ------ UTILITY & INPUT FUNCTIONS ------ */

/* Print a single 8-tile board
 */
void printBoard(int grid[SIZE][SIZE]) {
    size_t i, j;
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
    size_t i;
    int inversions = 0;
    int grid[SIZE][SIZE];

    loadBoard(grid, s);

    for (i = 0; i < SIZE * SIZE - 1; i++) {
        if (*(s + i) && *(s + 1 + i) && *(s + i) > *(s + i + 1)) {
            inversions++;
        }
    }

    return inversions % 2 == 0;
}

/* Swap two values */
void swap(int* n1, int* n2) {
    int tmp = *n1;
    *n1 = *n2;
    *n2 = tmp;
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

    findFreeTile(&test_board);
    assert(test_board.x == 0);
    assert(test_board.y == 0);

    printBoard(test_board.grid);

    /* Check that queue initialisation works properly */
    initQueue(&test_queue, "1234 5678");
    assert(test_queue.end == 0);
    assert(test_queue.node[test_queue.end].grid[0][0] == 1);
    assert(test_queue.node[test_queue.end].grid[2][2] == 8);
    assert(test_queue.node[test_queue.end].x == 1);
    assert(test_queue.node[test_queue.end].y == 1);

    printBoard(test_queue.node[0].grid);

    /* Check that whole set of node are generated properly */
    initQueue(&test_queue, "1234 5678");
    expandNode(&test_queue);

    /* TODO more explicit testing here. On visual inspection it work */
    printBoard(test_queue.node[0].grid);
    printBoard(test_queue.node[1].grid);
    printBoard(test_queue.node[2].grid);
    printBoard(test_queue.node[3].grid);
    printBoard(test_queue.node[4].grid);

    /* Testing of grid duplication avoidance */
    /* TODO more explicit testing required */
    expandNode(&test_queue);
    for (i = 0; i <= test_queue.end; i++) {
        printf("Board: %li\n", i);
        assert(compareBoards(test_queue.node[0].grid, test_queue.node[i + 1].grid) == 0);
        printBoard(test_queue.node[i].grid);
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
    shiftTile(DOWN, &test_queue);
    assert(test_queue.node[1].x == 1 && test_queue.node[1].y == 0);
    printBoard(test_board.grid);

    printf("Solve START\n");
    solve8Tile(&test_queue, "12 345678");
    printSolution(&test_queue);
    /*

    assert(checkInputString("12345 678") == 1);
    assert(checkInputString("thrr 1234") == 0);
    assert(checkInputString("123") == 0);
    assert(checkInputString("112233  4") == 0);
    assert(checkInputString("1234567890 ") == 0);*/
}
