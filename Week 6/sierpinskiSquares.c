#include <stdio.h>

#define SIZE 8

void sierpinskiTriangle(char grid[SIZE][SIZE], int sub_size);
void test(void);

int main(void) {
    test();
    return 0;
}

void sierpinskiTriangle(char grid[SIZE][SIZE], int sub_size) {
    int i, j;

    sub_size /= 2;

    for (i = 0; i < sub_size; i++) {
        for (j = 0; j < sub_size; j++) {
            grid[i][j] = '1';
        }
    }

    if (sub_size == 1) {
        return;
    }

    sierpinskiTriangle(&grid[0][sub_size], sub_size);
    sierpinskiTriangle(&grid[sub_size][0], sub_size);
    sierpinskiTriangle(&grid[sub_size][sub_size], sub_size);
}

void test(void) {
    int i, j;
    char grid[SIZE][SIZE];

    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            grid[i][j] = '0';
        }
    }
    sierpinskiTriangle(grid, SIZE);

    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            if (grid[i][j] == '0') {
                printf("[]");
            } else {
                printf("  ");
            }
        }
        printf("\n");
    }
}