#ifndef PARSER_H
#define PARSER_H

#include "symbols.h"
#include "tokenizer.h"

#define SINGLE_BRKT_TOKENS 3
#define DOUBLE_BRKT_TOKENS 5

typedef enum brkt_t { SINGLE = SINGLE_BRKT_TOKENS,
                      DOUBLE = DOUBLE_BRKT_TOKENS } brkt_t;

/* Starting function for parsing file. If it returns correctly, file has parsed
 * successfully
 */
bool_t parseFile(prog_t* program, symbol_t* SYMBOLS_H);

/* Check that first token is { */
void prog(prog_t* program, symbol_t* symbols);

/* Switches through instruction options. If invalid, returns an error 
 */
void instr(prog_t* program, symbol_t* symbols);

/* If file has already been passed, skips parsing next file. If interpreting,
 * tokenizes file and interprets 
 */
void file(prog_t* program, symbol_t* symbols);

/* May give a warning when extension is enabled if abort is followed by more
 * code
 */
void prog_abort(prog_t* program, symbol_t* symbols);

/* All functions below parse brackets and identify errors in bracket string 
 */
void in2str(prog_t* program, symbol_t* symbols);
void innum(prog_t* program, symbol_t* symbols);
void ifequal(prog_t* program, symbol_t* symbols);
void ifgreater(prog_t* program, symbol_t* symbols);
void inc(prog_t* program, symbol_t* symbols);
void rnd(prog_t* program, symbol_t* symbols);


void jump(prog_t* program, symbol_t* symbols);
void print(prog_t* program, symbol_t* symbols);

/* Set is parsed with parseSetVals 
 */
void set(prog_t* program, symbol_t* symbols);

bool_t parseCondBracket(prog_t* program);
bool_t parseBrackets(prog_t* program, type_t arg, brkt_t brkt);
bool_t parseSetVals(prog_t* program);

/* Fills program->instr with number of tokens as called from instr(). Allows
 * for easier analysis of syntax 
 */
void fillTokenString(prog_t* program,  int len);

#endif
