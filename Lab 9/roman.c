#include <stdio.h>

int romanToArabic(char* roman);

int main(int argc, char** argv) {
    if (argc == 2) {
        printf("The roman number %s is equal to %d\n",
               argv[1], romanToArabic(argv[1]));
    } else {
        printf("ERROR: Incorrect usage, try e.g. %s XXI\n", argv[0]);
    }

    return 0;
}

/* QUESTION can this be shortened? */
int romanToArabic(char* roman) {
    int i = 0;
    int sum = 0;

    while (*(roman + i) != '\0') {
        switch (roman[i]) {
            case 'I':
                if (roman[i + 1] == 'V') {
                    sum += 4;
                    i++;
                } else if (roman[i + 1] == 'X') {
                    sum += 9;
                    i++;
                } else {
                    sum += 1;
                }
                break;
            case 'V':
                sum += 5;
                break;
            case 'X':
                if (roman[i + 1] == 'L') {
                    sum += 40;
                    i++;
                } else if (roman[i + 1] == 'C') {
                    sum += 90;
                    i++;
                } else {
                    sum += 10;
                }
                break;
            case 'L':
                sum += 50;
                break;
            case 'C':
                if (roman[i + 1] == 'D') {
                    sum += 400;
                    i++;
                } else if (roman[i + 1] == 'M') {
                    sum += 900;
                    i++;
                } else {
                    sum += 100;
                }
                break;
            case 'D':
                sum += 500;
                break;
            case 'M':
                sum += 1000;
                break;
        }
        i++;
    }
    return sum;
}