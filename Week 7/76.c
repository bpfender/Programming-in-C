#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "neillsdl2.h"

/* SDL constants to determine tile size, delays and positions */
#define BORDER 5
#define TILE_SIZE (WHEIGHT - BORDER * 2) / 3
#define RADIUS TILE_SIZE / 2 - TILE_SIZE / 4
#define RADIUS_FREE TILE_SIZE / 3
#define OFFSET BORDER
#define CHAR_X_OFFSET TILE_SIZE / 2 - FNTHEIGHT / 2
#define CHAR_Y_OFFSET TILE_SIZE / 2 - FNTWIDTH / 2

#define START_DELAY 500
#define STEP_DELAY 75
#define SLIDE_DELAY 1

#define SDL_8BITCOLOUR 256

/* Hash table size doesn't need to be huge for good performance. 500 seems to 
 * be a fairly good compromise */
#define HASH_TABLE 500
/* djb2 Hash constants, defined based on reference below
 * http://www.cse.yorku.ca/~oz/hash.html
 */
#define HASH 5381
#define MAGIC 33

/* Priority queue definitions */
#define QUEUE_SIZE 500
#define BUFF_FACTOR 2
#define SIZE 3

/* http://w01fe.com/blog/2009/01/the-hardest-eight-puzzle-instances-take-31-moves-to-solve/ */
#define MAX_STEPS 32

/* Enum to allow selection of drawing colour */
typedef enum col_t { WHITE,
                     GRAY,
                     DARK_GRAY,
                     BLACK,
                     RED,
                     ORANGE,
                     GREEN
} col_t;

typedef enum bool { false = 0,
                    true = 1 } bool;

/* Encodes direction that tiles on the board are shifted */
typedef enum swap_t { UP,
                      DOWN,
                      LEFT,
                      RIGHT } swap_t;

/* Stores 8tile grid, location of free tile, the parent node and priority 
 * value
 */
typedef struct node_t {
    int grid[SIZE][SIZE];
    int f, step;
    int x, y;
    struct node_t* parent;
    unsigned long hash;
} node_t;

/* Queue struct to be used for array implementation of binary heap priority
 * queue
 * https://bradfieldcs.com/algos/trees/priority-queues-with-binary-heaps/
 */
typedef struct queue_t {
    node_t** node;
    size_t end;
    size_t size;
} queue_t;

/* Linked list to be inserted in hash table to store pointers to visited nodes
 */
typedef struct list_t {
    struct node_t* node;
    struct list_t* next;
} list_t;

/* Struct containing size of HASH_TABLE array with pointers to linked list
 * for hashed grid value
 */
typedef struct hash_t {
    list_t* hashed[HASH_TABLE];
} hash_t;

/* Stores list of solution steps, and variable for number
   of steps */
typedef struct stack_t {
    node_t* node[MAX_STEPS];
    int top;
} stack_t;

/* ------ SOLVER FUNCTIONS ------ */
void solve8Tile(queue_t* p_queue, hash_t* table, char* s);
bool expandNode(queue_t* p_queue, hash_t* table);
bool shiftTile(swap_t dir, queue_t* p_queue, hash_t* table, node_t* parent);

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

/* ------ HASHING FUNCTIONS ------ */
unsigned long djb2Hash(int grid[SIZE][SIZE]);
void initHashTable(hash_t* table);
void addHashTable(hash_t* table, node_t* node);
bool searchHashTable(hash_t* table, node_t* node);

list_t* addListNode(node_t* node);
bool searchList(list_t* list, node_t* node);
void unloadNodes(hash_t* table);
void unloadList(list_t* list);

/* ------ PRIORITY FUNCTION CALCULATIONS ------ */
int fPriority(int grid[SIZE][SIZE], int g);
int manhattanDistance(int grid[SIZE][SIZE]);

/* ------- STACK FUNCTIONS ------ */
void initStack(stack_t* solution);
void push(stack_t* solution, node_t* grid);
node_t* pop(stack_t* solution);

/* ------ UTILITY & INPUT FUNCTIONS ------ */
void loadSolution(queue_t* p_queue, stack_t* solution);
void printSolution(stack_t* solution);
void printBoard(int grid[SIZE][SIZE]);
bool checkInputString(char* s);
bool isSolvable(char* s);
void swap(int* n1, int* n2);
void test(void);

/* ------- SDL FUNCTIONS ------- */
void animateSolution(stack_t* solution);

void nextStep(node_t* curr, node_t* next, SDL_Simplewin* sw, SDL_Rect* rect, fntrow fontdata[FNTCHARS][FNTHEIGHT]);
void slideTile(swap_t dir, int val, int start_x, int start_y, SDL_Simplewin* sw, SDL_Rect* tile, fntrow fontdata[FNTCHARS][FNTHEIGHT]);
void getPosition(swap_t dir, int* x, int* y);

void drawGrid(int grid[SIZE][SIZE], SDL_Simplewin* sw, SDL_Rect* rect, fntrow fntdata[FNTCHARS][FNTHEIGHT]);
void drawTile(int tile, int x, int y, SDL_Simplewin* sw, SDL_Rect* rect, fntrow fontdata[FNTCHARS][FNTHEIGHT]);
void drawBorder(col_t colour, SDL_Simplewin* sw, SDL_Rect* border);
void setFillColour(SDL_Simplewin* sw, col_t colour);

void initObjects(SDL_Simplewin* sw, SDL_Rect* tile, fntrow fontdata[FNTCHARS][FNTHEIGHT]);
void responsiveDelay(Uint32 ms, SDL_Simplewin* sw);
void SDLExit(void);

int main(int argc, char* argv[]) {
    queue_t p_queue;
    hash_t table;
    stack_t solution;

    /*test();*/

    if (argc != 2) {
        fprintf(stderr,
                "ERROR: Incorrect usage, try e.g. %s \"64785 321\"\n",
                argv[0]);
        return 1;
    }
    if (!checkInputString(argv[1])) {
        fprintf(stderr,
                "INVALID. Please ensure string is: \n"
                "   - 3x3 grid as 9 characters\n"
                "   - Free tile denoted with space\n"
                "   - Unique tile values\n"
                "Why not try the hardest problem \"64785 321\"\n");
        return 1;
    }

    if (!isSolvable(argv[1])) {
        printf("This 8-tile board cannot be solved...\n");
        return 0;
    }

    printf("Solving puzzle. Please wait...\n");
    solve8Tile(&p_queue, &table, argv[1]);

    loadSolution(&p_queue, &solution);
    printf("\nPuzzle Solved in %i steps:\n\n", solution.top - 1);

    /* Print solution is old terminal print version. Animate uses SDL */
    /*printSolution(&solution);*/
    animateSolution(&solution);

    unloadPQueue(&p_queue);
    unloadNodes(&table);

    return 0;
}

/* ------ SOLVER FUNCTIONS ------ */
/* Solver initailises the queue with the starting grid and then expand nodes
 * onto the queue until a solution is found. Assumes a valid string. Uses
 * a* search with manhattan distance priority function. When solution is found, 
 * last element is loaded onto queue, where it can then be read when loading 
 * the solution.
 */
void solve8Tile(queue_t* p_queue, hash_t* table, char* s) {
    node_t* node = initNode(s);
    initPQueue(p_queue, node);
    initHashTable(table);
    /* Add first element to hash table */
    addHashTable(table, node);

    while (!expandNode(p_queue, table)) {
        /* Don't need to check if queue is empty, as only solvable grids 
       are permitted */
    }
}

/* Calls function shiftTile to expand possible moves of current node. Will 
 * return true if the solution is found
 */
bool expandNode(queue_t* p_queue, hash_t* table) {
    node_t* parent = getMin(p_queue);
    int x = parent->x;
    int y = parent->y;

    /* Remove expanded node from priority queue. Reference is still held in 
       hash table */
    delMin(p_queue);

    if (x < SIZE - 1) {
        if (shiftTile(LEFT, p_queue, table, parent)) {
            return true;
        }
    }
    if (x > 0) {
        if (shiftTile(RIGHT, p_queue, table, parent)) {
            return true;
        }
    }
    if (y < SIZE - 1) {
        if (shiftTile(UP, p_queue, table, parent)) {
            return true;
        }
    }
    if (y > 0) {
        if (shiftTile(DOWN, p_queue, table, parent)) {
            return true;
        }
    }

    return false;
}

/* Generates next board state based on direction of shift. Shift direction 
 * refers to direction of tile being moved into the free space. Function assumes
 * that a valid shiftdirection is given. Copies board into tmp pointer, which 
 * is only added to the queue if it's a valid board (i.e. not repeated)
 */
bool shiftTile(swap_t dir, queue_t* p_queue, hash_t* table, node_t* parent) {
    static node_t* tmp = NULL;
    int x1, y1, x2, y2;

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
    tmp->hash = djb2Hash(tmp->grid);

    if (checkTarget(tmp->grid)) {
        /* Set cost function to zero to ensure that this ends up at head of
           priority queue */
        tmp->f = 0;
        addHashTable(table, tmp);
        insertPQueue(p_queue, tmp);
        return true;
    } else if (!searchHashTable(table, tmp)) {
        addHashTable(table, tmp);
        insertPQueue(p_queue, tmp);
        /* reset tmp so it is malloced again on next loop */
        tmp = NULL;
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

/* Compares whether two 8-tile boards are the same. Have used memcmp() for
 * improved speed.
 */
bool compareBoards(int grid1[SIZE][SIZE], int grid2[SIZE][SIZE]) {
    return !(memcmp(grid1, grid2, SIZE * SIZE * sizeof(int)));
}

/* ------- LOADING FUNCTIONS ------ */
/* Initiliases node with starting board which can then be added to the hash
 * table and priority queue
 */
node_t* initNode(char* s) {
    node_t* node = (node_t*)malloc(sizeof(node_t));

    loadBoard(node->grid, s);
    findFreeTile(node);
    node->step = 0;
    node->parent = NULL;
    node->f = fPriority(node->grid, node->step);
    node->hash = djb2Hash(node->grid);
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
/* Priority queue is implemented as a binary heap with a dynamically sized 
 * array. Should offer O(1) retrieval of next (priority) node, O(log n) deletion
 * and insertion. Looking to minimise cost function so min f-value will be moved
 * to top
 */

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
    p_queue->end = 1;
}

/* Insert element into priority queue and put it in right position in binary
 * heap
 */
void insertPQueue(queue_t* p_queue, node_t* node) {
    if (p_queue->end == p_queue->size) {
        expandPQueue(p_queue);
    }

    p_queue->end++;

    p_queue->node[p_queue->end] = node;
    percolateUp(p_queue);
}

/* Retrieve the minimum f-value node from the queue. It does not get remove
 */
node_t* getMin(queue_t* p_queue) {
    return p_queue->node[1];
}

/* Delecte minimum f-value node from queue and bubble up next smallest f-value
 * node
 */
void delMin(queue_t* p_queue) {
    p_queue->node[1] = p_queue->node[p_queue->end];
    percolateDown(p_queue);

    p_queue->end--;
}

bool isEmpty(queue_t* p_queue) {
    if (p_queue->end == 0) {
        return true;
    }
    return false;
}

/* Reallocates array if there are too many elements in the queue. Expanded by
 * factor determined by BUFF_FACTOR
 */
void expandPQueue(queue_t* p_queue) {
    node_t** tmp;
    p_queue->size = p_queue->size * BUFF_FACTOR;

    tmp = (node_t**)realloc(p_queue->node, (p_queue->size + 1) * sizeof(node_t*));
    if (tmp == NULL) {
        fprintf(stderr, "Memory allocation error\n");
        /* QUESTION should i free previously allced memory?*/
        exit(EXIT_FAILURE);
    }
    p_queue->node = tmp;
}

/* Moves inserted element to corrrect place in tree
 */
void percolateUp(queue_t* p_queue) {
    node_t **child, **parent;
    size_t i = p_queue->end;

    while (i / 2 > 0) {
        child = &(p_queue->node[i]);
        parent = &(p_queue->node[i / 2]);
        if ((*child)->f < (*parent)->f) {
            swapNodePtr(child, parent);
        }
        i /= 2;
    }
}

/* Moves min element to top of tree when previous min is remove
 */
void percolateDown(queue_t* p_queue) {
    node_t **child, **parent;
    size_t i = 1;

    while (i * 2 <= p_queue->end) {
        parent = &p_queue->node[i];

        i = minChildIndex(p_queue, i);
        child = &p_queue->node[i];

        if ((*child)->f < (*parent)->f) {
            swapNodePtr(child, parent);
        }
    }
}

/* Find the minimum value child to determine which branch to move up a level
 */
size_t minChildIndex(queue_t* p_queue, size_t i) {
    int f1, f2;
    size_t child_index = i * 2;

    /* If final node only has one branch, can only return this */
    if (child_index == p_queue->end) {
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

/* Each node in binary tree is a pointer to a node, so need to swap the pointers
 * by passing pointers to pointers into function
 */
void swapNodePtr(node_t** n1, node_t** n2) {
    node_t* tmp = *n1;
    *n1 = *n2;
    *n2 = tmp;
}

void unloadPQueue(queue_t* p_queue) {
    free(p_queue->node);
}

/* ------ HASHING FUNCTIONS ------ */
/* Hashing the grids does result in significant speed increases, even with a 
 * small hash table. Repeatedly iterating through a LL of up to 30000 
 * expanded nodes for worst case (i.e. linear search) takes significantly longer
 */

/* Would it be possible to hash based on node ptr value instead? Would this be
   simpler? */
/* http://www.cse.yorku.ca/~oz/hash.html */
unsigned long djb2Hash(int grid[SIZE][SIZE]) {
    int i, j;
    unsigned long hash = HASH;
    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            hash += hash * MAGIC ^ (unsigned long)grid[i][j];
        }
    }
    return hash % HASH_TABLE;
}

/* Initialise an empty hash table
 */
void initHashTable(hash_t* table) {
    unsigned int i;
    for (i = 0; i < HASH_TABLE; i++) {
        table->hashed[i] = NULL;
    }
}

/* Add a node to hash table by hashing grid value and then adding node_t* to
 * linked list at relevant point of array. This deals with collisions, and
 * freeing all allocated nodes later as a reference to every explored node is
 * stored in the hash table
 */
void addHashTable(hash_t* table, node_t* node) {
    unsigned long hash = node->hash;
    list_t* list = table->hashed[hash];

    if (!list) {
        table->hashed[hash] = addListNode(node);
        return;
    }
    while (list->next != NULL) {
        list = list->next;
    }
    list->next = addListNode(node);
}

/* Search hash table for duplicate values
 */
bool searchHashTable(hash_t* table, node_t* node) {
    unsigned long hash = node->hash;
    list_t* list = table->hashed[hash];
    if (!list) {
        return false;
    }
    return searchList(list, node);
}

/* Add a node to the relevant linked list
 */
list_t* addListNode(node_t* node) {
    list_t* tmp = (list_t*)malloc(sizeof(list_t));
    if (tmp == NULL) {
        fprintf(stderr, "Memory allocation error\n");
        exit(EXIT_FAILURE);
    }
    tmp->node = node;
    tmp->next = NULL;
    return tmp;
}

/* Search linked list of corresponding hash table bucket
 */
bool searchList(list_t* list, node_t* node) {
    list_t* list_node = list;
    while (list_node) {
        if (compareBoards(list_node->node->grid, node->grid)) {
            return true;
        }
        list_node = list_node->next;
    }
    return false;
}

/* This and the function below unload all the expanded nodes allocated during
 * the search. The hash table contains references to all explored nodes, so
 * going through it ensures that all memory is freed
 */
void unloadNodes(hash_t* table) {
    unsigned long i;
    for (i = 0; i < HASH_TABLE; i++) {
        if (table->hashed[i]) {
            unloadList(table->hashed[i]);
        }
    }
}

void unloadList(list_t* list) {
    if (list->next) {
        unloadList(list->next);
    }
    free(list->node);
    free(list);
}

/* ------- PRIORIITY FUNCTION CALCULATION ------- */
/* Initial idea to a* search came from here
 * https://blog.goodaudience.com/solving-8-puzzle-using-a-algorithm-7b509c331288
 */
/* Calculate cost of node based. h-score is manhattan distance and g-score is
 * number of steps made
 */
int fPriority(int grid[SIZE][SIZE], int step) {
    return manhattanDistance(grid) + step;
}

int manhattanDistance(int grid[SIZE][SIZE]) {
    int i, j;
    int manhattan = 0;
    int num;
    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            if (grid[i][j]) {
                num = grid[i][j] - 1;
                manhattan += abs(num / 3 - i) + abs(num % 3 - j);
            }
        }
    }
    return manhattan;
}

/* ------ STACK FUNCTIONS ------ */
/* If I were to expand this to larger grids, I would define the stack as a linked
 * list or dynamically allocated array so it can be sized to match the number of
 * solution steps. Given that 8-tile is relatively constrained, have just used 
 * a fixed size array here that can hold the maximum number of steps.
 */

/* Stack has been implemented on basis of FIFO as list will be read in reverse
 * from queue once solution is found
 */
void initStack(stack_t* solution) {
    solution->top = 0;
}

/* Pushes pointer to grid_t* onto stack. Stack shouldn't overflow as 8tile has
 * maximum number of steps, defined by MAX_STEPS
 */
void push(stack_t* solution, node_t* grid) {
    if (solution->top > MAX_STEPS) {
        fprintf(stderr, "Stack overflow...\n");
        exit(EXIT_FAILURE);
    }
    solution->node[solution->top++] = grid;
}

/* Returns next node from stack or NULL if stack is empty */
node_t* pop(stack_t* solution) {
    if (solution->top <= 0) {
        return NULL;
    }
    return solution->node[--solution->top];
}

/* ------ UTILITY & INPUT FUNCTIONS ------ */
/* Loads solution by going through parent nodes end to start grid
 */
void loadSolution(queue_t* p_queue, stack_t* solution) {
    node_t* node = p_queue->node[1];
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

/* ------- SDL FUNCTIONS ------- */
/* Contains complete process for animating the final solution top. Initialises
 * SDL and draws grid
 */
void animateSolution(stack_t* solution) {
    node_t *curr, *next;
    SDL_Simplewin sw;
    SDL_Rect tile, border;
    fntrow fontdata[FNTCHARS][FNTHEIGHT];

    initObjects(&sw, &tile, fontdata);

    /* Draw starting grid */
    curr = pop(solution);
    drawGrid(curr->grid, &sw, &tile, fontdata);
    drawBorder(RED, &sw, &border);
    responsiveDelay(START_DELAY, &sw);

    /* Slide tiles through solution steps */
    drawBorder(ORANGE, &sw, &border);
    while ((next = pop(solution))) {
        nextStep(curr, next, &sw, &tile, fontdata);
        if (sw.finished) {
            SDLExit();
            return;
        }
        curr = next;
    }

    /* Wait here until user wants to close window */
    drawBorder(GREEN, &sw, &border);
    while (!sw.finished) {
        Neill_SDL_UpdateScreen(&sw);
        Neill_SDL_Events(&sw);
    }
    SDLExit();
}

/* Determines position of free space and which tile will be shifted into it.
 * Then calls slideTile() to slide the tile in the right direction
 */
void nextStep(node_t* curr, node_t* next, SDL_Simplewin* sw, SDL_Rect* tile, fntrow fontdata[FNTCHARS][FNTHEIGHT]) {
    /* Get x,y index of free cell in current step and value tile that will be 
       moved */
    int free_x = curr->x;
    int free_y = curr->y;
    int value = next->grid[free_y][free_x];

    /* Convert index of current and next free cell to pixel coordinates */
    int tile_x = next->x * TILE_SIZE;
    int tile_y = next->y * TILE_SIZE;
    free_x *= TILE_SIZE;
    free_y *= TILE_SIZE;

    if (tile_x > free_x) {
        slideTile(LEFT, value, tile_x, tile_y, sw, tile, fontdata);
    }
    if (tile_x < free_x) {
        slideTile(RIGHT, value, tile_x, tile_y, sw, tile, fontdata);
    }
    if (tile_y > free_y) {
        slideTile(UP, value, tile_x, tile_y, sw, tile, fontdata);
    }
    if (tile_y < free_y) {
        slideTile(DOWN, value, tile_x, tile_y, sw, tile, fontdata);
    }
}

/* Animates sliding of tile into free space
 */
void slideTile(swap_t dir, int val, int start_x, int start_y, SDL_Simplewin* sw, SDL_Rect* tile, fntrow fontdata[FNTCHARS][FNTHEIGHT]) {
    int i;
    int next_x = start_x;
    int next_y = start_y;

    for (i = 0; i < TILE_SIZE; i++) {
        getPosition(dir, &next_x, &next_y);
        /* Only new free space needs to be drawn. Old free tile gradually gets
           covered by sliding tile */
        drawTile(0, start_x, start_y, sw, tile, fontdata);
        drawTile(val, next_x, next_y, sw, tile, fontdata);
        Neill_SDL_UpdateScreen(sw);
        responsiveDelay(SLIDE_DELAY, sw);
        Neill_SDL_Events(sw);
        if (sw->finished) {
            return;
        }
    }
    responsiveDelay(STEP_DELAY, sw);
}

/* Calculates updated coordinates of sliding tile
 */
void getPosition(swap_t dir, int* x, int* y) {
    *x += ((dir == RIGHT) - (dir == LEFT));
    *y += ((dir == DOWN) - (dir == UP));
}

/* Draw complete 8tile grid 
 */
void drawGrid(int grid[SIZE][SIZE], SDL_Simplewin* sw, SDL_Rect* tile, fntrow fontdata[FNTCHARS][FNTHEIGHT]) {
    int i, j;
    int x, y;

    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            x = j * TILE_SIZE;
            y = i * TILE_SIZE;
            drawTile(grid[i][j], x, y, sw, tile, fontdata);
        }
    }
}

/* Draws a single tile. Expects value of tile and x,y coordinates in px as
 * input
 */
void drawTile(int val, int x, int y, SDL_Simplewin* sw, SDL_Rect* tile, fntrow fontdata[FNTCHARS][FNTHEIGHT]) {
    int cx = x + OFFSET + TILE_SIZE / 2;
    int cy = y + OFFSET + TILE_SIZE / 2;

    /* Set coordinates of tile */
    tile->x = x + OFFSET;
    tile->y = y + OFFSET;

    /* Draw tiles in gray with number and free space in darker gray */
    if (val) {
        setFillColour(sw, GRAY);
        SDL_RenderFillRect(sw->renderer, tile);
        setFillColour(sw, WHITE);
        Neill_SDL_RenderFillCircle(sw->renderer, cx, cy, RADIUS);
        Neill_SDL_DrawChar(sw, fontdata, val + '0', x + CHAR_X_OFFSET + OFFSET, y + CHAR_Y_OFFSET + OFFSET);
    } else {
        setFillColour(sw, BLACK);
        SDL_RenderFillRect(sw->renderer, tile);
        setFillColour(sw, DARK_GRAY);
        Neill_SDL_RenderFillCircle(sw->renderer, cx, cy, RADIUS_FREE);
    }

    /* Draw border between tiles */
    setFillColour(sw, DARK_GRAY);
    SDL_RenderDrawRect(sw->renderer, tile);
}

/* Draw outside border around 8tile grid. Will be set to RED for start, ORANGE
 * while solving and GREEN for solution
 */
void drawBorder(col_t colour, SDL_Simplewin* sw, SDL_Rect* border) {
    setFillColour(sw, colour);
    border->w = WHEIGHT;
    border->h = BORDER;
    border->x = 0;
    border->y = 0;
    SDL_RenderFillRect(sw->renderer, border);
    border->y = WHEIGHT - BORDER;
    SDL_RenderFillRect(sw->renderer, border);

    border->w = BORDER;
    border->h = WWIDTH;
    border->x = 0;
    border->y = 0;
    SDL_RenderFillRect(sw->renderer, border);
    border->x = WWIDTH - BORDER;
    SDL_RenderFillRect(sw->renderer, border);
}

/* Allows easy switching to predetermined colours for rendering 8 tile
 */
void setFillColour(SDL_Simplewin* sw, col_t colour) {
    switch (colour) {
        case WHITE:
            Neill_SDL_SetDrawColour(sw, 255, 255, 255);
            break;
        case GRAY:
            Neill_SDL_SetDrawColour(sw, 100, 100, 100);
            break;
        case DARK_GRAY:
            Neill_SDL_SetDrawColour(sw, 40, 40, 40);
            break;
        case BLACK:
            Neill_SDL_SetDrawColour(sw, 25, 25, 25);
            break;
        case RED:
            Neill_SDL_SetDrawColour(sw, 190, 0, 0);
            break;
        case ORANGE:
            Neill_SDL_SetDrawColour(sw, 200, 125, 0);
            break;
        case GREEN:
            Neill_SDL_SetDrawColour(sw, 0, 170, 0);
            break;
    }
}

/* Initialises all the objects used for drawin 8tile grid (tile rectangle,
 * border rectangle and font)
 */
void initObjects(SDL_Simplewin* sw, SDL_Rect* tile, fntrow fontdata[FNTCHARS][FNTHEIGHT]) {
    tile->h = TILE_SIZE;
    tile->w = TILE_SIZE;
    Neill_SDL_ReadFont(fontdata, "./mode7.fnt");
    Neill_SDL_Init(sw);
}

/* Allows delays during which it's possible to exit the window 
 */
void responsiveDelay(Uint32 ms, SDL_Simplewin* sw) {
    Uint32 i;
    for (i = 0; i <= ms; i++) {
        Neill_SDL_UpdateScreen(sw);
        Neill_SDL_Events(sw);
        if (sw->finished) {
            SDLExit();
            return;
        }
        SDL_Delay(1);
    }
}

/* Handles exiting SDL in single function
 */
void SDLExit(void) {
    SDL_Quit();
    atexit(SDL_Quit);
}

/* Testing functions don't repeat testing from preivous exercises. Have only 
 * checked that priority queue and hash table implementations work properly
 */
void test(void) {
    int i;
    int test_grid[SIZE][SIZE] = {{8, 1, 3}, {4, 0, 2}, {7, 6, 5}};
    node_t *tst_node_1, *tst_node_2, *tst_node_3, *tst_node_4, *tst_node_5;
    hash_t test_table;

    queue_t p_queue_tst;

    /* Find free tile and load board testing */
    assert(manhattanDistance(test_grid) == 10);

    tst_node_1 = initNode("12345 678");
    assert(tst_node_1->x == 2 && tst_node_1->y == 1);
    assert(tst_node_1->step == 0);
    assert(tst_node_1->f == 5);
    for (i = 0; i < 5; i++) {
        assert(tst_node_1->grid[i / 3][i % 3] == i + 1);
    }

    /* ------- PRIORITY QUEUE TESTING ------ */
    tst_node_2 = initNode("123456 78");
    tst_node_3 = initNode("1 2345678");
    tst_node_4 = initNode("87654 321");
    tst_node_5 = initNode("1234567 8");

    /* Mainly here for clarity to understand binary tree*/
    assert(tst_node_1->f == 5);
    assert(tst_node_2->f == 2);
    assert(tst_node_3->f == 11);
    assert(tst_node_4->f == 19);
    assert(tst_node_5->f == 1);

    initPQueue(&p_queue_tst, tst_node_1);
    assert(p_queue_tst.end == 1);
    assert(p_queue_tst.size == QUEUE_SIZE);
    assert(memcmp(p_queue_tst.node[1], tst_node_1, sizeof(node_t)) == 0);

    insertPQueue(&p_queue_tst, tst_node_3);
    assert(memcmp(p_queue_tst.node[1], tst_node_1, sizeof(node_t)) == 0);
    assert(memcmp(p_queue_tst.node[2], tst_node_3, sizeof(node_t)) == 0);

    insertPQueue(&p_queue_tst, tst_node_2);
    assert(memcmp(p_queue_tst.node[1], tst_node_2, sizeof(node_t)) == 0);
    assert(memcmp(p_queue_tst.node[2], tst_node_3, sizeof(node_t)) == 0);
    assert(memcmp(p_queue_tst.node[3], tst_node_1, sizeof(node_t)) == 0);

    insertPQueue(&p_queue_tst, tst_node_4);
    insertPQueue(&p_queue_tst, tst_node_5);

    assert(memcmp(p_queue_tst.node[1], tst_node_5, sizeof(node_t)) == 0);
    assert(memcmp(p_queue_tst.node[2], tst_node_2, sizeof(node_t)) == 0);
    assert(memcmp(p_queue_tst.node[3], tst_node_1, sizeof(node_t)) == 0);
    assert(memcmp(p_queue_tst.node[4], tst_node_4, sizeof(node_t)) == 0);
    assert(memcmp(p_queue_tst.node[5], tst_node_3, sizeof(node_t)) == 0);

    assert(p_queue_tst.end == 5);
    assert(memcmp(getMin(&p_queue_tst), tst_node_5, sizeof(node_t)) == 0);

    delMin(&p_queue_tst);
    assert(p_queue_tst.end == 4);
    assert(memcmp(getMin(&p_queue_tst), tst_node_2, sizeof(node_t)) == 0);
    assert(memcmp(p_queue_tst.node[2], tst_node_3, sizeof(node_t)) == 0);

    delMin(&p_queue_tst);
    assert(p_queue_tst.end == 3);
    assert(memcmp(getMin(&p_queue_tst), tst_node_1, sizeof(node_t)) == 0);
    assert(memcmp(p_queue_tst.node[1], tst_node_1, sizeof(node_t)) == 0);
    assert(memcmp(p_queue_tst.node[2], tst_node_3, sizeof(node_t)) == 0);
    assert(memcmp(p_queue_tst.node[3], tst_node_4, sizeof(node_t)) == 0);

    /* TESTING checked with HASH_TABLE set to 1 to ensure linked list is
     * working properly
     */
    initHashTable(&test_table);
    assert(searchHashTable(&test_table, p_queue_tst.node[2]) == false);

    addHashTable(&test_table, p_queue_tst.node[1]);
    assert(searchHashTable(&test_table, p_queue_tst.node[1]) == true);
    assert(searchHashTable(&test_table, p_queue_tst.node[2]) == false);

    addHashTable(&test_table, p_queue_tst.node[2]);
    assert(searchHashTable(&test_table, p_queue_tst.node[1]) == true);
    assert(searchHashTable(&test_table, p_queue_tst.node[2]) == true);
    assert(searchHashTable(&test_table, p_queue_tst.node[3]) == false);

    addHashTable(&test_table, p_queue_tst.node[3]);
    assert(searchHashTable(&test_table, p_queue_tst.node[1]) == true);
    assert(searchHashTable(&test_table, p_queue_tst.node[2]) == true);
    assert(searchHashTable(&test_table, p_queue_tst.node[3]) == true);

    /* Cleaning things up */
    unloadNodes(&test_table);
    free(tst_node_2);
    free(tst_node_5);
    free(p_queue_tst.node);
}

/* ------ DEPRECATED. NOT IN USE ANYMORE ------ */
/* ------ SEARCH TREE FOR VISITED LIST ------ */
typedef struct tree_t {
    node_t* node;
    struct tree_t* children[SIZE * SIZE];
} tree_t;

/* DEPRECATED. WAS PREVIOUSLY USING THIS TO SPEED UP SEARCH, BUT THEN DECIDED
 * A HASH TABLE WOULD BE NICE TO IMPLEMENT TOO ;)
 */
/* ------ SEARCH TREE AND VISITED FUNCTIONS ------ */
tree_t* initTree(node_t* node);
void insertTree(tree_t* tree, node_t* node);
bool searchInTree(tree_t* tree, int grid[SIZE][SIZE]);
tree_t* createTreeNode(void);
void unloadTree(tree_t* tree);

tree_t* initTree(node_t* node) {
    tree_t* tree = createTreeNode();
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
    tree_t* ptr = (tree_t*)calloc(1, sizeof(tree_t));
    if (ptr == NULL) {
        /* QUESTION should i free previously allced memory?*/
        fprintf(stderr, "Memory allocation error\n");
        exit(EXIT_FAILURE);
    }
    return ptr;
}

/*FIXME not totally sure on this */
void unloadTree(tree_t* tree) {
    int i;
    for (i = 0; i < SIZE * SIZE; i++) {
        if (tree->children[i]) {
            unloadTree(tree->children[i]);
        }
    }
    free(tree->node);
    free(tree);
}
