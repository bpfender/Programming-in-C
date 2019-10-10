#include <math.h>
#include <stdio.h>

#include <assert.h>

#define PI 3.141592653

float nextLeibniz(int divisor);
float iterateLeibniz(float current);

int main(void) {
    int loop = 0;
    float pi_approx = 0;

    /* QUESTION is this the right way of specifying the decimal places? */
    while (fabs(PI - pi_approx) > 0.000001) {
        pi_approx = iterateLeibniz(pi_approx);
        loop++;
        printf("Iteration #%d: Pi is %.20f\n", loop, pi_approx);
    }
    return 0;
}

float nextLeibniz(int divisor) {
    return (float)4 / divisor;
}

float iterateLeibniz(float current) {
    static int iteration = 0;
    int divisor;

    iteration++;
    divisor = iteration * 2 - 1;

    if (iteration % 2 != 0) {
        return current + nextLeibniz(divisor);
    } else {
        return current - nextLeibniz(divisor);
    }
}
