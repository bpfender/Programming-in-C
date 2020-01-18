#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "symbols.h"
#include "tokenizer.h"

void inter_abort(void);
void inter_innum(mvmcell* arg);
void inter_in2str(mvmcell* arg1, mvmcell* arg2);

#endif
