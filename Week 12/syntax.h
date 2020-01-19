#ifndef NAL_H
#define NAL_H

#include "tokenizer.h"

void parse_prog(token_t* token);
void parse_section(token_t* token);

void parse_abort(token_t* token);

#endif
