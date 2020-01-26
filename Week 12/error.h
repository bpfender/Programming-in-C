#ifndef ERROR_H
#define ERROR_H

#include "tokenizer.h"

/* FIXME this limit needs to be defined somewhere */
#define MATR_SIZE 11

#define ON_ERROR(STR)     \
    fprintf(stderr, STR); \
    exit(EXIT_FAILURE)

/* -------- PARSING ERROR FUNCTIONS ------- */
/* If opening bracket is not found warning is raised. Presumably it is just missing
 * so program tries to recover parsing by reducing pos in file and scanning the 
 * next token to check if it is a valid INSTR as this is what would be expected
 * next
 */
void err_prog(prog_t* program);

/* If the first token in the INSTRUCT line is not as expected, this function
 * handles it. Provides several sensible suggestions on alternatives based on
 * the character identified. Even suggests the correct instruction if it has been
 * mispelled by calling suggestCorrectToken()
 */
void err_instr(prog_t* program);

/* Handles errors when parse_file() is called. If the file argument is a STRCON,
 * then it flags this as a potentially invalid filename. Otherwise it checks 
 * whether the input might be a string ("" or ##) at the beginning or end of the
 * token string, or just returns an error
 */
void err_file(prog_t* program);

/* Both below files return error messages based on which character in bracket
 * tokens is incorrect
 */
void err_cond(prog_t* program, int index);
void err_bracket(prog_t* program, type_t expected, int index);

/* Relatively simple single argument checking 
 */
void err_jump(prog_t* program);
void err_print(prog_t* program);

void err_set(prog_t* program, int index);

/* Handles error when there are no tokens left in the stream or there are tokens
 * left.
 */
void err_nextToken(void);
void err_extraTokens(void);

/* ------- INTERPRETER ERROR FUNCTIONS ------- */
/* Function handler when variable hasn't been initialised in the symbol table
 */
void err_interVAR(prog_t* program, int index);

/* Handles invalid input during INNUM or IN2STR calls 
 */
void err_interInput(prog_t* program, type_t type);

/* Handles any undefined errors
 */
void err_interUndef(prog_t* program);

/* Handles error with missing {} for conditional statement
 */
void err_interCond(prog_t* program);

/* -------- GENERAL ERROR FUNCTION -------- */
void err_printLocation(token_t* token, char* filename);

/* Attempts to recover from error by moving to next line in code
 */
void err_recoverError(prog_t* program);

/* Makes a suggestion of the correct token to use based on the MSD calculation
 * of the input token
 */
void suggestCorrectToken(char* word);

size_t calculateMSD(const char* sample, char* data, size_t cost[MATR_SIZE][MATR_SIZE]);

size_t getMin(size_t a, size_t b, size_t c);

bool_t isINSTR(token_t* token);

#endif
