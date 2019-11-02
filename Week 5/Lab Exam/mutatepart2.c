#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define N 21

void populateBoard(int grid[N][N]);
void printBoard(int grid[N][N]);
void mutate(int grid[N][N]);
void swap(int* n1, int* n2);
int centreDistanceSq(int x, int y);
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
    int x1_cood = randCood();
    int y1_cood = randCood();
    int x2_cood = randCood();
    int y2_cood = randCood();
    int dist1 = centreDistanceSq(x1_cood, y1_cood);
    int dist2 = centreDistanceSq(x2_cood, y2_cood);

    if (dist1 > dist2) {
        swap(&x1_cood, &x2_cood);
        swap(&y1_cood, &y2_cood);
    }

    if (grid[y1_cood][x1_cood] > grid[y2_cood][x2_cood]) {
        swap(&grid[y1_cood][x1_cood], &grid[y2_cood][x2_cood]);
    }
}

int centreDistanceSq(int x, int y) {
    int xsq = (x - (N - 1) / 2);
    int ysq = (y - (N - 1) / 2);
    xsq *= xsq;
    ysq *= ysq;

    return xsq + ysq;
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