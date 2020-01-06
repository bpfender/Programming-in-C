#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "symbols.h"
#include "tokenizer.h"

void inter_abort(prog_t* program, symbol_t* symbols);
void inter_in2str(prog_t* program, symbol_t* symbols);
void inter_innum(prog_t* program, symbol_t* symbols);
void inter_jump(prog_t* program, symbol_t* symbols);
void inter_print(prog_t* program, symbol_t* symbols);
void inter_ifcond(prog_t* program, symbol_t* symbols);
void inter_inc(prog_t* program, symbol_t* symbols);
void inter_set(prog_t* program, symbol_t* symbols);

#endif
