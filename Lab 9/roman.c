#include <assert.h>
#include <ctype.h>
#include <stdio.h>

typedef enum bool { false,
                    true } bool;

int romanToArabic(char* roman);
int numeralValue(char roman);
int checkSecondNumeral(char numeral, char next_numeral, int* index);

int main(int argc, char** argv) {
    if (argc == 2) {
        printf("The roman number %s is equal to %d\n",
               argv[1], romanToArabic(argv[1]));
    } else {
        fprintf(stderr,
                "ERROR: Incorrect usage, try e.g. %s XXI\n", argv[0]);
    }

    return 0;
}

int checkSecondNumeral(char numeral, char next_numeral, int* index) {
    int multiplier;

    switch (toupper(numeral)) {
        case 'I':
            multiplier = 1;
            break;
        case 'X':
            multiplier = 10;
            break;
        case 'C':
            multiplier = 100;
            break;
    }

    switch (next_numeral) {
        case 'V':
        case 'L':
        case 'D':
            (*index)++;
            return 4 * multiplier;
        case 'X':
        case 'C':
        case 'M':
            (*index)++;
            return 9 * multiplier;
        default:
            return multiplier;
    }
}

/* QUESTION can this be shortened? */
int romanToArabic(char* roman) {
    int i;
    int sum = 0;

    for (i = 0; roman[i] != '\0'; i++) {
        switch (toupper(roman[i])) {
            case 'V':
                sum += 5;
                break;
            case 'L':
                sum += 50;
                break;
            case 'D':
                sum += 500;
                break;
            case 'M':
                sum += 1000;
                break;
            default: /* Check for second numeral for I, X and C */
                sum += checkSecondNumeral(roman[i], roman[i + 1], &i);
                break;
        }
    }
    return sum;
}

int numeralValue(char roman) {
    switch (roman) {
        case 'M':
            return 1000;
        case 'D':
            return 500;
        case 'C':
            return 100;
        case 'L':
            return 50;
        case 'X':
            return 10;
        case 'V':
            return 5;
        case 'I':
            return 1;
        default:
            fprintf(stderr, "%c is not a valid roman numeral\n", roman);
            return 0;
    }
}

void test(void) {
}