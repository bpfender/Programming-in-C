#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "tokenizer.h"

int main(void) {
    char tst_line[500] = "$A1=#Neill#";
    char word[50];
    char* pos = tst_line;
    line_t len;
    int i;

    prog_t program;
    initProgQueue(&program);

    printf("Testing start...\n");

    while ((len = parseBufferWords(&pos))) {
        memcpy(word, pos, len * sizeof(char));
        word[len] = '\0';
        printf("%s ", word);
        pos += len;
    }
    printf("\n");

    loadFile(&program, "./Files/test6.nal");
    for (i = 0; i < program.len; i++) {
        printf("%s\n", program.token[i].attrib);
    }

    freeProgQueue(&program);

    printf("Testing end...\n");

    return 0;
}
