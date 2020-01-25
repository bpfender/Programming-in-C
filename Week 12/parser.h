#ifndef PARSER_H
#define PARSER_H

#include "symbols.h"
#include "tokenizer.h"

/* Starting function for parsing file. If it returns correctly, file has parsed
 * successfully
 */
void parseFile(prog_t* program, symbol_t* SYMBOLS_H);

/* Check that first token is { */
void prog(prog_t* program, symbol_t* symbols);

/* Switches through instruction options. If invalid, returns an error */
void instr(prog_t* program, symbol_t* symbols);

/* If file has already been passed, skips parsing next file. If interpreting,
 * tokenizes file and interprets */
void file(prog_t* program, symbol_t* symbols);

void prog_abort(prog_t* program, symbol_t* symbols);
void in2str(prog_t* program, symbol_t* symbols);
void innum(prog_t* program, symbol_t* symbols);
void ifequal(prog_t* program, symbol_t* symbols);
void ifgreater(prog_t* program, symbol_t* symbols);
void inc(prog_t* program, symbol_t* symbols);
void jump(prog_t* program, symbol_t* symbols);
void print(prog_t* program, symbol_t* symbols);
void rnd(prog_t* program, symbol_t* symbols);
void set(prog_t* program, symbol_t* symbols);

bool_t parseCondBracket(prog_t* program);
bool_t parseBrackets(prog_t* program, type_t arg, int len);
bool_t parseSetVals(prog_t* program);

void fillTokenString(prog_t* program, token_t* tokens[], int len);

#endif
