#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct pos_t {
    int* index;
    int* stack;
    int top;
    int current;
    int count;

} pos_t;

void test(void);

int main(void) {
    test();
    return 0;
}

void initBracketPos(pos_t* brackets, int leaves) {
    brackets->index = (int*)malloc(leaves * sizeof(int));
    brackets->stack = (int*)malloc(leaves * sizeof(int));
    if (!(brackets->index || brackets->stack)) {
        fprintf(stderr, "Memory allocation error\n");
        exit(EXIT_FAILURE);
    }

    brackets->top = 0;
    brackets->current = 0;
    brackets->count = 0;
}

void getBracketPos(pos_t* brackets, char* s) {
}

int countLeaves(char* s) {
    size_t i;
    int leaves = 1; /* head before any brackets */
    for (i = 0; s[i + 1] != '\0'; i++) {
        if (s[i] == '(') {
            leaves++;
        }
    }
    return leaves;
}

char* getStrInBracket(char* s) {
    char* tmp;
    size_t i = 0;

    while (!(s[i] == '(' || s[i] == ')')) {
        i++;
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
    size_t i;
    pos_t brackets;
    char* str = readTreeString("HELLO");
    printf("%s\n", str);
    free(str);

    str = getStrInBracket("123)");
    printf("%s\n", str);
    free(str);
    str = getStrInBracket("456(");
    printf("%s\n", str);
    free(str);
    str = getStrInBracket("*(");
    printf("%s\n", str);
    free(str);

    initBracketPos(&brackets, 100);
    getBracketPos(&brackets, "20(10(*)(*))(5(*)(*))");
    /*012345678911234567892*/

    for (i = 0; i < 6; i++) {
        printf("Bracket %li: %d\n", i + 1, brackets.index[i]);
    }
}
