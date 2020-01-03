#ifndef PARSER_H
#define PARSER_H

#include "tokenizer.h"

#define ROT5 5
#define ROT13 13
#define ALPHA 26
#define DIGIT 10

void rot18(char* s);
void getSTRCON(char* word);

void prog(prog_t* program);
void instr(prog_t* program);
void file(prog_t* program);

void handleError(void);

/*    switch (token->type) {
        case FILE_:
        case ABORT:
        case IN2STR:
        case INNUM:
        case IFEQUAL:
        case IFGREATER:
        case INC:
        case SET:
        case JUMP:
        case PRINT:
        case PRINTN:
        case RND:
        case STRVAR:
        case NUMVAR:
        case STRCON:
        case NUMCON:
        case BRACKET:
        case SECTION:
        case COMMA:
        case ERROR:
        default:
            break;
    }
*/

#endif
