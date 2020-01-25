#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "interpreter.h"
#include "parser.h"
#include "symbols.h"
#include "tokenizer.h"

int main(void) {
    prog_t* program;

    char test_line[100];
    char* pos;

    token_t tst_token;
    token_t* token;

    program = initProgQueue("TEST");
    assert(strcmp(program->filename, "TEST") == 0);
    assert(program->len == 0);
    assert(program->pos == 0);
    freeProgQueue(program);

    assert(isFILE("FILE") == TRUE);
    assert(isFILE("file") == FALSE);
    assert(isFILE("blah") == FALSE);

    assert(isABORT("ABORT") == TRUE);
    assert(isABORT("abort") == FALSE);
    assert(isABORT("blah") == FALSE);

    assert(isIN2STR("IN2STR") == TRUE);
    assert(isIN2STR("in2str") == FALSE);
    assert(isIN2STR("blah") == FALSE);

    assert(isINNUM("INNUM") == TRUE);
    assert(isINNUM("innum") == FALSE);
    assert(isINNUM("blah") == FALSE);

    assert(isIFEQUAL("IFEQUAL") == TRUE);
    assert(isIFEQUAL("iFEqual") == FALSE);
    assert(isIFEQUAL("blah") == FALSE);

    assert(isGREATER("IFGREATER") == TRUE);
    assert(isGREATER("IFgreater") == FALSE);
    assert(isGREATER("blah") == FALSE);

    assert(isINC("INC") == TRUE);
    assert(isINC("inc") == FALSE);
    assert(isINC("blah") == FALSE);

    assert(isJUMP("JUMP") == TRUE);
    assert(isJUMP("jump") == FALSE);
    assert(isJUMP("124v") == FALSE);

    assert(isPRINT("PRINT") == TRUE);
    assert(isPRINT("pRiNT") == FALSE);
    assert(isPRINT("gwaeew") == FALSE);

    assert(isPRINTN("PRINTN") == TRUE);
    assert(isPRINTN("printN") == FALSE);
    assert(isPRINTN("ga24g3") == FALSE);

    assert(isRND("RND") == TRUE);
    assert(isRND("rND") == FALSE);
    assert(isRND("a4g23q") == FALSE);

    assert(isSTRVAR("$TEST") == TRUE);
    assert(isSTRVAR("$telfs") == FALSE);
    assert(isSTRVAR("$TEST1") == FALSE);

    assert(isNUMVAR("%TEST") == TRUE);
    assert(isNUMVAR("%test") == FALSE);
    assert(isNUMVAR("%TEST1") == FALSE);

    assert(isSTRCON("\"Test string\"") == TRUE);
    assert(isSTRCON("#Test string#") == TRUE);
    assert(isSTRCON("test string") == FALSE);

    assert(isNUMCON("14") == TRUE);
    assert(isNUMCON("14.5") == TRUE);
    assert(isNUMCON("14.5.6") == FALSE);
    assert(isNUMCON(".6") == TRUE);

    assert(isBRACKET("(") == TRUE);
    assert(isBRACKET(")") == TRUE);
    assert(isBRACKET("{") == FALSE);
    assert(isBRACKET("blah") == FALSE);

    assert(isSECTION("{") == TRUE);
    assert(isSECTION("}") == TRUE);
    assert(isSECTION(")") == FALSE);
    assert(isSECTION("blah") == FALSE);

    assert(isCOMMA(",") == TRUE);
    assert(isCOMMA(".") == FALSE);
    assert(isCOMMA("blah") == FALSE);

    assert(tokenType("FILE") == FILE_REF);
    assert(tokenType("\"Test string\"") == STRCON);
    assert(tokenType("BLAH") == ERROR);

    /* Testing that parse buffer words handles spaces properly */
    strcpy(test_line, "#TEST STRING #     ");
    pos = test_line;
    assert(parseBufferWords(&pos) == 14);

    strcpy(test_line, "RND(%TEST)");
    pos = test_line;

    assert(parseBufferWords(&pos) == 3);
    assert(strcmp(pos, "RND(%TEST)") == 0);
    pos += 3;
    assert(parseBufferWords(&pos) == 1);
    assert(strcmp(pos, "(%TEST)") == 0);
    pos += 1;
    assert(parseBufferWords(&pos) == 5);
    assert(strcmp(pos, "%TEST)") == 0);
    pos += 5;
    assert(parseBufferWords(&pos) == 1);
    assert(strcmp(pos, ")") == 0);

    strcpy(test_line, "    RND (%TEST    )\n");
    pos = test_line;
    assert(parseBufferWords(&pos) == 3);
    assert(strcmp(pos, "RND (%TEST    )\n") == 0);
    pos += 3;
    assert(parseBufferWords(&pos) == 1);
    assert(strcmp(pos, "(%TEST    )\n") == 0);
    pos += 1;
    assert(parseBufferWords(&pos) == 5);
    assert(strcmp(pos, "%TEST    )\n") == 0);
    pos += 5;
    assert(parseBufferWords(&pos) == 1);
    assert(strcmp(pos, ")\n") == 0);

    /* Testing string conversion functions */
    strcpy(test_line, "URYYB, uryyb . 1234");
    rot18(test_line);
    assert(strcmp(test_line, "HELLO, hello . 6789") == 0);

    strcpy(test_line, "\"TEST LINE\"");
    getSTRCON(test_line);
    assert(strcmp(test_line, "TEST LINE") == 0);

    strcpy(test_line, "#GRFG yvar, 678#");
    getSTRCON(test_line);
    assert(strcmp(test_line, "TEST line, 123") == 0);

    buildToken(&tst_token, "PRINT $VAR", 5, 0, 0);
    assert(strcmp(tst_token.attrib, "PRINT") == 0);
    assert(tst_token.type == PRINT);
    assert(tst_token.line == 0);
    assert(tst_token.word == 0);
    free(tst_token.attrib);

    buildToken(&tst_token, "#URYYB#  ", 7, 0, 0);
    assert(strcmp(tst_token.attrib, "HELLO") == 0);
    assert(tst_token.type == STRCON);
    assert(tst_token.line == 0);
    assert(tst_token.word == 0);
    free(tst_token.attrib);

    program = initProgQueue("test");
    enqueueToken(program, "PRINT $VAR", 5, 0, 0);
    assert(program->len == 1);
    enqueueToken(program, "#URYYB#  ", 7, 0, 1);
    assert(program->len == 2);

    assert(strcmp(program->token[0].attrib, "PRINT") == 0);
    assert(program->token[0].type == PRINT);
    assert(program->token[0].line == 0);
    assert(program->token[0].word == 0);
    assert(strcmp(program->token[1].attrib, "HELLO") == 0);
    assert(program->token[1].type == STRCON);
    assert(program->token[1].line == 0);
    assert(program->token[1].word == 1);

    token = peekToken(program, 0);
    assert(strcmp(token->attrib, "PRINT") == 0);
    assert(token->type == PRINT);
    assert(token->line == 0);
    assert(token->word == 0);

    token = dequeueToken(program);
    assert(program->pos == 1);
    assert(strcmp(token->attrib, "PRINT") == 0);
    assert(token->type == PRINT);
    assert(token->line == 0);
    assert(token->word == 0);

    freeProgQueue(program);

    assert(tokenizeFile("error")==NULL);
    program = tokenizeFile("test1.nal");
    assert(program->len == 4);
    assert(program->token[0].type == SECTION);
    assert(program->token[1].type == PRINT);
    assert(program->token[2].type == STRCON);
    assert(program->token[3].type == SECTION);

    freeProgQueue(program);
    
    
    
    /*


    char tst_line[500] = "$A1=#Neill#";
    char word[50];
    char* pos = tst_line;
    line_t len;
    int i;
    prog_t* program;
    symbol_t* symbols = initSymbolTable();
    char filename[500];

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

    program = tokenizeFile("test5.nal");
    for (i = 0; i < program->len; i++) {
        printInstr(program->token[i].type);
        printf(" Line: %d Word: %d", program->token[i].line, program->token[i].word);
        printf(" %s\n", program->token[i].attrib);
    }
    printf("\n\n");
    freeProgQueue(program);
    freeSymbolTable(symbols);

    printf("PARSE TEST\n");

    strcpy(filename, "escape211.nal");
    symboltab = initSymbolTable();
    addFilename(symboltab, filename, NULL);
    files = tok_filesinit();
    program = tokenizeFile(filename);
    if (!program) {
        fprintf(stderr, "Failed to open file\n");
        exit(EXIT_FAILURE);
    }
    tok_insertfilename(files, filename, program);

    parseFile(program, symboltab);

    tok_freefilenames(files);
    freeSymbolTable(symboltab);

    printf("Testing end...\n");
*/
    return 0;
}
