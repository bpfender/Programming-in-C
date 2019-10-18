#include <assert.h>
#include <stdio.h>

/* TODO, could define as 42 to ensure bounding cells or just add some padding? */
#define CIRC_GRID 40
#define MOORE_GRID 3

char nextCellState(char state[][CIRC_GRID]);
void nextGeneration(char circuit[][CIRC_GRID]);
int checkMooreNeighborhood(char state[][MOORE_GRID]);
void test(void);

int main(void) {
    test();
    return 0;
}

void nextGeneration(char circuit[][CIRC_GRID]) {
    int i, j;
    char tmp[CIRC_GRID][CIRC_GRID];

    /* QUESTION as before, what is the best way to handle 2D arrays? */
    for (i = 0; i < CIRC_GRID; i++) {
        for (j = 0; j < CIRC_GRID; j++) {
            /* TODO, do i need the tmp array to avoid funny behaviour? */
            tmp[i][j] = nextCellState(circuit[i][j]);
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
char nextCellState(char state[][CIRC_GRID]) {
    int heads;

    /* FIXME: don't think this is the right syntax yet */
    switch (**state) {
        case 'H':
            return 't';
            break;
        case 't':
            return 'c';
            break;
        case 'c':
            /* TODO need to check surrounding cells here */
            /* TODO how to handle larger to smaller 2D array? */
            heads = checkMooreNeighborhood(state);
            if (heads == 1 || heads == 2) {
                return 'H';
            }
        default:
            return *state;
            break;
    }
}

/* TODO need to check bounding is valid don't go out of index
 * QUESTION does c90 allow negative indexing? does the size in [] make any difference
 * QUESTION is it better to treat as contiguous memory or 2d array?
 * TODO, better to use whole CIRC_GRID for easier indexing, but harder testing
 */
int checkMooreNeighborhood(char state[][MOORE_GRID]) {
    int i, j;
    int head_count = 0;

    for (i = -1; i < 1; i++) {
        for (j = -1; j < 1; j++) {
            if (!(i == 1 && j == 1)) {
                head_count += (state[i][j] == 'H');
            }
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

    moore_state = &moore_test_1[1][1];
    assert(checkMooreNeighborhood(moore_state) == 1);
    moore_state = &moore_test_2[1][1];
    assert(checkMooreNeighborhood(moore_state) == 2);
    moore_state = &moore_test_3[1][1];
    assert(checkMooreNeighborhood(moore_state) == 0);
}