#include <stdio.h>

#define TRIANGLE_ITERATIONS 50

/* TODO This is not pretty at all at the moment */

int nextTriangle(int current_triangle, int iteration);
int findNthTriangle(int n);

int main(void) {
    int i;
    int current_triangle = 1;

    for (i = 0; i < TRIANGLE_ITERATIONS; i++) {
        printf("Next Triangle: %4d    ", current_triangle);
        printf("Nth Triangle: %4d\n", findNthTriangle(i + 1));
        current_triangle = nextTriangle(current_triangle, i + 2);
    }
    return 0;
}

int nextTriangle(int current_triangle, int iteration) {
    return current_triangle + iteration;
}

int findNthTriangle(int n) {
    return n * (n + 1) / 2;
}