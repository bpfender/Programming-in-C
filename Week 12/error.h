#ifndef ERROR_H
#define ERROR_H

#include "tokenizer.h"

/* FIXME this limit needs to be defined somewhere */
#define MATR_SIZE 50

#define ON_ERROR(STR)     \
    fprintf(stderr, STR); \
    exit(EXIT_FAILURE)

void err_printLocation(token_t* token, char* filename);
void err_prog(prog_t* program);
void err_instr(prog_t* program);
void err_file(prog_t* program);
/* FIXME this is more of a warning */
void err_abort(prog_t* program);
void err_jump(prog_t* program);
void err_print(prog_t* program);
void err_lex(prog_t* program, char* filename);

void err_bracket(prog_t* program, type_t expected, int index, int len);
void err_cond(prog_t* program, int index);
void err_nextToken(prog_t* program);
void err_extraTokens(prog_t* program);
void err_set(prog_t* program, int index);

void err_recoverError(prog_t* program);
void suggestCorrectToken(char* word);
size_t calculateMSD(const char* sample, char* data, size_t cost[MATR_SIZE][MATR_SIZE]);
size_t getMin(size_t a, size_t b, size_t c);
bool_t isINSTR(token_t* token);

#endif
