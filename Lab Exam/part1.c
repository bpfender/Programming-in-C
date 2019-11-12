#include <assert.h>
#include <ctype.h>
#include <stdio.h>

int pwdok(char* s);

int main(void) {
    assert(pwdok("Ab1") == 1);
    assert(pwdok("Ab#") == 1);
    assert(pwdok("Ab") == 0);
    assert(pwdok("#abndsjksd2434") == 0);

    return 0;
}

int pwdok(char* s) {
    int i;

    int upper = 0;
    int lower = 0;
    int non_alpha = 0;

    for (i = 0; s[i] != '\0'; i++) {
        if (isupper(s[i])) {
            upper = 1;
        }
        if (islower(s[i])) {
            lower = 1;
        }
        if (!isalpha(s[i])) {
            non_alpha = 1;
        }
    }

    return (upper == 1 && lower == 1 && non_alpha == 1);
}
