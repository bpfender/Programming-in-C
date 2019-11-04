#include <stdio.h>

#define NDEBUG
#include <assert.h>

#define TEST_SET 200000
unsigned int nextHailstone(unsigned int current);
unsigned int HailstoneLength(unsigned int seed);
unsigned int HailstoneLargest(unsigned int seed);
void test(void);

int main(void) {
    unsigned int i;
    unsigned int current_length, longest_found = 0, longest_seed;
    unsigned int current_largest, largest_found = 0, largest_seed;

    test();

    for (i = 1; i <= TEST_SET; i++) {
        if ((current_length = HailstoneLength(i)) > longest_found) {
            longest_seed = i;
            longest_found = current_length;
        }
        if ((current_largest = HailstoneLargest(i)) > largest_found) {
            largest_seed = i;
            largest_found = current_largest;
        }
    }

    printf(
        "The seed (1-%d) "
        "for the longest Hailstone sequence is %d\n",
        TEST_SET, longest_seed);
    printf(
        "The seed (1-%d) "
        "for the largest number to appear in the Hailstone sequence is %d\n",
        TEST_SET, largest_seed);

    return 0;
}

/* Calculate next hailstone number */
unsigned int nextHailstone(unsigned int current) {
    if (current % 2 == 0) {
        return current /= 2;
    } else {
        return current * 3 + 1;
    }
}

/* Return the current_length    of the hailstone sequence */
unsigned int HailstoneLength(unsigned int seed) {
    if (seed == 1) {
        return 1;
    }
    return HailstoneLength(nextHailstone(seed)) + 1;
}

/* Return the seed of the largest number in the hailstone sequence */
unsigned int HailstoneLargest(unsigned int seed) {
    unsigned int largest;

    if (seed == 1) {
        return 1;
    }
    if (seed > (largest = HailstoneLargest(nextHailstone(seed)))) {
        return seed;
    } else {
        return largest;
    }
}

void test(void) {
    assert(HailstoneLength(6) == 9);
    assert(HailstoneLength(11) == 15);
    assert(HailstoneLargest(11) == 52);
    assert(HailstoneLargest(6) == 16);
}