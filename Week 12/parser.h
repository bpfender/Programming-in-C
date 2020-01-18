#ifndef PARSER_H
#define PARSER_H

#include "ast.h"
#include "symbols.h"
#include "tokenizer.h"

#define ROT5 5
#define ROT13 13
#define ALPHA 26
#define DIGIT 10

void rot18(char* s);
void getSTRCON(char* word);

/* FIXME no error checking for non-closed brackets */
void parseFile(prog_t* program, symbol_t* symbols, mvm* files);

void prog(prog_t* program, symbol_t* symbols, mvm* files);
void instr(prog_t* program, symbol_t* symbols, mvm* files);
void file(prog_t* program, symbol_t* symbols, mvm* files);
void prog_abort(prog_t* program, symbol_t* symbols, mvm* files);
void in2str(prog_t* program, symbol_t* symbols, mvm* files);
void innum(prog_t* program, symbol_t* symbols, mvm* files);
void ifequal(prog_t* program, symbol_t* symbols, mvm* files);
void ifgreater(prog_t* program, symbol_t* symbols, mvm* files);
void inc(prog_t* program, symbol_t* symbols, mvm* files);
void jump(prog_t* program, symbol_t* symbols, mvm* files);
void print(prog_t* program, type_t type, symbol_t* symbols, mvm* files);
void rnd(prog_t* program, symbol_t* symbols, mvm* files);
void set(prog_t* program, type_t var, symbol_t* symbols, mvm* files);
bool_t parseBrackets(prog_t* program, type_t arg, int n);
bool_t parseCondBracket(prog_t* program);

token_t* parseCondBracketEdit(token_t* tokens[]);
token_t* parseBracketsEdit(token_t* tokens[], type_t arg, int len);
void fillTokenString(prog_t* program, token_t* tokens[], int len);

void handleError(void);

/*    switch (token->type) {
        case FILE_:
        case ABORT:
        case IN2STR:
        case INNUM:
        case IFEQUAL:
        case IFGREATER:
        case INC:
        case SET:
        case JUMP:
        case PRINT:
        case PRINTN:
        case RND:
        case STRVAR:
        case NUMVAR:
        case STRCON:
        case NUMCON:
        case BRACKET:
        case SECTION:
        case COMMA:
        case ERROR:
        default:
            break;
    }
*/

#endif
