#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 3
/* http://w01fe.com/blog/2009/01/the-hardest-eight-puzzle-instances-take-31-moves-to-solve/ */
#define MAX_STEPS 31

typedef enum bool { false = 0,
                    true = 1 } bool;

/* Encodes direction that tiles on the board are shifted */
typedef enum swap_t { UP,
                      DOWN,
                      LEFT,
                      RIGHT } swap_t;

/* Stores 8tile grid, location of free tile, the parent node, the next node in
   the queue and the number of steps from the starting grid */
typedef struct node_t {
    int grid[SIZE][SIZE];
    int x;
    int y;
    unsigned int step;
    struct node_t* parent;
    struct node_t* next;
} node_t;

/* Stores pointers to start, current and end node of node_t elements */
typedef struct queue_t {
    node_t* start;
    node_t* curr;
    node_t* end;
} queue_t;

/* Stores list of solution steps, and variable for number
   of steps */
typedef struct sol_t {
    node_t* node[MAX_STEPS];
    unsigned int steps;
} sol_t;

/* ------ SOLVER FUNCTIONS ------ */
void solve8Tile(queue_t* queue, char* s);
int expandNode(queue_t* queue);
bool shiftTile(swap_t dir, queue_t* queue);
bool checkTarget(int grid[SIZE][SIZE]);
bool checkUnique(queue_t* queue, int grid[SIZE][SIZE]);
bool compareBoards(int grid1[SIZE][SIZE], int grid2[SIZE][SIZE]);

/* ------ QUEUE FUNCTIONS  ------ */
void enqueue(queue_t* queue, node_t* grid);
void initQueue(queue_t* queue, char* s);
void loadBoard(int grid[SIZE][SIZE], char* s);
void findFreeTile(node_t* grid);
void unloadQueue(node_t* node);

void swap(int* n1, int* n2);

/* ----- UTILITY & INPUT FUNCTIONS ------ */
void loadSolution(queue_t* queue, sol_t* solution);
void printSolution(sol_t* solution);
void printBoard(int grid[SIZE][SIZE]);
bool checkInputString(char* s);
bool isSolvable(char* s);
void swap(int* n1, int* n2);
void test(void);

int main(int argc, char* argv[]) {
    queue_t queue;
    sol_t solution;

    /*test()*/;
    solve8Tile(&queue, "123456 78");

    if (argc != 2) {
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

    printf("Solving puzzle. Please wait...\n");
    solve8Tile(&queue, argv[1]);

    loadSolution(&queue, &solution);
    printf("\nPuzzle Solved in %i steps:\n\n", solution.steps);
    printSolution(&solution);

    unloadQueue(queue.start);
    return 0;
}

/* ------ SOLVER FUNCTIONS ------ */
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
int expandNode(queue_t* queue) {
    node_t* parent = queue->curr;

    int x = parent->x;
    int y = parent->y;
    printBoard(parent->grid);
    if (x < SIZE - 1) {
        if (shiftTile(LEFT, queue)) {
            return 1;
        }
    }
    if (x > 0) {
        if (shiftTile(RIGHT, queue)) {
            return 1;
        }
    }
    if (y < SIZE - 1) {
        if (shiftTile(UP, queue)) {
            return 1;
        }
    }
    if (y > 0) {
        if (shiftTile(DOWN, queue)) {
            return 1;
        }
    }
    /* Moving to next list element is effectively dequeuing the current node,
       without having to move it somewhere else for later duplicate checking */
    queue->curr = parent->next;
    return 0;
}

/* Generates next board state based on direction of shift. Shift direction 
 * refers to direction of tile being moved into the free space. Function assumes
 * that a valid shiftdirection is given. Copies board into queue, but queue
 * current index is only incremented if it's a valid board. This avoid copying
 * to a tmp and then copying to the queue
 */
bool shiftTile(swap_t dir, queue_t* queue) {
    node_t* child;
    node_t* parent = queue->curr;
    node_t* end = queue->end;

    /* Conditional evaluation will determine swap coordinates. "dir" will only
     * ever be one of these values, and as such the shift can only go, UP,
     * DOWN, LEFT or RIGHT
     */
    int x1 = parent->x;
    int y1 = parent->y;
    int x2 = x1 + (dir == LEFT) - (dir == RIGHT);
    int y2 = y1 + (dir == UP) - (dir == DOWN);

    /* FIXME functionise */
    /* Adds node to end of list which serves as temporary node. If it already 
       exists doesn't have to be malloced again. The end pointer in queue is not 
       updated until an element is "officially" added to the queue (Dan said this
       was nicer than using a tmp node) */
    if (end->next == NULL) {
        child = (node_t*)malloc(sizeof(node_t));
        if (child == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            unloadQueue(queue->start);
            exit(EXIT_FAILURE);
        }
        end->next = child;
    } else {
        child = end->next;
    }

    memcpy(child->grid, parent->grid, SIZE * SIZE * sizeof(int));
    swap(&child->grid[y1][x1], &child->grid[y2][x2]);

    child->x = x2;
    child->y = y2;
    child->step = parent->step + 1;
    child->parent = parent;
    child->next = NULL;

    if (checkTarget(child->grid)) {
        enqueue(queue, child);
        return true;
    } else if (checkUnique(queue, child->grid)) {
        enqueue(queue, child);
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

/* Iterates through complete list of explored nodes to check for duplicates
 */
bool checkUnique(queue_t* queue, int grid[SIZE][SIZE]) {
    node_t* node = queue->start;

    /* Check up to current end node (not including tmp node added to end of list)
     */
    while (node != queue->end) {
        if (compareBoards(node->grid, grid)) {
            return false;
        }
        node = node->next;
    }
    return true;
}

/* Compares whether two 8-tile boards are the same. Have used memcmp() for
 * improved speed. Did compare against storing a value key for each grid and 
 * using that as the comparator, but the speed seemed basically the same. 
 */
bool compareBoards(int grid1[SIZE][SIZE], int grid2[SIZE][SIZE]) {
    return !(memcmp(grid1, grid2, SIZE * SIZE * sizeof(int)));
}

/* ------ QUEUE FUNCTIONS ------ */
/* Add node_t to the end of the queue and update end pointer;
 */
void enqueue(queue_t* queue, node_t* node) {
    /* End node is only updated if element is enqueued */
    queue->end = node;
}

/* Initiliase a queue by looading the starting board and setting pointers.
 * Expects a pointer to a queue, and valid string input checked by 
 * checkInputString()
 */
void initQueue(queue_t* queue, char* s) {
    node_t* ptr = (node_t*)malloc(sizeof(node_t));
    if (ptr == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    queue->start = queue->curr = queue->end = ptr;

    loadBoard(queue->start->grid, s);
    findFreeTile(queue->start);

    queue->start->step = 0;
    queue->start->parent = NULL;
    queue->start->next = NULL;
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

void unloadQueue(node_t* node) {
    if (node->next != NULL) {
        unloadQueue(node->next);
    }
    free(node);
}

/* ------ UTILITY & INPUT FUNCTIONS ------ */
/* Loads solution by going through parent nodes back to start grid
 */
void loadSolution(queue_t* queue, sol_t* solution) {
    unsigned int i;
    node_t* node = queue->end;

    /* FIXME do I need the step counter? */
    i = solution->steps = node->step;

    while (node != NULL) {
        solution->node[i] = node;
        node = node->parent;
        i--;
    }
}

void printSolution(sol_t* solution) {
    unsigned int i;

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

    findFreeTile(&test_board);
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
    expandNode(&test_queue);

    /* TODO more explicit testing here. On visual inspection it work */

    /* Testing of grid duplication avoidance */
    /* TODO more explicit testing required */
    expandNode(&test_queue);

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
    shiftTile(DOWN, &test_queue);
    assert(test_queue.start->next->x == 1 && test_queue.start->next->y == 0);
    printBoard(test_board.grid);

    unloadQueue(test_queue.start);

    printf("Solve START\n");
    solve8Tile(&test_queue, "1234 5678");
    unloadQueue(test_queue.start);
}
