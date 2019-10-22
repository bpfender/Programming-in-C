#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "neillncurses.h"

#define CIRC_GRID 40
#define GENERATIONS 5

/*typedef enum bools { false,
                     true } bools;*/

/* Contains current and next generation of circuit, as well as location in grid. 
 * 2D arrays (grid_a & grid_b) should not be accessed directly. Instead use 
 * "current" and "next" pointers. Pointers are defined mainly so that "copying"
 * next array into current array just involves swapping pointers rather than
 * copying all elements.
 */
typedef struct circuit_struct {
    char (*current)[CIRC_GRID];
    char (*next)[CIRC_GRID];
    int x;
    int y;
    char grid_a[CIRC_GRID][CIRC_GRID];
    char grid_b[CIRC_GRID][CIRC_GRID];
} circuit_struct;

/* ------ SIMULATION FUNCTIONS ------- */
void nextGeneration(circuit_struct* circuit);
void updateCurrent(circuit_struct* circuit);
void nextCellState(circuit_struct* circuit);
bool checkMooreNeighborhood(circuit_struct* circuit);
bool checkBounds(int x, int y);

/* ------ UTILITY & FILE READING FUNCTIONS ------ */
void initStruct(circuit_struct* circuit);
bool loadCircuitFile(char* filename, circuit_struct* circuit);
bool getCircuitLine(char line[], FILE* fp);
bool checkCircuit(char circuit[][CIRC_GRID]);
bool checkSymbol(char c);
void printCircuit(char circuit[][CIRC_GRID]);
void test(void);

/* TODO IMPLEMENT ARGV ARGV */
int main(void) {
    circuit_struct circuit;
    int i;
    NCURS_Simplewin sw;

    test();

    initStruct(&circuit);

    if (loadCircuitFile("wirewcircuit1.txt", &circuit) == false) {
        return 1;
    }
    /*
    Neill_NCURS_Init(&sw);
    Neill_NCURS_CharStyle(&sw, "H", COLOR_BLUE, COLOR_BLUE, A_NORMAL);
    Neill_NCURS_CharStyle(&sw, "t", COLOR_RED, COLOR_RED, A_NORMAL);
    Neill_NCURS_CharStyle(&sw, "c", COLOR_YELLOW, COLOR_YELLOW, A_NORMAL);
    Neill_NCURS_CharStyle(&sw, " ", COLOR_BLACK, COLOR_BLACK, A_NORMAL);

    do {
        Neill_NCURS_PrintArray(&circuit.current[0][0], CIRC_GRID, CIRC_GRID, &sw);
        nextGeneration(&circuit);
        Neill_NCURS_Delay(50.0);
        Neill_NCURS_Events(&sw);

    } while (!sw.finished);
    Neill_NCURS_Done();*/

    for (i = 0; i < GENERATIONS; i++) {
        nextGeneration(&circuit);
        printf("Generation %4d\n", i + 1);
        printCircuit(circuit.current);
    };
    return 0;
}

/* ------ SIMULATION FUNCTIONS ------ */

/* Iterate through circuit cells. x and y are being modified in circuit_struct
 * directly so that the correct cell is checked when nextCellState() is called
 */
void nextGeneration(circuit_struct* circuit) {
    int* x = &circuit->x;
    int* y = &circuit->y;

    for (*x = 0; *x < CIRC_GRID; (*x)++) {
        for (*y = 0; *y < CIRC_GRID; (*y)++) {
            nextCellState(circuit);
        }
    }

    updateCurrent(circuit);
}

/* Swaps pointers of current and next, so that the current pointer points to the
 * newly calculated next array
 */
void updateCurrent(circuit_struct* circuit) {
    char(*tmp)[CIRC_GRID];

    tmp = circuit->current;
    circuit->current = circuit->next;
    circuit->next = tmp;
}

/* Checks what the next cell state should be based on current x/y-coordinates 
 * in the circuit_struct and returns corresponding value. Modifies array pointed
 * to by next directly to avoid writing ' ' repeatedly
 */
void nextCellState(circuit_struct* circuit) {
    char current = circuit->current[circuit->y][circuit->x];
    char* next = &circuit->next[circuit->y][circuit->x];

    switch (current) {
        case 'H':
            *next = 't';
            break;
        case 't':
            *next = 'c';
            break;
        case 'c':
            if (checkMooreNeighborhood(circuit) == true) {
                *next = 'H';
                break;
            }
            *next = 'c';
            break;
    }
}

/* Checks surrounding 8 cells for heads, returning true if conditions for 
 * copper->head are met. Takes whole circuit_struct as argument to allow 
 * additional bounds checking in checkBounds()
 */
bool checkMooreNeighborhood(circuit_struct* circuit) {
    int i, j;
    int head_count = 0;
    int x = circuit->x;
    int y = circuit->y;

    for (i = -1; i <= 1; i++) {
        for (j = -1; j <= 1; j++) {
            /* Check that coordinate is within circuit grid and ignore the 
             * central cell. Checking the central cell isn't necesarry here, 
             * but may improve portability of code
             */
            if (checkBounds(x + i, y + j) && !(i == 0 && j == 0)) {
                head_count += (circuit->current[y + j][x + i] == 'H');
            }
        }
    }

    return head_count == 1 || head_count == 2;
}

/* Checks that the coordinates tested fall within the bounds of the circuit
 * grid. If they don't, returns 0;
 */
bool checkBounds(int x, int y) {
    return (x >= 0 && x < CIRC_GRID) &&
           (y >= 0 && y < CIRC_GRID);
}

/* ------ UTILITY AND FILE HANDLING FUNCTIONS ------ */

/* Point current and next pointer to 2D circuit grids */
void initStruct(circuit_struct* circuit) {
    circuit->current = circuit->grid_a;
    circuit->next = circuit->grid_b;
}

bool loadCircuitFile(char* filename, circuit_struct* circuit) {
    FILE* file;
    int line = 0;

    /* TODO more error handling aorund incorrect filenames? length etc. */
    file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Cannot open \"%s\"\nExiting\n", filename);
        return false;
    }

    /* File will always be (40 + \n) * 40 characters long. This does a quick
     * sense check that the file is the expected size so that it can fit into
     * circuit array
     */
    fseek(file, 0, SEEK_END);
    if (ftell(file) != (CIRC_GRID + 1) * CIRC_GRID) {
        fprintf(stderr, "Unexpected file size\nExiting\n");
        fclose(file);
        return false;
    }
    rewind(file);

    /* TODO some error checking required here. */
    while (getCircuitLine(circuit->current[line], file) == true) {
        line++;
    }

    /* Check loaded circuit is valid before copying to second circuit array.
     * Circuit is initially copied to populate both arrays, as nextCellState()
     * function modifies circuit arrays directly, only changing cells when
     * necesarry rather than returning an "updated" value for each cell
     * examined.
     */
    if (checkCircuit(circuit->current) == false) {
        fprintf(stderr, "Exiting...\n");
        fclose(file);
        return false;
    }
    memcpy(circuit->next, circuit->current, CIRC_GRID * CIRC_GRID);

    fclose(file);
    return true;
}

/* Reads lines based on expected length of CIRC_GRID + '\n' characters. If the
 * line doesn't follow this format, an error will be returned. 
 */
bool getCircuitLine(char line[CIRC_GRID], FILE* file) {
    int c, i;

    for (i = 0; i < CIRC_GRID + 1; i++) {
        c = getc(file);

        if (c == EOF) {
            if (ferror(file)) {
                fprintf(stderr, "Error reading file\nExiting...\n");
                fclose(file);
                exit(EXIT_FAILURE);
            } else {
                return false;
            }
        }
        if ((c == '\n' && i < CIRC_GRID) ||
            (i == CIRC_GRID && c != '\n')) {
            fprintf(stderr, "Unexpected line length\nExiting...\n");
            fclose(file);
            exit(EXIT_FAILURE);
        }
        if (i < CIRC_GRID) {
            line[i] = (char)c;
        }
    }
    return true;
}

/* Iterates through complete circuit array to check for any errors. Lists
 * locations of any invalid characters
 */
bool checkCircuit(char circuit[CIRC_GRID][CIRC_GRID]) {
    int i, j;
    bool check = true;

    for (i = 0; i < CIRC_GRID; i++) {
        for (j = 0; j < CIRC_GRID; j++) {
            if (checkSymbol(circuit[i][j]) == false) {
                fprintf(stderr,
                        "Invalid symbol \"%c\" used in circuit at line %d, column %d\n",
                        circuit[i][j], i + 1, j + 1);
                check = false;
            }
        }
    }
    return check;
}

bool checkSymbol(char c) {
    switch (c) {
        case 'H':
        case 't':
        case 'c':
        case ' ':
            return true;
    }
    return false;
}

void printCircuit(char circuit[][CIRC_GRID]) {
    int i;

    /* QUESTION is this  a naughty way of doing this? */
    /* FIXME hard coded line bounds at the moment */
    for (i = 0; i < CIRC_GRID; i++) {
        printf("%.*s\n", CIRC_GRID, circuit[i]);
    }
}

void test(void) {
    assert(checkSymbol('c') == true);
    assert(checkSymbol('H') == true);
    assert(checkSymbol('t') == true);
    assert(checkSymbol(' ') == true);
    assert(checkSymbol('o') == false);
}