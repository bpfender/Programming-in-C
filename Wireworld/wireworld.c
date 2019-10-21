#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "neillncurses.h"

#define CIRC_GRID 40
#define GRID_SIZE CIRC_GRID + 2 * PADDING

#define MOORE_GRID 3
#define GENERATIONS 5

typedef enum bools { false,
                     true } bools;

/* QUESTION notation is a bit weird at the moment */
typedef struct circuit_struct {
    char (*current)[CIRC_GRID];
    char (*next)[CIRC_GRID];
    int x;
    int y;
} circuit_struct;

bool loadCircuitFile(char* filename, char circuit[][CIRC_GRID]);
char nextCellState(circuit_struct* circuit);
void nextGeneration(circuit_struct* circuit);
bool checkMooreNeighborhood(circuit_struct* circuit);
bool checkSymbol(char c);
bool getCircuitLine(char line[], FILE* fp);
void updateCurrent(circuit_struct* circuit);
void test(void);
void printCircuit(char circuit[][CIRC_GRID], int generation);
bool checkCircuit(char circuit[][CIRC_GRID]);
bool checkBounds(int x, int y);

int main(void) {
    circuit_struct circuit;
    char current[CIRC_GRID][CIRC_GRID];
    char next[CIRC_GRID][CIRC_GRID];
    int i;

    circuit.current = current;
    circuit.next = next;

    test();

    NCURS_Simplewin sw;
    Neill_NCURS_CharStyle(&sw, "H", COLOR_BLUE, COLOR_BLUE, A_NORMAL);
    Neill_NCURS_CharStyle(&sw, "t", COLOR_RED, COLOR_RED, A_NORMAL);
    Neill_NCURS_CharStyle(&sw, "c", COLOR_YELLOW, COLOR_YELLOW, A_NORMAL);
    Neill_NCURS_CharStyle(&sw, " ", COLOR_BLACK, COLOR_BLACK, A_NORMAL);

    if (loadCircuitFile("wirewcircuit1.txt", circuit.current) == false) {
        return 1;
    }
    if (checkCircuit(circuit.current) == false) {
        return 1;
    }

    for (i = 0; i < GENERATIONS; i++) {
        nextGeneration(&circuit);
        printCircuit(circuit.current, i + 1);
    }

    return 0;
}

bool loadCircuitFile(char* filename, char circuit[][CIRC_GRID]) {
    FILE* file;
    int i = 0;

    /* TODO more error handling aorund incorrect filenames? length etc. */
    file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Cannot open %s\n", filename);
        return false;
    }

    while (getCircuitLine(circuit[i], file) == true) {
        i++;
    }

    /*while ((length = (getLine(buffer, CIRC_GRID + 1, file)))) {
        if (length != CIRC_GRID) {
            fprintf(stderr,
                    "Circuit width in line %d is not %d cells wide\n",
                    i, CIRC_GRID);
            fclose(file);
            return false;
        }
        if (i >= CIRC_GRID) {
            fprintf(stderr,
                    "Too many lines in input file\n");
            fclose(file);
            return false;
        }

        for (j = 0; j < CIRC_GRID; j++) {
            circuit[i][j] = buffer[j];
        }
        i++;*/

    fclose(file);
    return true;
}

/* TODO unsure about this function... could it be better? plus error handling
 * what about fgets etc.
 * TODO could be rewritten to write directly into matrix. 
 * QUESTION as above, what is more elegant?
 */
bool getCircuitLine(char line[], FILE* fp) {
    int c, i;

    for (i = 0; i < CIRC_GRID + 1; i++) {
        c = getc(fp);
        line[i] = (char)c;

        if (c != '\n') {
            line[i] = (char)c;
        }
        if (c == EOF) {
            if (ferror(fp)) {
                fprintf(stderr, "Error reading file\n");
                exit(EXIT_FAILURE);
            }
            return false;
        }
    }
    return true;
}

/* QUESTION this could just be integrated into the load functoin */
bool checkCircuit(char circuit[][CIRC_GRID]) {
    int i, j;

    for (i = 0; i < CIRC_GRID; i++) {
        for (j = 0; j < CIRC_GRID; j++) {
            if (checkSymbol(circuit[i][j]) == false) {
                /* TODO could check whole circuit to find all problems? */
                /* QUESTION how do variable argument functions work? */
                fprintf(stderr, "Invalid symbol used in circuit\n");
                return false;
            }
        }
    }
    return true;
}

bool checkSymbol(char c) {
    switch (c) {
        case 'H':
        case 't':
        case 'c':
        case ' ':
            return true;
            break;
    }
    return false;
}

void nextGeneration(circuit_struct* circuit) {
    int* x = &circuit->x;
    int* y = &circuit->y;

    for (*x = 0; *x < CIRC_GRID; (*x)++) {
        for (*y = 0; *y < CIRC_GRID; (*y)++) {
            circuit->next[*y][*x] = nextCellState(circuit);
        }
    }

    updateCurrent(circuit);
}

void updateCurrent(circuit_struct* circuit) {
    char(*tmp)[CIRC_GRID];

    tmp = circuit->current;
    circuit->current = circuit->next;
    circuit->next = tmp;
}

char nextCellState(circuit_struct* circuit) {
    char current = circuit->current[circuit->y][circuit->x];

    switch (current) {
        case 'H':
            return 't';
        case 't':
            return 'c';
        case 'c':;
            if (checkMooreNeighborhood(circuit) == true) {
                return 'H';
            }
            /* Fall through */
            /* QUESTION how to prevent fall through warning? */
        default:
            return current;
    }
}

void printCircuit(char circuit[][CIRC_GRID], int generation) {
    int i;
    if (generation == 0) {
        printf("STARTING CONFIGURATION\n\n");
    } else {
        printf("GENERATION: %4d of %d\n\n", generation, GENERATIONS);
    }
    /* QUESTION is this  a naughty way of doing this? */
    /* FIXME hard coded line bounds at the moment */
    for (i = 4; i < 15; i++) {
        printf("%.*s\n", CIRC_GRID, circuit[i]);
    }
}

/* QUESTION does c90 allow negative indexing? does the size in [] make any difference
 * QUESTION is it better to treat as contiguous memory or 2d array?
 */
/* Passing in parent grid size allows me to check circuit array directly and also test smaller matrices */
/* QUESTION have written function with testing in mind */
bool checkMooreNeighborhood(circuit_struct* circuit) {
    int i, j;
    int head_count = 0;
    int x = circuit->x;
    int y = circuit->y;

    /* TODO this is horrible at the moment.. needs to be updated */
    for (i = -1; i < 2; i++) {
        for (j = -1; j < 2; j++) {
            if (checkBounds(x + i, y + j) && !(i == 0 && j == 0)) {
                head_count += (circuit->current[y + j][x + i] == 'H');
            }
        }
    }

    return head_count == 1 || head_count == 2;
}

bool checkBounds(int x, int y) {
    return x >= 0 && x < CIRC_GRID &&
           y >= 0 && y < CIRC_GRID;
}

void test(void) {
}