#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum bool { false,
                    true } bool;

/* Functions to calculate values */
int romanToArabic(char* roman);
int* createDecimals(char* roman);
int calculateSum(int* decimal, int size);
int numeralValue(char roman);
/* Functions to do error checking on input */
bool validRoman(char* roman, int* decimal);
bool checkRepetition(char* roman);
bool checkUnique(char* roman);
bool checkOrder(char* roman, int* decimal);
int checkDoubleNumeral(int* decimal);

void test(void);

int main(int argc, char* argv[]) {
    /*test();*/

    if (argc == 2) {
        printf("The roman number %s is equal to %d\n",
               argv[1], romanToArabic(argv[1]));
    } else {
        fprintf(stderr,
                "ERROR: Incorrect usage, try e.g. %s XXI\n", argv[0]);
    }

    return 0;
}

/*----- CALCULATION FUNCTIONS ------*/
int romanToArabic(char* roman) {
    int* decimal = NULL;
    int sum;

    if ((decimal = createDecimals(roman)) == NULL) {
        fprintf(stderr, "Exiting program...\n");
        free(decimal);
        exit(1);
    }
    if (!validRoman(roman, decimal)) {
        free(decimal);
        exit(2);
    }

    sum = calculateSum(decimal, strlen(roman));
    free(decimal);
    return sum;
}

/* Assigns memory to store decimal values of roman numerals. On error (malloc
 * fail or invalid numeral) returns NULL. Otherwise, returns pointer to memory
 */
int* createDecimals(char* roman) {
    int i;
    int size = strlen(roman); /*TODO check this shouldn't be -1*/
    int* decimals = NULL;

    decimals = (int*)malloc(size * sizeof(int));
    if (decimals == NULL) {
        fprintf(stderr, "Memory allocation failed...\n");
        return NULL;
    }

    for (i = 0; i < size; i++) {
        decimals[i] = numeralValue(roman[i]);
        if (decimals[i] == -1) {
            free(decimals);
            return NULL;
        }
    }
    return decimals;
}

/* Assumes valid numeral input (based on user input checking). Calculates
 * decimal value of roman numeral
 */
int calculateSum(int* decimal, int size) {
    int i = 0;
    int sum = decimal[size - 1];

    for (i = size - 2; i >= 0; i--) {
        if (decimal[i] < decimal[i + 1]) {
            sum -= decimal[i];
        } else {
            sum += decimal[i];
        }
    }
    return sum;
}

/* Maps roman rumeral to decimal value. Returns -1 if invalid
 */
int numeralValue(char roman) {
    switch (toupper(roman)) {
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
            return -1;
    }
}

/*------ INPUT VALIDATION FUNCTIONS ------*/
bool validRoman(char* roman, int* decimal) {
    if (checkRepetition(roman) == true &&
        checkOrder(roman, decimal) == true &&
        checkUnique(roman) == true) {
        return true;
    }
    return false;
}

/* Checks that any one numeral doesn't appear more than three times in a row
 */
bool checkRepetition(char* roman) {
    int i;
    int count = 1;
    for (i = 1; roman[i] != '\0'; i++) {
        if (roman[i - 1] == roman[i]) {
            count++;
            if (count > 3) {
                fprintf(stderr,
                        "%c is repeated too many times in a row\n",
                        roman[i]);
                return false;
            }
        } else {
            count = 1;
        }
    }
    return true;
}

/* Checks that V, L or D appear only once in roman numeral. (Two instances of 
 * 5, 50 or 500 can just be expressed with 10, 100, 1000 etc)
 */
bool checkUnique(char* roman) {
    int i;
    int count_V = 0;
    int count_L = 0;
    int count_D = 0;

    for (i = 0; roman[i] != '\0'; i++) {
        if (roman[i] == 'V') {
            count_V++;
        } else if (roman[i] == 'L') {
            count_L++;
        } else if (roman[i] == 'D') {
            count_D++;
        }
    }

    if (count_V > 1 || count_L > 1 || count_D > 1) {
        fprintf(stderr, "V, L and D should only appear once in numeral\n");
        return false;
    }
    return true;
}

/* Checks that roman numerals are in a valid order 
 */
bool checkOrder(char* roman, int* decimal) {
    int i;
    int dbl_value, dbl_comp;
    int length = strlen(roman);

    for (i = 0; i < length - 1; i++) {
        if (decimal[i] < decimal[i + 1]) {
            /* Check for valid double numeral */
            if ((dbl_value = checkDoubleNumeral(decimal + i))) {
                /* Check that a second double numeral won't go out of bounds */
                if (i < length - 3) {
                    /* Check that there is a valid second double numeral */
                    if ((dbl_comp = checkDoubleNumeral(decimal + i + 2))) {
                        /* If the second double numeral is larger in value or
                         * the first numeral of each double numeral are the same
                         *  it's an invalid input
                         */
                        if (dbl_comp >= dbl_value ||
                            decimal[i] == decimal[i + 2]) {
                            fprintf(stderr,
                                    "%c%c may not be followed by %c%c\n",
                                    roman[i], roman[i + 1],
                                    roman[i + 2], roman[i + 3]);
                            return false;
                        }
                    }
                }
                /* Check that an additional numeral won't go out of bounds */
                if (i < length - 2) {
                    /* If the next numeral falls in the range between the first
                     * and second numeral of the double numeral it's an invalid 
                     * input
                     */
                    if (decimal[i] <= decimal[i + 2] &&
                        decimal[i + 2] <= decimal[i + 1]) {
                        fprintf(stderr, "%c may not follow %c%c\n",
                                roman[i + 2], roman[i], roman[i + 1]);
                        return false;
                    }
                }
                /* Check there is enough space for a preceeding numeral */
                if (i > 0) {
                    /* If the preceeding numeral is smaller in value than 
                    * the double numeral, it's an invalid input
                    */
                    if (dbl_value > decimal[i - 1]) {
                        fprintf(stderr, "%c may not preceed %c%c\n",
                                roman[i - 1], roman[i], roman[i + 1]);
                        return false;
                    }
                }
            } else {
                fprintf(stderr,
                        "%c may not preceed %c\n",
                        roman[i], roman[i + 1]);
                return false;
            }
        }
    }
    return true;
}

/* Check that numeral combination is a valid double numeral IV, IX, XL, XC, CD
 * or CM. Assumes valid input (i.e. can index 1 forward because this is checked
 * in checkOrder function)
 */
int checkDoubleNumeral(int* decimal) {
    if (decimal[0] == 1 || decimal[0] == 10 || decimal[0] == 100) {
        if (decimal[0] * 5 == decimal[1] || decimal[0] * 10 == decimal[1]) {
            return decimal[1] - decimal[0];
        }
    }
    return 0;
}

void test(void) {
    int i;

    /* Pointer to store converted decimal valued */
    int* dec_test = NULL;

    /* Decimal conversion test, valid and validation set first, invalid second */
    char rom_dec_1[] = "MDcLXVI";
    int dec_valid[] = {1000, 500, 100, 50, 10, 5, 1};
    char rom_dec_2[] = "MDLg";

    /* Double numeral test cases. Valid and test set, followed by invalid */
    char dbl_rom_1[][3] = {"IV", "IX", "XL", "XC", "CD", "CM"};
    int dbl_dec_1[] = {4, 9, 40, 90, 400, 900};
    char dbl_rom_2[][3] = {"IL", "VX", "XD", "XM", "DC", "LM"};

    /* Repetition test cases, invalid first, valid second */
    char rep_rom_1[][10] = {"IIII", "IVVVV", "CMXIIII", "XXXX", "CCCC"};
    char rep_rom_2[][10] = {"VIII", "XX", "CLX", "MDXXX", "CC"};

    /* Unique value test case, invalid first, valid second */
    char unq_rom_1[][10] = {"VV", "VLLL", "DDDD", "XIVIV", "MCDD"};
    char unq_rom_2[][10] = {"IV", "LXI", "DLV", "IXV", "MDCLXVI"};

    /* Numeral order test cases, invalid first, valid second */
    char ord_rom_1[][10] = {"IXIX", "IXIV", "CMCDII", "CIL", "MMCMCD", "IXI"};
    char ord_rom_2[][10] = {"XCIV", "CMXCIV", "XIV", "CDXLV", "VIII", "MCDXLVI"};

    char sum_rom[][10] = {"MCMXCIX", "MCMLXVII", "MCDXCI"};
    int sum_dec[] = {1999, 1967, 1491};

    assert(numeralValue('I') == 1);
    assert(numeralValue('V') == 5);
    assert(numeralValue('X') == 10);
    assert(numeralValue('L') == 50);
    assert(numeralValue('c') == 100);
    assert(numeralValue('d') == 500);
    assert(numeralValue('m') == 1000);
    assert(numeralValue('G') == -1);

    dec_test = createDecimals(rom_dec_1);
    for (i = 0; i < (int)strlen(rom_dec_1); i++) {
        assert(dec_test[i] == dec_valid[i]);
    }
    free(dec_test);
    assert(createDecimals(rom_dec_2) == NULL);

    for (i = 0; i < 6; i++) {
        dec_test = createDecimals(dbl_rom_1[i]);
        assert(checkDoubleNumeral(dec_test) == dbl_dec_1[i]);
        free(dec_test);

        dec_test = createDecimals(dbl_rom_2[i]);
        assert(checkDoubleNumeral(dec_test) == 0);
        free(dec_test);
    }

    for (i = 0; i < 5; i++) {
        assert(checkRepetition(rep_rom_1[i]) == false);
        assert(checkRepetition(rep_rom_2[i]) == true);
        assert(checkUnique(unq_rom_1[i]) == false);
        assert(checkUnique(unq_rom_2[i]) == true);
    }

    for (i = 0; i < 6; i++) {
        dec_test = createDecimals(ord_rom_1[i]);
        assert(checkOrder(ord_rom_1[i], dec_test) == false);
        free(dec_test);
        dec_test = createDecimals(ord_rom_2[i]);
        assert(checkOrder(ord_rom_2[i], dec_test) == true);
        free(dec_test);
    }

    for (i = 0; i < 3; i++) {
        dec_test = createDecimals(sum_rom[i]);
        assert(calculateSum(dec_test, strlen(sum_rom[i])) == sum_dec[i]);
        free(dec_test);
    }
}
