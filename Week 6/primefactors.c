#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define START_PRIME 2
#define PRIME_LIST_SIZE 10

typedef struct prime_factor {
    long val;
    long count;
} prime_factor;

typedef struct prime_list {
    prime_factor* list;
    size_t size;
} prime_list;

void primeFactors(unsigned long n, unsigned long prime);
unsigned long nextPrime(unsigned long prime);
unsigned long isPrime(unsigned long candidate);
void test(void);

int main(void) {
    test();

    return 0;
}

void primeFactors(unsigned long n, unsigned long prime) {
    if (n % prime == 0) {
        n /= prime;

        printf("%li ", prime);
        if (n == 1) {
            printf("\n");
            return;
        }
    } else {
        prime = nextPrime(prime);
    }

    primeFactors(n, prime);
}

unsigned long nextPrime(unsigned long prime) {
    while (!isPrime(++prime)) {
    }

    return prime;
}

unsigned long isPrime(unsigned long candidate) {
    unsigned long i;

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
    primeFactors(1532579688, START_PRIME);
    primeFactors(38654705664, START_PRIME);
}