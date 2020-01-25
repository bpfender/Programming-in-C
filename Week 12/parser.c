#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* FIXME ifdef on interpreter.h */
#include "error.h"
#include "interpreter.h"
#include "parser.h"

#define ARG_INDEX 1

#define SECT_LEN 1
#define ABORT_LEN 1
#define SET_LEN 3
#define SINGLE_ARG_LEN 2
#define SINGLE_BRACKET_LEN 4
#define DOUBLE_BRACKET_LEN 6

#define TWO_ARG_LEN 5
#define ONE_ARG_LEN 3

#define RND_ARGS 1
#define IFCOND_ARGS 2
#define IN2STR_ARGS 2
#define INNUM_ARGS 1
#define INC_ARGS 1

#define ON_ERROR(STR)     \
    fprintf(stderr, STR); \
    exit(EXIT_FAILURE)

#define ERROR(TOKEN)                                                                                           \
    {                                                                                                          \
        fprintf(stderr, "Error code line %d\n Program line %d word %d\n", __LINE__, TOKEN->line, TOKEN->word); \
        exit(2);                                                                                               \
    }

void parseFile(prog_t* program, symbol_t* symbols) {
    prog(program, symbols);

    if (program->pos == program->len) {
        printf("\n%s parsed ok\n", program->filename);
    } else {
        tokenLeft_error(program);
    }
}

void prog(prog_t* program, symbol_t* symbols) {
    token_t* token = program->instr[0] = dequeueToken(program);

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
            fillTokenString(program, program->instr, SINGLE_ARG_LEN);
            file(program, symbols);
            break;
        case ABORT:
            fillTokenString(program, program->instr, ABORT_LEN);
            prog_abort(program, symbols);
            return;
            break;
        case IN2STR:
            fillTokenString(program, program->instr, DOUBLE_BRACKET_LEN);
            in2str(program, symbols);
            break;
        case INNUM:
            fillTokenString(program, program->instr, SINGLE_BRACKET_LEN);
            innum(program, symbols);
            break;
        case IFEQUAL:
            fillTokenString(program, program->instr, DOUBLE_BRACKET_LEN);
            ifequal(program, symbols);
            break;
        case IFGREATER:
            fillTokenString(program, program->instr, DOUBLE_BRACKET_LEN);
            ifgreater(program, symbols);
            break;
        case INC:
            fillTokenString(program, program->instr, SINGLE_BRACKET_LEN);
            inc(program, symbols);
            break;
        case JUMP:
            fillTokenString(program, program->instr, SINGLE_ARG_LEN);
            jump(program, symbols);
            break;
        case PRINT:
        case PRINTN:
            fillTokenString(program, program->instr, SINGLE_ARG_LEN);
            print(program, symbols);
            break;
        case RND:
            fillTokenString(program, program->instr, SINGLE_BRACKET_LEN);
            rnd(program, symbols);
            break;
        case STRVAR:
        case NUMVAR:
            fillTokenString(program, program->instr, SET_LEN);
            set(program, symbols);
            break;
        case SECTION:
            fillTokenString(program, program->instr, SECT_LEN);
            if (strcmp(token->attrib, "}")) {
                instr_error(program);
                /* FIXME could this be part of instr_error()? */
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

    if (token->type == STRCON) {
        filename = token->attrib;

        /* If filename is in the symbol table, file doesn't need to be parsed again.
        However, if interpreting, file needs to be opened again */
        if (!getFilename(symbols, filename) || INTERP) {
            if (!(next_program = tokenizeFile(filename))) {
                file_error(program);
            } else {
                /* Add filename to symbol table to signify that it's been processed */
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

/* FIXME Is this the right condition for prog_abort? */
void prog_abort(prog_t* program, symbol_t* symbols) {
    token_t* token = peekToken(program, 0);

    if (strcmp(token->attrib, "}")) {
        abort_error(program);
    }

    if (INTERP) {
        inter_abort();
    }

    instr(program, symbols);
}

void in2str(prog_t* program, symbol_t* symbols) {
    if (parseBrackets(program, STRVAR, TWO_ARG_LEN)) {
        /* ERROR */
    }

    if (INTERP) {
        inter_in2str(program, symbols);
    }

    instr(program, symbols);
}

void innum(prog_t* program, symbol_t* symbols) {
    if (parseBrackets(program, NUMVAR, ONE_ARG_LEN)) {
        /* ERROR */
    }

    if (INTERP) {
        inter_innum(program, symbols);
    }

    instr(program, symbols);
}

/* FIXME ifequal and ifgreater are identical */
void ifequal(prog_t* program, symbol_t* symbols) {
    if (parseCondBracket(program)) {
    }

    if (!INTERP || inter_ifequal(program, symbols)) {
        prog(program, symbols);
    }

    instr(program, symbols);
}

void ifgreater(prog_t* program, symbol_t* symbols) {
    if (parseCondBracket(program)) {
    }

    if (!INTERP || inter_ifgreater(program, symbols)) {
        prog(program, symbols);
    }

    instr(program, symbols);
}

void inc(prog_t* program, symbol_t* symbols) {
    if (parseBrackets(program, NUMVAR, ONE_ARG_LEN)) {
    }

    if (INTERP) {
        inter_inc(program, symbols);
    }

    instr(program, symbols);
}

void jump(prog_t* program, symbol_t* symbols) {
    token_t* token = program->instr[1];

    if (token->type != NUMCON) {
        jump_error(program);
    }

    if (INTERP) {
        inter_jump(program);
    }

    instr(program, symbols);
}

void print(prog_t* program, symbol_t* symbols) {
    token_t* token = program->instr[1];

    if (token->type != STRVAR && token->type != NUMVAR &&
        token->type != STRCON && token->type != NUMCON) {
        print_error(program);
    }

    if (INTERP) {
        inter_print(program, symbols);
    }

    instr(program, symbols);
}

void rnd(prog_t* program, symbol_t* symbols) {
    if (parseBrackets(program, NUMVAR, ONE_ARG_LEN)) {
    }

    if (INTERP) {
        inter_rnd(program, symbols);
    }

    instr(program, symbols);
}

bool_t parseSetVals(prog_t* program) {
    switch (program->instr[0]->type) {
        case STRVAR:
            if (program->instr[1]->type == SET) {
                if (program->instr[2]->type == STRCON || program->instr[2]->type == STRVAR) {
                    return TRUE;
                }
            }
            return FALSE;
        case NUMVAR:
            if (program->instr[1]->type == SET) {
                if (program->instr[2]->type == NUMVAR || program->instr[2]->type == NUMCON) {
                    return TRUE;
                }
            }
            return FALSE;
        default:
            return FALSE;
    }
}

void set(prog_t* program, symbol_t* symbols) {
    if (!parseSetVals(program)) {
        ERROR(program->instr[0]);
    }

    if (INTERP) {
        inter_set(program, symbols);
    }

    instr(program, symbols);
}

bool_t parseCondBracket(prog_t* program) {
    token_t** tokens = program->instr + 1;

    if (strcmp(tokens[0]->attrib, "(")) {
        cond_error(program, 0);
        return TRUE;
    }

    if (!(tokens[1]->type == STRVAR || tokens[1]->type == NUMVAR ||
          tokens[1]->type == STRCON || tokens[1]->type == NUMCON)) {
        cond_error(program, 1);
        return TRUE;
    }

    if (tokens[2]->type != COMMA) {
        cond_error(program, 2);
        return TRUE;
    }

    switch (tokens[1]->type) {
        case STRVAR:
        case STRCON:
            if (!(tokens[3]->type == STRVAR || tokens[3]->type == STRCON)) {
                cond_error(program, 3);
                return TRUE;
            }
            break;
        case NUMVAR:
        case NUMCON:
            if (!(tokens[3]->type == NUMVAR || tokens[3]->type == NUMCON)) {
                cond_error(program, 3);
                return TRUE;
            }
            break;
        default:
            break;
    }

    if (strcmp(tokens[4]->attrib, ")")) {
        cond_error(program, 4);
        return TRUE;
    }

    return FALSE;
}

/* FIXME magic number problems and numbers passed*/
bool_t parseBrackets(prog_t* program, type_t arg, int len) {
    int i;
    /* FIXME this is cludgy */
    token_t** tokens = program->instr + 1;

    if (strcmp(tokens[0]->attrib, "(")) {
        bracket_error(program, BRACKET, 0, len);
        return TRUE;
    }

    for (i = 1; i < len - 2; i += 2) {
        if (tokens[i]->type != arg) {
            bracket_error(program, arg, i, len);
            return TRUE;
        }
        if (tokens[i + 1]->type != COMMA) {
            bracket_error(program, COMMA, i + 1, len);
            return TRUE;
        }
    }

    if (tokens[len - 2]->type != arg) {
        bracket_error(program, arg, len - 2, len);
        return TRUE;
    }
    if (strcmp(tokens[len - 1]->attrib, ")")) {
        bracket_error(program, BRACKET, len - 1, len);
        return TRUE;
    }

    return FALSE;
}

void fillTokenString(prog_t* program, token_t* tokens[], int len) {
    int i;
    for (i = 0; i < len; i++) {
        tokens[i] = dequeueToken(program);
    }
}
