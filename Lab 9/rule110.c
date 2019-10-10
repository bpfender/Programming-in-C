
#define NDEBUG
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define LINE_LENGTH 41
#define LINES_SHOWN 41

/* QUESTION order of functions? plus function naming to define helpers? */
void selectSeedLine(int line[]);
void randomSeedLine(int line[]);
void middleSeedLine(int line[]);

void calculateNextLine(int line[], int tmp_line[]);
void copyLine(int line[], int tmp[]);
void findParents(int line[], int index, int parents[]);
int newCellState(int parents[]);

void test(void);

int main(void) {
    /* QUESTION Is it better to initialise here or in the function calls? */
    int line[LINE_LENGTH];
    int tmp_line[LINE_LENGTH];
    int lines_shown, i;

    test();

    selectSeedLine(line);
    for (lines_shown = 0; lines_shown < LINES_SHOWN; lines_shown++) {
        for (i = 0; i < LINE_LENGTH; i++) {
            if (line[i] == 0) {
                printf("%s", "  ");
            } else {
                printf("%s", "[]");
            }
        }

        printf("\n");
        calculateNextLine(line, tmp_line);
    }

    return 0;
}

void selectSeedLine(int line[]) {
    int seed_select;
    printf(
        "How would you like the Automata to start?\n"
        "   1. Randomised line...\n"
        "   2. Single cell in the middle...  ");
    scanf("%d", &seed_select);

    if (seed_select == 1) {
        randomSeedLine(line);
    } else {
        middleSeedLine(line);
    }
}

void randomSeedLine(int line[]) {
    int i;
    for (i = 0; i < LINE_LENGTH; i++) {
        line[i] = rand() % 2;
    }
}

void middleSeedLine(int line[]) {
    int i;
    for (i = 0; i < LINE_LENGTH; i++) {
        line[i] = 0;
    }

    line[LINE_LENGTH / 2] = 1;
}

void calculateNextLine(int line[], int tmp_line[]) {
    int i;
    int parents[3];

    copyLine(line, tmp_line);

    for (i = 0; i < LINE_LENGTH; i++) {
        findParents(tmp_line, i, parents);
        line[i] = newCellState(parents);
    }
}

void copyLine(int line[], int tmp[]) {
    int i;
    for (i = 0; i < LINE_LENGTH; i++) {
        tmp[i] = line[i];
    }
}

/* QUESTION can i check this with asserts? */
void findParents(int line[], int index, int parents[]) {
    parents[1] = line[index];

    if (index == 0) {
        parents[0] = 0;
        parents[2] = line[index + 1];
    } else if (index == LINE_LENGTH - 1) {
        parents[0] = line[index - 1];
        parents[2] = 0;
    } else {
        parents[0] = line[index - 1];
        parents[2] = line[index + 1];
    }
}

int newCellState(int parents[]) {
    if ((parents[1] == 0 && parents[2] == 0) ||
        (parents[0] == 1 && parents[1] == 1 && parents[2] == 1)) {
        return 0;
    }
    return 1;
}

void test(void) {
    int array[] = {1, 1, 1};

    assert(newCellState(array) == 0);
}