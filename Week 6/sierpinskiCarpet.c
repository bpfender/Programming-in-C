#include <assert.h>
#include <stdio.h>

#define SIERP_SIZE 81

void test(void);
void sierpinski(char gridp[][SIERP_SIZE], int sub_size);

int main(void) {
    test();
    return 0;
}

void test(void) {
    int i, j;
    char grid[SIERP_SIZE][SIERP_SIZE];

    for (i = 0; i < SIERP_SIZE; i++) {
        for (j = 0; j < SIERP_SIZE; j++) {
            grid[i][j] = '0';
        }
    }

    sierpinski(grid, SIERP_SIZE);

    for (i = 0; i < SIERP_SIZE; i++) {
        for (j = 0; j < SIERP_SIZE; j++) {
            if (grid[i][j] == '0') {
                printf("  ");
            } else {
                printf("[]");
            }
        }
        printf("\n");
    }
}

/* QUESTION, i feel like this is quite elegant but obvious errors.. */
void sierpinski(char grid[][SIERP_SIZE], int sub_size) {
    int i, j;

    sub_size /= 3;
    printf("Sub size %d\n", sub_size);

    for (i = sub_size; i < sub_size * 2; i++) {
        for (j = sub_size; j < sub_size * 2; j++) {
            grid[i][j] = '1';
        }
    }

    if (sub_size == 1) {
        return;
    }

    for (i = 0; i < sub_size * 3; i += sub_size) {
        for (j = 0; j < sub_size * 3; j += sub_size) {
            if (!(i == sub_size && j == sub_size)) {
                sierpinski(&grid[i][j], sub_size);
            }
        }
    }
}