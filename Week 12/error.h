#ifndef ERROR_H
#define ERROR_H

#include "tokenizer.h"

#define MATR_SIZE 50

void printLocation(token_t* token, char* filename);
void prog_error(prog_t* program);
void instr_error(prog_t* program);
void file_error(prog_t* program);
/* FIXME this is more of a warning */
void abort_error(prog_t* program);

void bracket_error(prog_t* program, type_t expected, int index, int len);
void cond_error(prog_t* program, int index);

void recoverError(prog_t* program);
void suggestCorrectToken(char* word);
size_t calculateMSD(const char* sample, char* data, size_t cost[MATR_SIZE][MATR_SIZE]);
size_t getMin(size_t a, size_t b, size_t c);
bool_t isINSTR(token_t* token);

#endif
