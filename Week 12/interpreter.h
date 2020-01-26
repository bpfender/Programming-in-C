#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "symbols.h"
#include "tokenizer.h"

#define MAX_INPUT_LEN 50

/* ------ INTERPRETER FUNCTIONS ------ */
/* Ends program and returns success message
 */
void inter_abort(void);

/* Takes two string arguments and add them to STRVARS in the symbol table
 */
void inter_in2str(prog_t* program, symbol_t* symbols);

/* Take one num argument and adds it to a NUMVAR in the symbol table
 */
void inter_innum(prog_t* program, symbol_t* symbols);

/* Conditional statements take arguments, identify them as STR or NUM and perform
 * appropriate comparison on them. If the condition is met, function returns
 * and parse continues as before. IF the condition fails, findElseJump() is 
 * called to update the program position to where it should continue.
 */
bool_t inter_ifequal(prog_t* program, symbol_t* symbols);
bool_t inter_ifgreater(prog_t* program, symbol_t* symbols);

/* Checks that NUMVAR exists in symbol table before incrementing it
 */
void inter_inc(prog_t* program, symbol_t* symbols);

/* Generates random (integert) number between 0-99 and adds it to the symbol
 * table under NUMVAR entry
 */
void inter_rnd(prog_t* program, symbol_t* symbols);

/* Checks the NUMCON jump value. First checks that it is an integer value, 
 * before checking that the program location it jumps to is a valid INSTRUCT
 */
void inter_jump(prog_t* program);

/* Processes arguments to check for STRVAR, NUMVAR, STRCON or NUMCON and prints
 * accordingly. ALso checks for PRINT/PRINTN argument to determine whether a \n
 * should be printed at the end of the statement or not
 */
void inter_print(prog_t* program, symbol_t* symbols);

/* Adds or updates VAR to be update into the symbol table, casting as appropriate
 * to string or double. Checks that assignment value is valid (in case of VAR)
 * and updates symbol table entry
 */
void inter_set(prog_t* program, symbol_t* symbols);

/* ------- HELPER FUNCTIONS ------ */
/* Returns void* to the arguments in conditional statements. These are then cast
 * according the argument type identified in the token stream. There's a little 
 * bit of trickery involved for NUMCONS, where the double* num argument is 
 * updated directly. WOuld otherwise have required a static variable declaration
 * within the function which would have felt even messier
 */
void* getArg(token_t* token, symbol_t* symbols, double* num);

/* Parses through sets of {} brackets to find the end of the conditional
 * statement. Updates program->pos the location from which execution should
 * resume
 */
void findElseJump(prog_t* program);

/* Checks that string expressing the jump value is an integer
 */
bool_t checkJumpValue(char* num);

/* Checks that the program location the jump resolves to is a valid INSTRUCT
 */
bool_t checkValidJump(prog_t* program, int pos);

/* ------ UTILITY FUNCTIONS ------ */
/* Easy handler functions to allocate space for numbers of strings that are to
 * be stored with variables
 */
double* allocNumber(void);
char* allocString(size_t len);

/* Seed function for random number generator
 */
void inter_rndSeed(void);

#endif
