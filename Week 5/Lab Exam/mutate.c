#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define N 100

void populateBoard(int grid[N][N]);
void printBoard(int grid[N][N]);
void mutate(int grid[N][N]);
void swap(int* n1, int* n2);
void generateSwaps(int* swap1, int* swap2);
int randCood();

int main(void) {
    int i;
    int grid[N][N];

    populateBoard(grid);
    for (i = 0; i < N * N * N; i++) {
        mutate(grid);
    }

    printBoard(grid);

    return 0;
}

void mutate(int grid[N][N]) {
    /* Choose line and column where horizontal and vertical swap will happen */
    int x_cood = randCood();
    int y_cood = randCood();
    int swap1;
    int swap2;

    /* Swap horizontal */
    generateSwaps(&swap1, &swap2);
    if (grid[y_cood][swap1] > grid[y_cood][swap2]) {
        swap(&grid[y_cood][swap1], &grid[y_cood][swap2]);
    }

    /* Swap vertical */
    generateSwaps(&swap1, &swap2);
    if (grid[swap1][x_cood] > grid[swap2][x_cood]) {
        swap(&grid[swap1][x_cood], &grid[swap2][x_cood]);
    }
}

void generateSwaps(int* swap1, int* swap2) {
    int cood1 = randCood();
    int cood2 = randCood();

    if (cood1 > cood2) {
        swap(&cood1, &cood2);
    }
    *swap1 = cood1;
    *swap2 = cood2;
}

void swap(int* n1, int* n2) {
    int tmp = *n1;
    *n1 = *n2;
    *n2 = tmp;
}

int randCood() {
    return rand() % N;
}

void populateBoard(int grid[N][N]) {
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            grid[i][j] = rand() % 10;
        }
    }
}

void printBoard(int grid[N][N]) {
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            printf("%d ", grid[i][j]);
        }
        printf("\n");
    }
}