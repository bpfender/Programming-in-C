#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 3
/* http://w01fe.com/blog/2009/01/the-hardest-eight-puzzle-instances-take-31-moves-to-solve/ */
#define MAX_STEPS 32

typedef enum bool { false = 0,
                    true = 1 } bool;

/* Encodes direction that tiles on the board are shifted 
 */
typedef enum swap_t { UP,
                      DOWN,
                      LEFT,
                      RIGHT } swap_t;

/* Stores 8tile grid, location of free tile, the parent node and the next node
 * in the queue
 */
typedef struct node_t {
    int grid[SIZE][SIZE];
    int x;
    int y;
    struct node_t* parent;
    struct node_t* next;
} node_t;

/* Stores pointers to start, current and end node of node_t elements 
 */
typedef struct queue_t {
    node_t* start;
    node_t* curr;
    node_t* end;
} queue_t;

/* Stores list of solution top, and variable for number
 * of top 
 */
typedef struct stack_t {
    node_t* node[MAX_STEPS];
    unsigned int top;
} stack_t;

/* ------ SOLVER FUNCTIONS ------ */
void solve8Tile(queue_t* queue, char* s);
bool expandNode(queue_t* queue);
bool shiftTile(swap_t dir, queue_t* queue);

bool checkTarget(int grid[SIZE][SIZE]);
bool checkUnique(queue_t* queue, int grid[SIZE][SIZE]);
bool compareBoards(int grid1[SIZE][SIZE], int grid2[SIZE][SIZE]);

/* ------ QUEUE FUNCTIONS  ------ */
node_t* getNextNode(queue_t* queue);
void enqueue(queue_t* queue, node_t* grid);
void initQueue(queue_t* queue, char* s);
void loadBoard(int grid[SIZE][SIZE], char* s);
void findFreeTile(node_t* node);
void unloadQueue(node_t* node);

/* ------ STACK FUNCTIONS ------- */
void initStack(stack_t* solution);
void push(stack_t* solution, node_t* grid);
node_t* pop(stack_t* solution);
node_t* peek(stack_t* solution);

/* ----- UTILITY & INPUT/OUTPUT FUNCTIONS ------ */
void loadSolution(queue_t* queue, stack_t* solution);
void printSolution(stack_t* solution);
void printBoard(int grid[SIZE][SIZE]);
bool checkInputString(char* s);
bool isSolvable(char* s);
void swap(int* n1, int* n2);

/* ------ TESTING -------- */
void test(void);

int main(int argc, char* argv[]) {
    queue_t queue;
    stack_t solution;
    /*test();*/

    if (argc != 2) {
        fprintf(stderr,
                "ERROR: Incorrect usage, try e.g. %s \"12345 678\"\n", argv[0]);
        return 1;
    }
    if (!checkInputString(argv[1])) {
        fprintf(stderr,
                "INVALID. Please ensure string is: \n"
                "   - 3x3 grid as 9 characters\n"
                "   - Free tile denoted with space\n"
                "   - Unique tile values\n");
        return 1;
    }

    if (!isSolvable(argv[1])) {
        printf("This 8-tile board cannot be solved...\n");
        return 0;
    }

    printf("Solving puzzle. Please wait...\n");
    solve8Tile(&queue, argv[1]);

    loadSolution(&queue, &solution);
    printf("\nPuzzle Solved in %i steps:\n\n", solution.top - 1);

    printSolution(&solution);

    unloadQueue(queue.start);
    return 0;
}

/* ------ SOLVER FUNCTIONS ------ */
/* Solver initailises the queue with the starting grid and then expand nodes
 * onto the queue until a solution is found. Assumes a valid string. Uses
 * breadth first search. When solution is found, last element is loaded onto
 * queue, where it can then be read when loading the solution.
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
    node_t* parent = queue->curr;

    int x = parent->x;
    int y = parent->y;
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
    /* Moving to next list element is effectively dequeuing the current node,
       without having to move it somewhere else for later duplicate checking */
    queue->curr = parent->next;
    return false;
}

/* Generates next board state based on direction of shift. Shift direction 
 * refers to direction of tile being moved into the free space. Function assumes
 * that a valid shiftdirection is given. Copies board into queue, but queue
 * current index is only incremented if it's a valid board. This avoid copying
 * to a tmp and then copying to the queue
 */
bool shiftTile(swap_t dir, queue_t* queue) {
    node_t* child = getNextNode(queue);
    node_t* parent = queue->curr;

    /* Conditional evaluation will determine swap coordinates. "dir" will only
     * ever be one of these values, and as such the shift can only go, UP,
     * DOWN, LEFT or RIGHT
     */
    int x1 = parent->x;
    int y1 = parent->y;
    int x2 = x1 + (dir == LEFT) - (dir == RIGHT);
    int y2 = y1 + (dir == UP) - (dir == DOWN);

    memcpy(child->grid, parent->grid, SIZE * SIZE * sizeof(int));
    swap(&child->grid[y1][x1], &child->grid[y2][x2]);

    child->x = x2;
    child->y = y2;
    child->parent = parent;
    child->next = NULL;

    /* Child is only added to queue if this node has not been visited before */
    if (checkTarget(child->grid)) {
        enqueue(queue, child);
        return true;
    } else if (checkUnique(queue, child->grid)) {
        enqueue(queue, child);
    }

    return false;
}

/* This just calls the compare boards function on the target solution. Has been
 * defined as static so that it doesn't keep on being created and destroyed on
 * function call. Not sure if this is good style
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
     * end->next, might already have a node so need to check against it
     */
    while (node != queue->end->next) {
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
/* Returns a node which child can be written into. Is immediately appended to
 * end of queue, but queue end pointer isn't updated unless element ends up
 * being valid. This avoids having to malloc two nodes (tmp and queue) each
 * time a node is exanded
 */
node_t* getNextNode(queue_t* queue) {
    node_t* end = queue->end;
    node_t* tmp;
    /* Adds node to end of list which serves as temporary node. If it already 
       exists doesn't have to be malloced again. The end pointer in queue is not 
       updated until an element is "officially" added to the queue (Dan said this
       was nicer than using a tmp node) */
    if (end->next == NULL) {
        tmp = (node_t*)malloc(sizeof(node_t));
        if (tmp == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            exit(EXIT_FAILURE);
        }
        end->next = tmp;
    } else {
        tmp = end->next;
    }
    return tmp;
}

/* Mainly defined for more formal abstraction of adding to the queue 
 */
void enqueue(queue_t* queue, node_t* node) {
    /* End node is only updated if element is enqueued */
    queue->end = node;
}

/* Initiliase a queue by looading the starting board and setting pointers.
 * Expects a pointer to a queue struct, and valid string input checked by 
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

/* Unloads all malloced nodes. Because everything is held in the queue, it can
 * simply traverse the "next" pointer to unload all the nodes
 */
void unloadQueue(node_t* node) {
    if (node->next != NULL) {
        unloadQueue(node->next);
    }
    free(node);
}

/* ------ STACK FUNCTIONS ------ */
/* Stack has been implemented on basis of FIFO as list will be read in reverse
 * from queue once solution is found
 */
void initStack(stack_t* solution) {
    solution->top = 0;
}

/* Pushes pointer to grid_t* onto node. Stack shouldn't overflow as 8tile has
 * maximum number of steps, defined by MAX_STEPS.
 */
void push(stack_t* solution, node_t* grid) {
    if (solution->top > MAX_STEPS) {
        fprintf(stderr, "Stack overflow...\n");
        exit(EXIT_FAILURE);
    }
    solution->node[solution->top++] = grid;
}

/* Returns next node from node or NULL if node is empty */
node_t* pop(stack_t* solution) {
    if (solution->top <= 0) {
        return NULL;
    }
    return solution->node[--solution->top];
}

/* Look at top node without "removing it from stack" */
node_t* peek(stack_t* solution) {
    if (solution->top <= 0) {
        return NULL;
    }
    return solution->node[solution->top - 1];
}

/* ------ UTILITY & INPUT/OUTPUT FUNCTIONS ------ */
/* Loads solution by going through parent nodes back to start grid. Queue->end
 * will contain target grid, and chasing parent pointers goes all the way back
 * to starting grid. Solution needs to be loaded as nodes only have reference to
 * parents and not children. If this isn't dont, solution could only be read in
 * reverse
 */
void loadSolution(queue_t* queue, stack_t* solution) {
    node_t* node = queue->end;
    initStack(solution);

    while (node != NULL) {
        push(solution, node);
        node = node->parent;
    }
}

void printSolution(stack_t* solution) {
    unsigned int step = 1;
    node_t* node;

    node = pop(solution);
    printf("Starting board:\n");
    printBoard(node->grid);

    while ((node = pop(solution))) {
        printf("Step %i\n\n", step);
        printBoard(node->grid);
        step++;
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

/* Checks that a valid string has been inputted 
 */
bool checkInputString(char* s) {
    int i;
    int count[SIZE * SIZE] = {0};
    bool valid = true;

    if ((strlen(s)) != SIZE * SIZE) {
        valid = false;
    }

    /* Check for invalid chars and check valid ones are unique */
    for (i = 0; i < SIZE * SIZE && valid == true; i++) {
        if (s[i] == ' ') {
            count[0]++;
        } else if ('1' <= s[i] && s[i] <= '8') {
            count[s[i] - '0']++;
        } else {
            valid = false;
        }
    }
    for (i = 0; i < SIZE * SIZE && valid == true; i++) {
        if (count[i] > 1) {
            valid = false;
        }
    }

    return valid;
}

/* Reference: https://www.geeksforgeeks.org/check-instance-8-puzzle-solvable/ 
 * Checks whether the input string is acutally solvable before attempting to 
 * find a solution
 */
bool isSolvable(char* s) {
    int i, j;
    int inversions = 0;
    int grid[SIZE][SIZE];

    loadBoard(grid, s);

    for (i = 0; i < SIZE * SIZE - 1; i++) {
        for (j = i + 1; j < SIZE * SIZE; j++) {
            /* First && comparison eliminates 0 from inversion count, second
               size comp counts number of inversions */
            if (grid[i / 3][i % 3] && grid[j / 3][j % 3] &&
                grid[i / 3][i % 3] > grid[j / 3][j % 3]) {
                inversions++;
            }
        }
    }
    return inversions % 2 == 0;
}

void swap(int* n1, int* n2) {
    int tmp = *n1;
    *n1 = *n2;
    *n2 = tmp;
}

/* ------ TESTING ------- */
/* Testing is same protocol as used for 72.c, though modified to examine
 * pointers rather than array indexes
 */
void test(void) {
    long i, j;
    static queue_t test_queue;
    node_t test_node;
    node_t* test_node_ptr;
    int test_grid[SIZE][SIZE], test_grid_2[SIZE][SIZE];
    stack_t test_solution;

    /* TESTING loadBoard() function */
    loadBoard(test_grid, " 12345678");
    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            assert(test_grid[i][j] == (int)(SIZE * i + j));
        }
    }

    loadBoard(test_grid,
              "123"
              "4 5"
              "678");
    assert(test_grid[0][0] == 1);
    assert(test_grid[1][1] == 0);
    assert(test_grid[2][2] == 8);

    /* TESTING swap() function */
    swap(&test_grid[0][0], &test_grid[2][2]);
    swap(&test_grid[1][0], &test_grid[2][0]);
    swap(&test_grid[1][1], &test_grid[1][2]);
    assert(test_grid[0][0] == 8);
    assert(test_grid[2][2] == 1);
    assert(test_grid[1][0] == 6);
    assert(test_grid[2][0] == 4);
    assert(test_grid[1][1] == 5);
    assert(test_grid[1][2] == 0);

    /* TESTING findFreeTile() function  */
    loadBoard(test_grid, " 23415678");
    memcpy(test_node.grid, test_grid, SIZE * SIZE * sizeof(int));
    findFreeTile(&test_node);
    assert(test_node.x == 0);
    assert(test_node.y == 0);

    loadBoard(test_grid, "14567 823");
    memcpy(test_node.grid, test_grid, SIZE * SIZE * sizeof(int));
    findFreeTile(&test_node);
    assert(test_node.x == 2);
    assert(test_node.y == 1);

    /* TESTING compareBoard() and checkTarget() */
    loadBoard(test_grid, "123456 78");
    loadBoard(test_grid_2, "123456 78");
    assert(compareBoards(test_grid, test_grid_2) == true);

    loadBoard(test_grid_2, "12 563478");
    assert(compareBoards(test_grid, test_grid_2) == false);

    loadBoard(test_grid, "12345678 ");
    assert(checkTarget(test_grid) == true);
    assert(checkTarget(test_grid_2) == false);

    /* TESTING initQueue() function */
    initQueue(&test_queue, "14567 823");
    assert(test_queue.end == test_queue.curr);
    assert(test_queue.curr == test_queue.start);
    assert(test_queue.curr->parent == NULL);
    assert(test_queue.curr->next == NULL);
    assert(test_queue.curr->grid[0][0] == 1);
    assert(test_queue.curr->grid[2][2] == 3);
    assert(test_queue.curr->x == 2);
    assert(test_queue.curr->y == 1);

    /* TESTING getNextNode() function */
    /* getNextNode called now should return a new pointer */
    test_node_ptr = getNextNode(&test_queue);
    assert(test_node_ptr != NULL);
    free(test_node_ptr);

    /* Set end->next to dummy value, as though node has been allocated.
     * getNextNode should return this value;
     */
    test_queue.end->next = (node_t*)5;
    test_node_ptr = getNextNode(&test_queue);
    assert(test_node_ptr == test_queue.end->next);

    /* Reset ptr in queue */
    test_queue.end->next = NULL;

    /* TESTING shiftTile() based on init from above */
    shiftTile(UP, &test_queue);
    loadBoard(test_grid, "14567382 ");
    assert(compareBoards(test_queue.end->grid, test_grid) == true);
    assert(test_queue.end->parent == test_queue.curr);
    assert(test_queue.end->x == 2);
    assert(test_queue.end->y == 2);
    assert(test_queue.curr == test_queue.start);

    /* Pre-assign end->next (virtual temp) so that I can keep track of it below
     */
    test_queue.end->next = getNextNode(&test_queue);
    test_node_ptr = test_queue.end->next;

    shiftTile(DOWN, &test_queue);
    loadBoard(test_grid, "14 675823");
    assert(compareBoards(test_queue.end->grid, test_grid) == true);
    assert(test_queue.curr == test_queue.start);

    /* The fact that end pointer is previous end->next ptr indicates that 
     * enqueue() is working as expected 
     */
    assert(test_queue.end == test_node_ptr);

    /* TESTING checkUnique() function on two items loaded in queue */
    assert(checkUnique(&test_queue, test_grid) == false);
    loadBoard(test_grid, "12345 678");
    assert(checkUnique(&test_queue, test_grid) == true);

    /* Repeat UP shift. Item should not be enqeued because it's a duplicate, so
       end pounter should stay the same */
    shiftTile(UP, &test_queue);
    assert(test_queue.end == test_node_ptr);

    unloadQueue(test_queue.start);

    /* Reset queue and check that it recognises target reached correctly */
    initQueue(&test_queue, "1234567 8");
    assert(shiftTile(LEFT, &test_queue) == true);
    assert(checkTarget(test_queue.end->grid) == true);
    unloadQueue(test_queue.start);

    /* TESTING initQueue again expandNode()*/
    initQueue(&test_queue, "1234 5678");
    assert(test_queue.end == test_queue.curr);
    assert(test_queue.curr == test_queue.start);
    assert(test_queue.curr->grid[0][0] == 1);
    assert(test_queue.end->grid[2][2] == 8);
    assert(test_queue.start->x == 1);
    assert(test_queue.curr->y == 1);

    /* Assign next node so i can keep track of it. Unconventional but shouldn't
     * break things
     */
    test_queue.curr->next = test_node_ptr = getNextNode(&test_queue);
    expandNode(&test_queue);
    /* Having expanded the node, the next queue element should be expanded */
    assert(test_queue.curr == test_node_ptr);

    /* Left */
    loadBoard(test_grid, "12345 678");
    assert(compareBoards(test_queue.curr->grid, test_grid) == true);
    /* Right */
    loadBoard(test_grid, "123 45678");
    assert(compareBoards(test_queue.curr->next->grid, test_grid) == true);
    /* Up */
    loadBoard(test_grid, "1234756 8");
    assert(compareBoards(test_queue.curr->next->next->grid, test_grid) == true);
    /* Down */
    loadBoard(test_grid, "1 3425678");
    assert(compareBoards(test_queue.curr->next->next->next->grid, test_grid) == true);

    assert(test_queue.curr->x == 2);
    assert(test_queue.curr->y == 1);

    /* TESTING initStack() */
    initStack(&test_solution);
    assert(test_solution.top == 0);

    /* TESTING push() by taking elements loaded into queue above */
    push(&test_solution, test_queue.start);
    assert(test_solution.top == 1);
    push(&test_solution, test_queue.start->next);
    assert(test_solution.top == 2);
    push(&test_solution, test_queue.start->next->next);
    assert(test_solution.top == 3);
    push(&test_solution, test_queue.start->next->next->next);
    assert(test_solution.top == 4);

    loadBoard(test_grid, "1234 5678");
    assert(compareBoards(test_solution.node[0]->grid, test_grid) == true);
    loadBoard(test_grid, "12345 678");
    assert(compareBoards(test_solution.node[1]->grid, test_grid) == true);
    loadBoard(test_grid, "123 45678");
    assert(compareBoards(test_solution.node[2]->grid, test_grid) == true);
    loadBoard(test_grid, "1234756 8");
    assert(compareBoards(test_solution.node[3]->grid, test_grid) == true);

    /* TESTING pop() should just return grid loaded above */
    assert(compareBoards(pop(&test_solution)->grid, test_grid) == true);

    /* Pop all elements and check that pop returns Null when list is empty */
    pop(&test_solution);
    pop(&test_solution);
    pop(&test_solution);
    assert(pop(&test_solution) == NULL);

    /* TESTING loadSolution() */
    /* Should load element 4 in queue and it's parent only */
    loadSolution(&test_queue, &test_solution);
    loadBoard(test_grid, "1 3425678");
    assert(compareBoards(test_solution.node[0]->grid, test_grid) == true);

    loadBoard(test_grid, "1234 5678");
    assert(compareBoards(test_solution.node[1]->grid, test_grid) == true);

    /* TESTING checkInputString() */
    assert(checkInputString("12345 678") == true);
    assert(checkInputString("thrr 1234") == false);
    assert(checkInputString("123") == false);
    assert(checkInputString("112233  4") == false);
    assert(checkInputString("123456789") == false);

    /* TESTING isSolvable() */
    assert(isSolvable("1234567 8") == true);
    assert(isSolvable("182 43765") == true);
    assert(isSolvable("812 43765") == false);
    assert(isSolvable("12345687 ") == false);
}
