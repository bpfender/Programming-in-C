#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "interpreter.h"
#include "parser.h"
#include "tokenizer.h"

int main(void) {
    char tst_line[500] = "$A1=#Neill#";
    char word[50];
    char* pos = tst_line;
    line_t len;
    int i;
    prog_t* program;
    symbol_t* symbols = initSymbolTable();
    char filename[500];
    ast_t* ast;

    symbol_t* symboltab;
    mvm* files;

    printf("Testing start...\n");

    while ((len = parseBufferWords(&pos))) {
        memcpy(word, pos, len * sizeof(char));
        word[len] = '\0';
        printf("%s ", word);
        pos += len;
    }
    printf("\n");

    program = tokenizeFile("./Files/test5.nal", symbols);
    for (i = 0; i < program->len; i++) {
        printInstr(program->token[i].type);
        printf(" Line: %d Word: %d", program->token[i].line, program->token[i].word);
        printf(" %s\n", program->token[i].attrib);
    }
    printf("\n\n");
    freeProgQueue(program);
    freeSymbolTable(symbols);

    printf("PARSE TEST\n");

    strcpy(filename, "./Files/test5.nal");
    symboltab = initSymbolTable();
    files = tok_filesinit();
    program = tokenizeFile(filename, symboltab);
    tok_insertfilename(files, filename, program);
    ast = initAST();

    parseFile(program, symboltab, files);

    tok_freefilenames(files);
    freeSymbolTable(symboltab);
    freeAST(ast);

    printf("Testing end...\n");

    return 0;
}
