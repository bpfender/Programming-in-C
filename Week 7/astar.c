#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define SIZE 3
#define MAX_CHILDREN 4

typedef struct node_t {
    int grid[SIZE][SIZE];
    size_t f, g, h;
    size_t x, y;
    struct node_t* parent;
} node_t;

int fPriority(int grid[SIZE][SIZE], size_t step);
int manhattanDistance(int grid[SIZE][SIZE]);
int hammingDistance(int grid[SIZE][SIZE]);

void test(void);

int main(void) {
    test();
    return 0;
}

/* FIXME size_t vs int */
int fPriority(int grid[SIZE][SIZE], size_t step) {
    return manhattanDistance(grid) + (int)step;
}

int manhattanDistance(int grid[SIZE][SIZE]) {
    int i, j;
    int manhattan = 0;
    int num;
    printf("MANHATTAN\n");
    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            num = grid[i][j] - 1;
            /* FIXME definition doesn't ignore 0 */
            manhattan += abs(num / 3 - i) + abs(num % 3 - j);
            printf("%d\n", manhattan);
        }
    }

    return manhattan;
}

int hammingDistance(int grid[SIZE][SIZE]) {
    int i, j;
    int hamming = 0;

    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            /* FIXME different definition of hamming distance elsewhere */
            hamming += (grid[i][j] != (int)(3 * i + j + 1) % (SIZE * SIZE));
            printf("%d\n", hamming);
        }
    }
    return hamming;
}

void test(void) {
    int i, j;
    int test_grid[SIZE][SIZE] = {{8, 1, 3}, {4, 0, 2}, {7, 6, 5}};

    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            printf("%d", test_grid[i][j]);
        }
        printf("\n");
    }

    assert(hammingDistance(test_grid) == 6);
    assert(manhattanDistance(test_grid) == 10);
}