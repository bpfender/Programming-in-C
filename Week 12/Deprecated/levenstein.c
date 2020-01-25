#include <stdlib.h>

#define STR_END(s) (s == '\0' || s == '\n')

/* FIXME super dirty cost definition at the moment */
#define MATR_SIZE 500

size_t calculateMSD(char* sample, char* data, size_t cost[MATR_SIZE][MATR_SIZE]);
size_t getMin(size_t a, size_t b, size_t c);

/* Based on https://en.wikipedia.org/wiki/Wagner%E2%80%93Fischer_algorithm
 * and https://en.wikipedia.org/wiki/Levenshtein_distance
 */
size_t calculateMSD(char* sample, char* data, size_t cost[MATR_SIZE][MATR_SIZE]) {
    size_t i, j;
    int substitution;

    /* Initialise top left corner of cost array */
    cost[0][0] = 0;

    for (i = 1; !STR_END(sample[i - 1]); i++) {
        cost[i][0] = i;
    }
    for (j = 1; !STR_END(data[j - 1]); j++) {
        cost[0][j] = j;
    }
    for (j = 1; !STR_END(data[j - 1]); j++) {
        for (i = 1; !STR_END(sample[i - 1]); i++) {
            substitution = (data[j - 1] == sample[i - 1]) ? 0 : 1;

            cost[i][j] = getMin(cost[i - 1][j] + 1,
                                cost[i][j - 1] + 1,
                                cost[i - 1][j - 1] + substitution);
        }
    }

    /* Remove increment from last instance of for loop */
    return cost[i - 1][j - 1];
}

size_t getMin(size_t a, size_t b, size_t c) {
    size_t tmp = a < b ? a : b;
    return tmp < c ? tmp : c;
}
