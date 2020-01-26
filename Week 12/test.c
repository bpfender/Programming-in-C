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
    char* str;
    double* num;

    symbol_t* symbols;
    mvmcell* cell;

    /* Testing of TOKENIZER */
    program = initProgram("TEST");
    assert(strcmp(program->filename, "TEST") == 0);
    assert(program->len == 0);
    assert(program->pos == 0);
    freeProgram(program);

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

    program = initProgram("test");
    addToken(program, "PRINT $VAR", 5, 0, 0);
    assert(program->len == 1);
    addToken(program, "#URYYB#  ", 7, 0, 1);
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

    token = nextToken(program);
    assert(program->pos == 1);
    assert(strcmp(token->attrib, "PRINT") == 0);
    assert(token->type == PRINT);
    assert(token->line == 0);
    assert(token->word == 0);

    freeProgram(program);

    assert(tokenizeFile("error") == NULL);
    program = tokenizeFile("test1.nal");
    assert(program->len == 4);
    assert(program->token[0].type == SECTION);
    assert(program->token[1].type == PRINT);
    assert(program->token[2].type == STRCON);
    assert(program->token[3].type == SECTION);

    freeProgram(program);

    /* Testing of SYMBOLS */
    symbols = initSymbolTable();
    assert(symbols->files->head == NULL);
    assert(symbols->files->numkeys == 0);
    assert(symbols->vars->head == NULL);
    assert(symbols->vars->numkeys == 0);

    freeSymbolTable(symbols);

    symbols = initSymbolTable();

    assert(getVariable(symbols, "invalid") == NULL);
    assert(getFilename(symbols, "invalid") == NULL);

    updateVariable(symbols, "VAR1", NULL);
    cell = getVariable(symbols, "VAR1");
    assert(cell);
    assert(cell->data == NULL);
    assert(cell->next == NULL);

    updateVariable(symbols, "VAR2", NULL);
    cell = getVariable(symbols, "VAR2");
    assert(cell);
    assert(cell->data == NULL);
    assert(cell->next != NULL);
    assert(symbols->vars->head == cell);

    num = (double*)malloc(sizeof(double));
    if (!num) {
        fprintf(stderr, "ERROR\n");
    }
    *num = 5;

    str = (char*)malloc(sizeof(char) * 50);
    if (!str) {
        fprintf(stderr, "ERROR\n");
    }
    strcpy(str, "TESTSTRING");

    updateVariable(symbols, "VAR1", num);
    cell = getVariable(symbols, "VAR1");
    assert(*(double*)(cell->data) == 5);

    updateVariable(symbols, "VAR2", str);
    cell = getVariable(symbols, "VAR2");
    assert(strcmp(cell->data, "TESTSTRING") == 0);

    /* Testing direct adding to symbol table vars */
    num = (double*)malloc(sizeof(double));
    if (!num) {
        fprintf(stderr, "ERROR\n");
    }
    *num = 10;

    str = (char*)malloc(sizeof(char) * 50);
    if (!str) {
        fprintf(stderr, "ERROR\n");
    }
    strcpy(str, "teststring");

    updateVariable(symbols, "VAR3", num);
    updateVariable(symbols, "VAR4", str);
    cell = getVariable(symbols, "VAR3");
    assert(*(double*)(cell->data) == 10);
    cell = getVariable(symbols, "VAR4");
    assert(strcmp(cell->data, "teststring") == 0);

    addFilename(symbols, "file1", NULL);
    cell = getFilename(symbols, "file1");
    assert(strcmp(cell->key, "file1") == 0);

    freeSymbolTable(symbols);

    /* Testing some PARSING functions */
    program = tokenizeFile("./dblbrkt.nal");
    fillTokenString(program, 6);
    assert(program->instr[1]->type == BRACKET);
    assert(program->instr[2]->type == STRVAR);
    assert(program->instr[3]->type == COMMA);
    assert(program->instr[4]->type == STRVAR);
    assert(program->instr[5]->type == BRACKET);
    assert(parseBrackets(program, STRVAR, DOUBLE) == FALSE);
    freeProgram(program);

    program = tokenizeFile("./snglbrkt.nal");
    fillTokenString(program, 4);
    assert(program->instr[1]->type == BRACKET);
    assert(program->instr[2]->type == NUMVAR);
    assert(program->instr[3]->type == BRACKET);
    assert(parseBrackets(program, NUMVAR, SINGLE) == FALSE);
    freeProgram(program);

    program = tokenizeFile("./condbrkt.nal");
    fillTokenString(program, 6);
    assert(program->instr[1]->type == BRACKET);
    assert(program->instr[2]->type == STRVAR);
    assert(program->instr[3]->type == COMMA);
    assert(program->instr[4]->type == STRCON);
    assert(program->instr[5]->type == BRACKET);
    assert(parseCondBracket(program) == FALSE);
    freeProgram(program);

    program = tokenizeFile("./set.nal");
    fillTokenString(program, 3);
    assert(program->instr[0]->type == STRVAR);
    assert(program->instr[1]->type == SET);
    assert(program->instr[2]->type == STRCON);
    assert(parseSetVals(program) == FALSE);

    freeProgram(program);

    /* INTERPRETER Checking */
    program = tokenizeFile("./test2.nal");

    assert(checkJumpValue("5") == TRUE);
    assert(checkJumpValue("4.5") == FALSE);

    assert(checkValidJump(program, 7) == TRUE);
    assert(checkValidJump(program, 10) == FALSE);

    freeProgram(program);

    return 0;
}
