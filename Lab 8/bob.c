#include <ctype.h>
#include <stdio.h>

#define NDEBUG
#include <assert.h>

#define letterVal(X) X - 'a' + 1

typedef enum bool { false,
                    true } bool;

void findPlanetBob(void);
bool validNameStructure(char name[]);
bool isVowel(char c);
bool isPrime(int candidate);
void test(void);

int main(void) {
    test();
    findPlanetBob();
    return 0;
}

void findPlanetBob(void) {
    int i, j;
    char name[3];

    for (i = 'a'; i <= 'z'; i++) {
        name[0] = i;
        name[2] = i;

        for (j = 'a'; j <= 'z'; j++) {
            name[1] = j;

            if (validNameStructure(name)) {
                if (isPrime(letterVal(name[0]) + letterVal(name[1]) + letterVal(name[2]))) {
                    printf("%s\n", name);
                }
            }
        }
    }
}

bool validNameStructure(char name[]) {
    if ((isVowel(name[0]) && !isVowel(name[1])) ||
        (!isVowel(name[0]) && isVowel(name[1]))) {
        return true;
    }
    return false;
}

bool isVowel(char c) {
    switch (tolower(c)) {
        case 'a':
        case 'e':
        case 'i':
        case 'o':
        case 'u':
            return true;
    }
    return false;
}

bool isPrime(int candidate) {
    int j;

    if (candidate == 2) {
        return true;
    }
    if (candidate < 2 || candidate % 2 == 0) {
        return false;
    }

    /* QUESTION is this naughty? */
    for (j = 3; j <= candidate / 2; j += 2) {
        if (candidate % j == 0) {
            return false;
        }
    }
    return true;
}

void test(void) {
    assert(isPrime(3) == true);
    assert(isVowel('a') == true);
    assert(isVowel('t') == false);
    assert(validNameStructure("aaa") == false);
    assert(validNameStructure("aba") == true);
}