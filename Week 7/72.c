#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 3
/* 9 Factorial/2 as around half of boards are invalid*/
#define QUEUE 362880 / 2
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

/* Stores 8tile grid, location of free tile and the parent node 
 */
typedef struct grid_t {
    int grid[SIZE][SIZE];
    int x;
    int y;
    long parent;
} grid_t;

/* Stores array of grid_t elements, with index of the current node and 
 * last node added to the array. "long" types used to ensure that index
 * can go high enough for many iterations 
 */
typedef struct queue_t {
    grid_t node[QUEUE];
    long curr;
    long end;
} queue_t;

/* Stores list of solution steps, and variable for number of steps. Effectively
 * implemented as a stack.
 */
typedef struct stack_t {
    grid_t* stack[MAX_STEPS];
    unsigned int top;
} stack_t;

/* ------ SOLVER FUNCTIONS ------ */
void solve8Tile(queue_t* queue, char* s);
bool expandNode(queue_t* queue);
bool shiftTile(swap_t dir, queue_t* queue);

bool checkTarget(int grid[SIZE][SIZE]);
bool checkUnique(queue_t* queue, int grid[SIZE][SIZE]);
bool compareBoards(int grid1[SIZE][SIZE], int grid2[SIZE][SIZE]);

/* ------ QUEUE FUNCTIONS ------ */
void enqueue(queue_t* queue);
void initQueue(queue_t* queue, char* s);
void loadBoard(int grid[SIZE][SIZE], char* s);
void findFreeTile(grid_t* grid);

/* ------- STACK FUNCTIONS ------ */
void initStack(stack_t* solution);
void push(stack_t* solution, grid_t* grid);
grid_t* pop(stack_t* solution);

/* ------ UTILITY & INPUT/OUTPUT FUNCTIONS ------ */
void loadSolution(queue_t* queue, stack_t* solution);
void printSolution(stack_t* solution);
void printBoard(int grid[SIZE][SIZE]);
bool checkInputString(char* s);
bool isSolvable(char* s);
void swap(int* n1, int* n2);
void test(void);

int main(int argc, char* argv[]) {
    static queue_t queue;
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

    return 0;
}

/* ------- SOLVER FUNCTIONS ------ */
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
    long parent = queue->curr;
    /* QUESTION is duplication of x call bad? c.f. shiftTile. Could just pass
       as function variables */
    int x = queue->node[parent].x;
    int y = queue->node[parent].y;

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
 * that a valid shiftdirection is given. Copies board into queue, but queue
 * current index is only incremented if it's a valid board. This avoid copying
 * to a tmp and then copying to the queue
 */
bool shiftTile(swap_t dir, queue_t* queue) {
    grid_t* parent = &queue->node[queue->curr];

    /* Child is put into "temporary" index at end of queue. End index is only
       incremented and "added" properly, when enqueue() is called */
    grid_t* child = &queue->node[queue->end + 1];

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
    child->parent = queue->curr;

    /* Child is only added to queue if this node has not been visited before */
    if (checkTarget(child->grid)) {
        enqueue(queue);
        return true;
    } else if (checkUnique(queue, child->grid)) {
        enqueue(queue);
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
    long i;
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
    return !memcmp(grid1, grid2, SIZE * SIZE * sizeof(int));
}

/* ------ QUEUE FUNCTIONS ------ */
/* Add grid_t to the end of the queue and increment end of queue index
 */
void enqueue(queue_t* queue) {
    /* Incrementing end keeps track of where to add future nodes */
    queue->end++;
}

/* Initiliase a queue by Loading the starting board and setting indeces to zero.
 * Expects a pointer to a queue struct, and valid string input checked by 
 * checkInputString()
 */
void initQueue(queue_t* queue, char* s) {
    long start = queue->end = queue->curr = 0;

    loadBoard(queue->node[start].grid, s);
    findFreeTile(&queue->node[start]);
    /* Set start to -1 for identification later */
    queue->node[start].parent = -1;
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
void findFreeTile(grid_t* node) {
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

/* ------ STACK FUNCTIONS ------ */
/* Stack has been implemented on basis of FIFO as list will be read in reverse
 * from queue once solution is found
 */
void initStack(stack_t* solution) {
    solution->top = 0;
}

/* Pushes pointer to grid_t* onto stack. Stack shouldn't overflow as 8tile has
 * maximum number of steps.
 */
void push(stack_t* solution, grid_t* grid) {
    if (solution->top > MAX_STEPS) {
        fprintf(stderr, "Stack overflow...\n");
        exit(EXIT_FAILURE);
    }
    solution->stack[solution->top++] = grid;
}

/* Returns next node from stack or NULL if stack is empty */
grid_t* pop(stack_t* solution) {
    if (solution->top <= 0) {
        return NULL;
    }
    return solution->stack[--solution->top];
}

/* ------ UTILITY & INPUT/OUTPUT FUNCTIONS ------ */
/* Loads solution by going through parent nodes back to start grid. Queue->end
 * will contain target grid, and chasing parent pointers goes all the way back
 * to starting grid. Solution needs to be loaded as nodes only have reference to
 * parents and not children. If this isn't dont, solution could only be read in
 * reverse
 */
void loadSolution(queue_t* queue, stack_t* solution) {
    long index = queue->end;

    initStack(solution);
    while (index != -1) {
        push(solution, &queue->node[index]);
        index = queue->node[index].parent;
    }
}

void printSolution(stack_t* solution) {
    unsigned int step = 1;
    grid_t* node;

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
    size_t i, j;
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

/* Swap two values */
void swap(int* n1, int* n2) {
    int tmp = *n1;
    *n1 = *n2;
    *n2 = tmp;
}

void test(void) {
    long i, j;
    static queue_t test_queue;
    grid_t test_node;
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
    assert(test_queue.end == 0);
    assert(test_queue.curr == 0);
    assert(test_queue.node[test_queue.end].parent == -1);
    assert(test_queue.node[test_queue.end].grid[0][0] == 1);
    assert(test_queue.node[test_queue.end].grid[2][2] == 3);
    assert(test_queue.node[test_queue.end].x == 2);
    assert(test_queue.node[test_queue.end].y == 1);

    /* TESTING shiftTile() based on init from above */
    shiftTile(UP, &test_queue);
    loadBoard(test_grid, "14567382 ");
    assert(compareBoards(test_queue.node[1].grid, test_grid) == true);
    assert(test_queue.node[1].parent == 0);
    assert(test_queue.node[1].x == 2);
    assert(test_queue.node[1].y == 2);
    assert(test_queue.curr == 0);
    assert(test_queue.end == 1);

    shiftTile(DOWN, &test_queue);
    loadBoard(test_grid, "14 675823");
    assert(compareBoards(test_queue.node[2].grid, test_grid) == true);
    assert(test_queue.curr == 0);
    /* The fact that the end index increments, indicates that enqueue() is 
       works as expected */
    assert(test_queue.end == 2);

    /* TESTING checkUnique() function on two items loaded in queue */
    assert(checkUnique(&test_queue, test_grid) == false);

    loadBoard(test_grid, "12345 678");
    assert(checkUnique(&test_queue, test_grid) == true);

    /* Repeat UP shift. Item should not be enqeued because it's a duplicate, so
       end index should stay the same */
    shiftTile(UP, &test_queue);
    assert(test_queue.end == 2);

    /* Reset queue and check that it recognises target reached correctly */
    initQueue(&test_queue, "1234567 8");
    assert(shiftTile(LEFT, &test_queue) == true);
    assert(test_queue.end == 1);

    /* TESTING initQueue again expandNode()*/
    initQueue(&test_queue, "1234 5678");
    assert(test_queue.end == 0);
    assert(test_queue.curr == 0);
    assert(test_queue.node[test_queue.end].grid[0][0] == 1);
    assert(test_queue.node[test_queue.end].grid[2][2] == 8);
    assert(test_queue.node[test_queue.end].x == 1);
    assert(test_queue.node[test_queue.end].y == 1);

    expandNode(&test_queue);
    assert(test_queue.end == 4);
    /* Having expanded the node, the next queue element should be expanded */
    assert(test_queue.curr == 1);

    /* Left */
    loadBoard(test_grid, "12345 678");
    assert(compareBoards(test_queue.node[1].grid, test_grid) == true);
    /* Right */
    loadBoard(test_grid, "123 45678");
    assert(compareBoards(test_queue.node[2].grid, test_grid) == true);
    /* Up */
    loadBoard(test_grid, "1234756 8");
    assert(compareBoards(test_queue.node[3].grid, test_grid) == true);
    /* Down */
    loadBoard(test_grid, "1 3425678");
    assert(compareBoards(test_queue.node[4].grid, test_grid) == true);

    assert(test_queue.node[1].x == 2);
    assert(test_queue.node[1].y == 1);
    assert(test_queue.node[4].x == 1);
    assert(test_queue.node[4].y == 0);
    assert(test_queue.node[2].parent == 0);
    assert(test_queue.curr == 1);
    assert(test_queue.end == 4);

    /* TESTING initStack() */
    initStack(&test_solution);
    assert(test_solution.top == 0);

    /* TESTING push() by taking elements loaded into queue above */
    push(&test_solution, &test_queue.node[0]);
    assert(test_solution.top == 1);
    push(&test_solution, &test_queue.node[1]);
    assert(test_solution.top == 2);
    push(&test_solution, &test_queue.node[2]);
    assert(test_solution.top == 3);
    push(&test_solution, &test_queue.node[3]);
    assert(test_solution.top == 4);

    loadBoard(test_grid, "1234 5678");
    assert(compareBoards(test_solution.stack[0]->grid, test_grid) == true);
    loadBoard(test_grid, "12345 678");
    assert(compareBoards(test_solution.stack[1]->grid, test_grid) == true);
    loadBoard(test_grid, "123 45678");
    assert(compareBoards(test_solution.stack[2]->grid, test_grid) == true);
    loadBoard(test_grid, "1234756 8");
    assert(compareBoards(test_solution.stack[3]->grid, test_grid) == true);

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
    assert(compareBoards(test_solution.stack[0]->grid, test_grid) == true);

    loadBoard(test_grid, "1234 5678");
    assert(compareBoards(test_solution.stack[1]->grid, test_grid) == true);

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
