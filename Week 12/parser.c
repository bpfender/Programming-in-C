#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "interpreter.h"
#include "parser.h"

/* Index definition for not bracketed args */
#define ARG_INDEX 1
#define SET_VAR 0
#define SET_VAL 2
#define SET_OP 1

/* Index definitions for bracketed arguments */
#define BRKT_OPEN 1
#define BRKT_ARG1 2
#define BRKT_1ARGCLOSE 3
#define BRKT_COMMA 3
#define BRKT_ARG2 4
#define BRKT_2ARGCLOSE 5

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

bool_t parseFile(prog_t* program, symbol_t* symbols) {
    prog(program, symbols);

    if (program->pos == program->len) {
        return TRUE;
    } else {
        tokenLeft_error(program);
        return FALSE;
    }
}

void prog(prog_t* program, symbol_t* symbols) {
    token_t* token = dequeueToken(program);

    if (strcmp(token->attrib, "{")) {
        prog_error(program);
    }
    instr(program, symbols);
}

void instr(prog_t* program, symbol_t* symbols) {
    /* Check next token to determine instruction type */
    token_t* token = peekToken(program, 0);

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
                instr_error(program);
                /* If instr_error doesn't abort program, continue parsing */
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
            instr_error(program);
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
        interp_flag = 0;
#endif

        if (!getFilename(symbols, filename) || interp_flag) {
            if (!(next_program = tokenizeFile(filename))) {
                lex_error(program, filename);
            } else {
                addFilename(symbols, filename, NULL);

                parseFile(next_program, symbols);
                freeProgQueue(next_program);
            }
        }
    } else {
        file_error(program);
    }

    instr(program, symbols);
}

void prog_abort(prog_t* program, symbol_t* symbols) {
    token_t* token = peekToken(program, 0);

    if (strcmp(token->attrib, "}")) {
        /* This call only gives a warning when the extension is enabled. */
        abort_error(program);
    }

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
    if (inter_ifgreater(program, symbols)) {
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
        jump_error(program);
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
        print_error(program);
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

bool_t parseCondBracket(prog_t* program) {
    token_t** instr = program->instr;

    if (strcmp(instr[BRKT_OPEN]->attrib, "(")) {
        cond_error(program, 0);
        return TRUE;
    }

    if (!(instr[BRKT_ARG1]->type == STRVAR || instr[BRKT_ARG1]->type == NUMVAR ||
          instr[BRKT_ARG1]->type == STRCON || instr[BRKT_ARG1]->type == NUMCON)) {
        cond_error(program, BRKT_ARG1);
        return TRUE;
    }

    if (instr[BRKT_COMMA]->type != COMMA) {
        cond_error(program, BRKT_COMMA);
        return TRUE;
    }

    /* Depending on the initial argument, the second argument has to be of the 
    same type */
    switch (instr[BRKT_ARG1]->type) {
        case STRVAR:
        case STRCON:
            if (!(instr[BRKT_ARG2]->type == STRVAR || instr[BRKT_ARG2]->type == STRCON)) {
                cond_error(program, BRKT_ARG2);
                return TRUE;
            }
            break;
        case NUMVAR:
        case NUMCON:
            if (!(instr[BRKT_ARG2]->type == NUMVAR || instr[BRKT_ARG2]->type == NUMCON)) {
                cond_error(program, BRKT_ARG2);
                return TRUE;
            }
            break;
        default:
            break;
    }

    if (strcmp(instr[BRKT_2ARGCLOSE]->attrib, ")")) {
        cond_error(program, BRKT_2ARGCLOSE);
        return TRUE;
    }

    return FALSE;
}

bool_t parseBrackets(prog_t* program, type_t arg, brkt_t brkt) {
    token_t** instr = program->instr;

    if (strcmp(instr[BRKT_OPEN]->attrib, "(")) {
        bracket_error(program, BRACKET, BRKT_OPEN, brkt);
        return TRUE;
    }

    if (instr[BRKT_ARG1]->type != arg) {
        bracket_error(program, arg, BRKT_ARG1, brkt);
        return TRUE;
    }

    /* Parse according to single or double bracket */
    switch (brkt) {
        case SINGLE:
            if (strcmp(instr[BRKT_1ARGCLOSE]->attrib, ")")) {
                bracket_error(program, arg, BRKT_1ARGCLOSE, brkt);
                return TRUE;
            }
            break;
        case DOUBLE:
            if (instr[BRKT_COMMA]->type != COMMA) {
                bracket_error(program, COMMA, BRKT_COMMA, brkt);
                return TRUE;
            }

            if (instr[BRKT_ARG2]->type != arg) {
                bracket_error(program, arg, BRKT_2ARGCLOSE, brkt);
                return TRUE;
            }

            if (strcmp(instr[BRKT_2ARGCLOSE]->attrib, ")")) {
                bracket_error(program, BRACKET, BRKT_2ARGCLOSE, brkt);
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
            set_error(program, index);
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
            set_error(program, index);
            return TRUE;
        default:
            return TRUE;
    }
}

void fillTokenString(prog_t* program, int len) {
    int i;
    for (i = 0; i < len; i++) {
        program->instr[i] = dequeueToken(program);
    }
}
