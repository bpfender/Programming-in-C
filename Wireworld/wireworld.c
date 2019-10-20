#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum bool { false,
                    true } bool;

/* TODO, could define as 42 to ensure bounding cells or just add some padding? */
#define CIRC_GRID 40
#define MOORE_GRID 3

void loadCircuit(char* filename, char circuit[][CIRC_GRID]);
char nextCellState(char state[][CIRC_GRID], int i, int j);
void nextGeneration(char circuit[][CIRC_GRID]);
int checkMooreNeighborhood(char* state);
bool checkSymbol(char c);
int getLine(char buffer[], int buffer_size, FILE* fp);
void test(void);

int main(void) {
    test();

    return 0;
}

void loadCircuit(char* filename, char circuit[][CIRC_GRID]) {
    FILE* file;
    char buffer[CIRC_GRID + 1];
    int length;
    int i = 0, j;

    /* TODO more error handling aorund incorrect filenames? length etc. */
    file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Cannot open %s\n", filename);
        exit(EXIT_FAILURE);
    }

    while (length = (getLine(buffer, CIRC_GRID + 1, file))) {
        if (length != 40) {
            fprintf(stderr,
                    "Circuit width in line %d is not %d cells wide",
                    i, CIRC_GRID);
            exit(EXIT_FAILURE);
        }

        printf("%s %d %d\n", buffer, i, j);
        i++;
    }

    fclose(file);
}

bool checkCircuit(char circuit[][CIRC_GRID]) {
    int i, j;

    for (i = 0; i < CIRC_GRID; i++) {
        for (j = 0; j < CIRC_GRID; j++) {
            if (!checkSymbol(circuit[i][j])) {
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

/* TODO unsure about this function... could it be better? plus error handling
 * what about fgets etc.
 * TODO could be rewritten to write directly into matrix
 */
int getLine(char buffer[], int buffer_size, FILE* fp) {
    int c, i;

    for (i = 0; i < buffer_size; i++) {
        c = getc(fp);
        buffer[i] = (char)c;

        if (c == '\n' || i == buffer_size - 1) {
            buffer[i] = '\0';
            return i;
        }
        if (c == EOF) {
            if (ferror(fp)) {
                fprintf(stderr, "Error reading file\n");
                exit(EXIT_FAILURE);
            }
            return 0;
        }
    }
}

void nextGeneration(char circuit[][CIRC_GRID]) {
    int i, j;
    char tmp[CIRC_GRID][CIRC_GRID];

    /* QUESTION as before, what is the best way to handle 2D arrays? */
    for (i = 0; i < CIRC_GRID; i++) {
        for (j = 0; j < CIRC_GRID; j++) {
            /* TODO, do i need the tmp array to avoid funny behaviour? */
            tmp[i][j] = nextCellState(circuit, i, j);
        }
    }

    /* TODO: can repeated for loop structure be cleaned up? */
    for (i = 0; i < CIRC_GRID; i++) {
        for (j = 0; j < CIRC_GRID; j++) {
            circuit[i][j] = tmp[i][j];
        }
    }
}

/* QUESTION should i include break if I return in a switch */
char nextCellState(char state[][CIRC_GRID], int i, int j) {
    int heads;

    /* FIXME: don't think this is the right syntax yet */
    switch (state[i][j]) {
        case 'H':
            return 't';
            break;
        case 't':
            return 'c';
            break;
        case 'c':
            /* TODO need to check surrounding cells here */
            /* TODO how to handle larger to smaller 2D array? */
            heads = 1;
            if (heads == 1 || heads == 2) {
                return 'H';
            }
            /* QUESTION how to prevent fall through warning? */
        default:
            return state[i][j];
            break;
    }
}

/* TODO need to check bounding is valid don't go out of index
 * QUESTION does c90 allow negative indexing? does the size in [] make any difference
 * QUESTION is it better to treat as contiguous memory or 2d array?
 * TODO, better to use whole CIRC_GRID for easier indexing, but harder testing
 * TODO currently expects pointer to center of 3x3 grid
 */
int checkMooreNeighborhood(char* state) {
    int i;
    int head_count = 0;

    /* TODO this is horrible at the moment.. needs to be updated */
    for (i = -4; i < 5; i++) {
        if (i != 0) {
            head_count += (state[i] == 'H');
        }
    }
    return head_count;
}

void test(void) {
    char moore_test_1[][MOORE_GRID] = {{' ', ' ', ' '},
                                       {'H', 'c', 'c'},
                                       {' ', ' ', ' '}};

    char moore_test_2[][MOORE_GRID] = {{'H', ' ', ' '},
                                       {'H', 'c', 'c'},
                                       {' ', ' ', ' '}};

    char moore_test_3[][MOORE_GRID] = {{' ', ' ', 'c'},
                                       {'c', 'c', 'c'},
                                       {'c', ' ', ' '}};

    char* moore_state = NULL;

    char filename[] = "wirewcircuit1.txt";
    char circuit[CIRC_GRID][CIRC_GRID];

    moore_state = &moore_test_1[1][1];
    assert(checkMooreNeighborhood(moore_state) == 1);
    moore_state = &moore_test_2[1][1];
    assert(checkMooreNeighborhood(moore_state) == 2);
    moore_state = &moore_test_3[1][1];
    assert(checkMooreNeighborhood(moore_state) == 0);

    loadCircuit(filename, circuit);
}