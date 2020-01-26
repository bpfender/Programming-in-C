#ifndef PARSER_H
#define PARSER_H

#include "symbols.h"
#include "tokenizer.h"

/* Index definition for not bracketed args */
#define ARG_INDEX 1
#define SET_VAR 0
#define SET_VAL 2
#define SET_OP 1

/* Index definitions for bracketed arguments */
#define BRKT_OPEN 1
#define BRKT_ARG1 2
#define BRKT_1ARGCLOSE 3
#define BRKT_COMMA 3
#define BRKT_ARG2 4
#define BRKT_2ARGCLOSE 5

typedef enum brkt_t { SINGLE,
                      DOUBLE} brkt_t;

/* ------- PARSING FUNCTIONS ------- */
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

/* ------- HELPER FUNCTIONS ------- */
/* The functions below are used to parse INSTRUCT statements that use brackets.
 * this packages the parsing so the function just has to be called with the 
 * appropriate argument from the corresponding parsing function
 */
bool_t parseCondBracket(prog_t* program);
bool_t parseBrackets(prog_t* program, type_t arg, brkt_t brkt);
bool_t parseSetVals(prog_t* program);

/* Fills program->instr with number of tokens as called from instr(). Allows
 * for easier analysis of syntax 
 */
void fillTokenString(prog_t* program, int len);

#endif
