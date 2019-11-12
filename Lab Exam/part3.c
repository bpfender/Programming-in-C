#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EXT_LEN 3
#define EXTENSION "808"

char* pwdextend(char* a);

int main(void) {
    char *p2, *p1;
    p1 = pwdextend("password");
    p2 = pwdextend("hackable");

    assert(strcmp(p1, "password808") == 0);
    assert(strcmp(p2, "hackable808") == 0);

    free(p1);
    free(p2);

    return 0;
}

char* pwdextend(char* a) {
    int len = strlen(a);
    char* ptr;

    /* Allocate memory for existing string, extension and null terminator */
    ptr = (char*)malloc((len + EXT_LEN + 1) * sizeof(char));
    if (ptr == NULL) {
        fprintf(stderr, "Memory allocation error\n");
        exit(EXIT_FAILURE);
    }

    strcpy(ptr, a);
    strcpy(ptr + len, EXTENSION);

    return ptr;
}