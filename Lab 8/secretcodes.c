#include <ctype.h>
#include <stdio.h>

#define NDEBUG
#include <assert.h>

void test(void);
int scode(int a);

int main(void) {
    int input_char;
    int output_char;

    test();

    while ((input_char = getchar()) != EOF) {
        output_char = convert(input_char);
        printf("%c", output_char);
    }

    return 0;
}

/* QUESTION would it not be better to define the types as chars here? */
int scode(int a) {
    if (isalpha(a)) {
        if (isupper(a)) {
            return 'Z' - (a - 'A');
        } else {
            return 'z' - (a - 'a');
        }
    }
    return a;
}

void test() {
    assert(convert('A') == 'Z');
    assert(convert('b') == 'y');
    assert(convert('5') == '5');
    assert(convert(',') == ',');
    assert(convert(' ') == ' ');
    assert(convert('\n') == '\n');
}