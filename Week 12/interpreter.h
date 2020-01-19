#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "symbols.h"
#include "tokenizer.h"

void inter_abort(void);
void inter_innum(prog_t* program, symbol_t* symbols);
void inter_in2str(prog_t* program, symbol_t* symbols);
void inter_inc(prog_t* program, symbol_t* symbols);
void inter_print(prog_t* program, symbol_t* symbols);
void inter_rnd(prog_t* program, symbol_t* symbols);
void inter_set(prog_t* program, symbol_t* symbols);

bool_t inter_ifequal(prog_t* program, symbol_t* symbols);
bool_t inter_ifgreater(prog_t* program, symbol_t* symbols);
void* getArg(token_t* token, symbol_t* symbols);
void findElseJump(prog_t* program);

void inter_jump(prog_t* program);
bool_t checkJumpValue(char* num);
bool_t checkValidJump(prog_t* program, int pos);

#endif
