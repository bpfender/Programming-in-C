#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void test(void);

int main(void) {
    test();
    return 0;
}

char* getStrInBracket(char* s) {
    char* tmp;
    size_t i;
    for (i = 0; !(s[i] == '(' || s[i] == ')'); i++) {
    }
    tmp = (char*)malloc((i + 1) * sizeof(char));
    memcpy(tmp, s, (i + 1) * sizeof(char));
    tmp[i] = '\0';

    return tmp;
}

char* readTreeString(char* s) {
    char* tmp;
    size_t len = strlen(s);

    tmp = (char*)malloc((len + 1) * sizeof(char));
    if (!tmp) {
        fprintf(stderr, "Memory allocation error");
        exit(EXIT_FAILURE);
    }
    strcpy(tmp, s);
    return tmp;
}

void test(void) {
    char* str = readTreeString("HELLO");
    printf("%s\n", str);
    free(str);

    str = getStrInBracket("123)");
    printf("%s\n", str);
    free(str);
    str = getStrInBracket("456(");
    printf("%s\n", str);
    free(str);
}
