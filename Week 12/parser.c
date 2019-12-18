#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define ROT5 5
#define ROT13 13
#define ALPHA 26
#define DIGIT 10

#define SUCCESS 0
#define ENCODE_ERR 1

void test(void);

int main(void) {
    test();
    return 0;
}



/* ROT18 encoder for strings */
int ROT18(char* s) {
    size_t i;
    for (i = 0; s[i] != '\0'; i++) {
        if (isupper(s[i])) {
            s[i] = 'A' + (s[i] - 'A' + ROT13) % ALPHA;
        } else if (islower(s[i])) {
            s[i] = 'a' + (s[i] - 'a' + ROT13) % ALPHA;
        } else if (isdigit(s[i])) {
            s[i] = (s[i] + ROT5) % DIGIT;
        } else {
            return ENCODE_ERR;
        }
    }
    return SUCCESS;
}

void test(void) {
    char str[50] = "Arvyy";
    ROT18(str);
    printf("%s\n", str);
}