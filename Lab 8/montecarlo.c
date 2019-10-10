#include <stdio.h>
#include <stdlib.h>

#define RADIUS 1

typedef enum bool { false,
                    true } bool;

float generateCoord();
bool MonteCarloFinder(float x_coord, float y_coord);
void MonteCarloIterator(int iterations);

int main(void) {
    float x_coord;
    float y_coord;
    int i, iterations;
    int attempts, hits = 0;

    printf("How many iterations do you want to do?  ");
    scanf("%d", &iterations);
    /* QUESTION is there a neat way to do error ahdnlign on scanf */

    for (i = 0; i < iterations; i++) {
        attempts = i + 1;

        x_coord = generateCoord();
        y_coord = generateCoord();

        if (MonteCarloFinder(x_coord, y_coord)) {
            hits++;
        }
        printf("Iteration #%d: Pi approximation is %f\n",
               attempts, (float)4 * hits / attempts);
    }

    return 0;
}

float generateCoord() {
    return (float)rand() / RAND_MAX;
    /* QUESTION do i have to cast here to return a float? */
}

/* QUESTION should i just link in th emaths library? */
bool MonteCarloFinder(float x_coord, float y_coord) {
    if (x_coord * x_coord + y_coord * y_coord <= RADIUS * RADIUS) {
        return 1;
    }
    return 0;
}
