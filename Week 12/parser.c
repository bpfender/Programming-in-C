#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "interpreter.h"
#include "parser.h"

/* Lengths for populating instr line */
#define SECT_LEN 1
#define ABORT_LEN 1
#define SET_LEN 3
#define SINGLE_ARG_LEN 2
#define SINGLE_BRKT_LEN 4
#define DOUBLE_BRKT_LEN 6

#define ERROR(TOKEN)                                                                                           \
    {                                                                                                          \
        fprintf(stderr, "Error code line %d\n Program line %d word %d\n", __LINE__, TOKEN->line, TOKEN->word); \
        exit(2);                                                                                               \
    }

/* ------- PARSING FUNCTIONS ------- */

bool_t parseFile(prog_t* program, symbol_t* symbols) {
    prog(program, symbols);

    if (program->pos == program->len) {
        return TRUE;
    } else {
        err_extraTokens();
        return FALSE;
    }
}

void prog(prog_t* program, symbol_t* symbols) {
    token_t* token = program->instr[0] = nextToken(program);

    if (strcmp(token->attrib, "{")) {
        err_prog(program);
    }
    instr(program, symbols);
}

void instr(prog_t* program, symbol_t* symbols) {
    /* Check next token to determine instruction type. Populated for error
    handling */
    token_t* token = program->instr[0] = peekToken(program, 0);

    switch (token->type) {
        case FILE_REF:
            fillTokenString(program, SINGLE_ARG_LEN);
            file(program, symbols);
            break;
        case ABORT:
            fillTokenString(program, ABORT_LEN);
            prog_abort(program, symbols);
            return;
            break;
        case IN2STR:
            fillTokenString(program, DOUBLE_BRKT_LEN);
            in2str(program, symbols);
            break;
        case INNUM:
            fillTokenString(program, SINGLE_BRKT_LEN);
            innum(program, symbols);
            break;
        case IFEQUAL:
            fillTokenString(program, DOUBLE_BRKT_LEN);
            ifequal(program, symbols);
            break;
        case IFGREATER:
            fillTokenString(program, DOUBLE_BRKT_LEN);
            ifgreater(program, symbols);
            break;
        case INC:
            fillTokenString(program, SINGLE_BRKT_LEN);
            inc(program, symbols);
            break;
        case JUMP:
            fillTokenString(program, SINGLE_ARG_LEN);
            jump(program, symbols);
            break;
        case PRINT:
        case PRINTN:
            fillTokenString(program, SINGLE_ARG_LEN);
            print(program, symbols);
            break;
        case RND:
            fillTokenString(program, SINGLE_BRKT_LEN);
            rnd(program, symbols);
            break;
        case STRVAR:
        case NUMVAR:
            fillTokenString(program, SET_LEN);
            set(program, symbols);
            break;
        case SECTION:
            fillTokenString(program, SECT_LEN);
            if (strcmp(token->attrib, "}")) {
                err_instr(program);
                /* If err_instr doesn't abort program, continue parsing */
                instr(program, symbols);
            }
            /* If }, should be end of a section and recursion for this section 
            can be ended */
            break;
        case ERROR:
        case SET:
        case STRCON:
        case NUMCON:
        case BRACKET:
        case COMMA:
        default:
            err_instr(program);
            instr(program, symbols);
            break;
    }
}

void file(prog_t* program, symbol_t* symbols) {
    token_t* token = program->instr[ARG_INDEX];

    prog_t* next_program;
    char* filename;

    int interp_flag = 0;

    if (token->type == STRCON) {
        filename = token->attrib;

        /* If filename is in the symbol table, file doesn't need to be parsed again.
        However, if interpreting, file needs to be opened again */
#ifdef INTERP
        interp_flag = 1;
#endif

        if (!getFilename(symbols, filename) || interp_flag) {
            if (!(next_program = tokenizeFile(filename))) {
                err_file(program);
            } else {
                addFilename(symbols, filename, NULL);

                parseFile(next_program, symbols);
                freeProgram(next_program);
            }
        }
    } else {
        err_file(program);
    }

    instr(program, symbols);
}

void prog_abort(prog_t* program, symbol_t* symbols) {
#ifdef INTERP
    inter_abort();
#endif

    instr(program, symbols);
}

void in2str(prog_t* program, symbol_t* symbols) {
    parseBrackets(program, STRVAR, DOUBLE);

#ifdef INTERP
    inter_in2str(program, symbols);
#endif

    instr(program, symbols);
}

void innum(prog_t* program, symbol_t* symbols) {
    parseBrackets(program, NUMVAR, SINGLE);

#ifdef INTERP
    inter_innum(program, symbols);
#endif

    instr(program, symbols);
}

/* FIXME ifequal and ifgreater are identical */
void ifequal(prog_t* program, symbol_t* symbols) {
    parseCondBracket(program);

#ifdef INTERP
    if (inter_ifequal(program, symbols)) {
        prog(program, symbols);
    }
#else
    prog(program, symbols);
#endif

    instr(program, symbols);
}

void ifgreater(prog_t* program, symbol_t* symbols) {
    parseCondBracket(program);

#ifdef INTERP
    if (inter_ifgreater(program, symbols)) {
        prog(program, symbols);
    }
#else
    prog(program, symbols);
#endif

    instr(program, symbols);
}

void inc(prog_t* program, symbol_t* symbols) {
    parseBrackets(program, NUMVAR, SINGLE);

#ifdef INTERP
    inter_inc(program, symbols);
#endif

    instr(program, symbols);
}

void jump(prog_t* program, symbol_t* symbols) {
    token_t* token = program->instr[ARG_INDEX];

    if (token->type != NUMCON) {
        err_jump(program);
    }

#ifdef INTERP
    inter_jump(program);
#endif

    instr(program, symbols);
}

void print(prog_t* program, symbol_t* symbols) {
    token_t* token = program->instr[ARG_INDEX];

    if (token->type != STRVAR && token->type != NUMVAR &&
        token->type != STRCON && token->type != NUMCON) {
        err_print(program);
    }

#ifdef INTERP
    inter_print(program, symbols);
#endif

    instr(program, symbols);
}

void rnd(prog_t* program, symbol_t* symbols) {
    parseBrackets(program, NUMVAR, SINGLE);

#ifdef INTERP
    inter_rnd(program, symbols);
#endif

    instr(program, symbols);
}

void set(prog_t* program, symbol_t* symbols) {
    parseSetVals(program);

#ifdef INTERP
    inter_set(program, symbols);
#endif

    instr(program, symbols);
}

/* -------- HELPER FUNCTIONS --------- */

bool_t parseCondBracket(prog_t* program) {
    token_t** instr = program->instr;

    if (strcmp(instr[BRKT_OPEN]->attrib, "(")) {
        err_cond(program, BRKT_OPEN);
        return TRUE;
    }

    if (!(instr[BRKT_ARG1]->type == STRVAR || instr[BRKT_ARG1]->type == NUMVAR ||
          instr[BRKT_ARG1]->type == STRCON || instr[BRKT_ARG1]->type == NUMCON)) {
        err_cond(program, BRKT_ARG1);
        return TRUE;
    }

    if (instr[BRKT_COMMA]->type != COMMA) {
        err_cond(program, BRKT_COMMA);
        return TRUE;
    }

    /* Depending on the initial argument, the second argument has to be of the 
    same type */
    switch (instr[BRKT_ARG1]->type) {
        case STRVAR:
        case STRCON:
            if (!(instr[BRKT_ARG2]->type == STRVAR || instr[BRKT_ARG2]->type == STRCON)) {
                err_cond(program, BRKT_ARG2);
                return TRUE;
            }
            break;
        case NUMVAR:
        case NUMCON:
            if (!(instr[BRKT_ARG2]->type == NUMVAR || instr[BRKT_ARG2]->type == NUMCON)) {
                err_cond(program, BRKT_ARG2);
                return TRUE;
            }
            break;
        default:
            break;
    }

    if (strcmp(instr[BRKT_2ARGCLOSE]->attrib, ")")) {
        err_cond(program, BRKT_2ARGCLOSE);
        return TRUE;
    }

    return FALSE;
}

bool_t parseBrackets(prog_t* program, type_t arg, brkt_t brkt) {
    token_t** instr = program->instr;

    if (strcmp(instr[BRKT_OPEN]->attrib, "(")) {
        err_bracket(program, BRACKET, BRKT_OPEN);
        return TRUE;
    }

    if (instr[BRKT_ARG1]->type != arg) {
        err_bracket(program, arg, BRKT_ARG1);
        return TRUE;
    }

    /* Parse according to single or double bracket */
    switch (brkt) {
        case SINGLE:
            if (strcmp(instr[BRKT_1ARGCLOSE]->attrib, ")")) {
                err_bracket(program, arg, BRKT_1ARGCLOSE);
                return TRUE;
            }
            break;
        case DOUBLE:
            if (instr[BRKT_COMMA]->type != COMMA) {
                err_bracket(program, COMMA, BRKT_COMMA);
                return TRUE;
            }

            if (instr[BRKT_ARG2]->type != arg) {
                err_bracket(program, arg, BRKT_2ARGCLOSE);
                return TRUE;
            }

            if (strcmp(instr[BRKT_2ARGCLOSE]->attrib, ")")) {
                err_bracket(program, BRACKET, BRKT_2ARGCLOSE);
                return TRUE;
            }
            break;
    }
    return FALSE;
}

bool_t parseSetVals(prog_t* program) {
    int index;
    switch (program->instr[SET_VAR]->type) {
        case STRVAR:
            index = SET_OP;
            if (program->instr[SET_OP]->type == SET) {
                index = SET_VAL;
                if (program->instr[SET_VAL]->type == STRCON ||
                    program->instr[SET_VAL]->type == STRVAR) {
                    return FALSE;
                }
            }
            err_set(program, index);
            return TRUE;
        case NUMVAR:
            index = SET_OP;
            if (program->instr[SET_OP]->type == SET) {
                index = SET_VAL;
                if (program->instr[SET_VAL]->type == NUMVAR ||
                    program->instr[SET_VAL]->type == NUMCON) {
                    return FALSE;
                }
            }
            err_set(program, index);
            return TRUE;
        default:
            return TRUE;
    }
}

void fillTokenString(prog_t* program, int len) {
    int i;
    for (i = 0; i < len; i++) {
        program->instr[i] = nextToken(program);
    }
}
