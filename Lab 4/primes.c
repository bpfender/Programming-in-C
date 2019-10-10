#include <stdio.h>

int isPrime(int candidate);

int main(void) {
    int i, num;
    int prime_found = 0, candidate = 1;

    printf("How many primes do you want to print? ");
    scanf("%i", &num);

    for (i = 0; i < num; i++) {
        while (!prime_found) {
            if (isPrime(candidate)) {
                printf("%i\n", candidate);
                prime_found = 1;
            }
            candidate++;
        }
        prime_found = 0;
    }

    return 0;
}

int isPrime(int candidate) {
    int j;

    if (candidate == 2) {
        return 1;
    }
    if (candidate < 2 || candidate % 2 == 0) {
        return 0;
    }
    for (j = 3; j <= candidate / 2; j += 2) {
        if (candidate % j == 0) {
            return 0;
        }
    }
    return 1;
}