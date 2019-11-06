#include <assert.h>
#include <stdio.h>

#define START_PRIME 2

typedef struct primes {
    int val;
    int count;
} primes;

void primeFactors(int n, int prime);
int nextPrime(int prime);
int isPrime(int candidate);
void test(void);

int main(void) {
    test();
    return 0;
}

int primeDivisor() {
    return 0;
}

void primeFactors(int n, int prime) {
    if (n % prime == 0) {
        n /= prime;
        printf("%d ", prime);
        if (n == 1) {
            printf("\n");
            return;
        }
    } else {
        prime = nextPrime(prime);
    }

    primeFactors(n, prime);
}

int nextPrime(int prime) {
    while (!isPrime(++prime)) {
    }

    return prime;
}

int isPrime(int candidate) {
    int i;

    if (candidate == 2) {
        return 1;
    }
    if (candidate < 2 || candidate % 2 == 0) {
        return 0;
    }
    for (i = 3; i <= candidate / 2; i += 2) {
        if (candidate % i == 0) {
            return 0;
        }
    }
    return 1;
}

void test(void) {
    assert(nextPrime(2) == 3);
    assert(nextPrime(3) == 5);
    assert(nextPrime(7) == 11);

    primeFactors(210, START_PRIME);
    primeFactors(117, START_PRIME);
    primeFactors(1532579687, START_PRIME);
}