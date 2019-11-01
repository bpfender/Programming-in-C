#include <assert.h>
#include <stdio.h>

#define SIZE 5
#define PATTERN_LENGTH 5

void findPath(int grid[SIZE][SIZE], int x, int y, int length);
int checkBounds(int x, int y);
void test(void);

int main(void) {
    test();
    return 0;
}

void findPath(int grid[SIZE][SIZE], int x, int y, int length) {
    int i, j;

    grid[y][x] = PATTERN_LENGTH - length + 1;

    if (length == 1) {
        for (i = 0; i < SIZE; i++) {
            for (j = 0; j < SIZE; j++) {
                printf("%2d ", grid[i][j]);
            }
            printf("\n");
        }
        printf("\n");
        return;
    }

    if (checkBounds(x + 1, y)) {
        if (grid[y][x + 1] == 0) {
            findPath(grid, x + 1, y, length - 1);
            grid[y][x + 1] = 0;
        }
    }

    if (checkBounds(x - 1, y)) {
        if (grid[y][x - 1] == 0) {
            findPath(grid, x - 1, y, length - 1);
            grid[y][x - 1] = 0;
        }
    }

    if (checkBounds(x, y + 1)) {
        if (grid[y + 1][x] == 0) {
            findPath(grid, x, y + 1, length - 1);
            grid[y + 1][x] = 0;
        }
    }

    if (checkBounds(x, y - 1)) {
        if (grid[y - 1][x] == 0) {
            findPath(grid, x, y - 1, length - 1);
            grid[y - 1][x] = 0;
        }
    }
}

int checkBounds(int x, int y) {
    return (x >= 0 && x < SIZE) &&
           (y >= 0 && y < SIZE);
}

void test(void) {
    int i, j;

    int test_grid[SIZE][SIZE] = {0};

    test_grid[0][0] = 1;

    findPath(test_grid, 0, 0, PATTERN_LENGTH);

    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            printf("%d ", test_grid[i][j]);
        }
        printf("\n");
    }
}
