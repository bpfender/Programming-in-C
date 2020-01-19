#include "parser.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interpreter.h"
#include "symbols.h"
#include "syntax.h"

#define INTERP 1

/* FIXME need a gettoken function for bounds checking */
/* FIXME functionise bracket handling */

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

void parseFile(prog_t* program, symbol_t* symbols, mvm* files) {
    char* p;
    prog(program, symbols, files);

    if (program->pos == program->len) {
        printf("\n\nParsed ok\n");
    } else {
        ON_ERROR("\n\nTokens left\n");
    }

    p = mvm_print(symbols->files);
    printf("%s\n", p);
    free(p);
    p = mvm_print(symbols->vars);
    printf("%s\n", p);
    free(p);
}

void prog(prog_t* program, symbol_t* symbols, mvm* files) {
    token_t* token = dequeueToken(program);

    if (!strcmp(token->attrib, "{")) {
        instr(program, symbols, files);
    } else {
        ERROR(token);
    }
}

/* FIXME GET RID OF MAGIC NUMBERS */
void instr(prog_t* program, symbol_t* symbols, mvm* files) {
    token_t* token = peekToken(program, 0);

    switch (token->type) {
        case FILE_:
            fillTokenString(program, program->instr, 2);
            file(program, symbols, files);
            break;
        case ABORT:
            fillTokenString(program, program->instr, 1);
            prog_abort(program, symbols, files);
            return;
            break;
        case IN2STR:
            fillTokenString(program, program->instr, 6);
            in2str(program, symbols, files);
            break;
        case INNUM:
            fillTokenString(program, program->instr, 4);
            innum(program, symbols, files);
            break;
        case IFEQUAL:
            fillTokenString(program, program->instr, 6);
            ifequal(program, symbols, files);
            break;
        case IFGREATER:
            fillTokenString(program, program->instr, 6);
            ifgreater(program, symbols, files);
            break;
        case INC:
            fillTokenString(program, program->instr, 4);
            inc(program, symbols, files);
            break;
        case JUMP:
            fillTokenString(program, program->instr, 2);
            jump(program, symbols, files);
            break;
        case PRINT:
        case PRINTN:
            fillTokenString(program, program->instr, 2);
            print(program, symbols, files);
            break;
        case RND:
            fillTokenString(program, program->instr, 4);
            rnd(program, symbols, files);
            break;
        case STRVAR:
        case NUMVAR:
            fillTokenString(program, program->instr, 3);
            set(program, symbols, files);
            break;
        case SECTION:
            if (!strcmp(token->attrib, "}")) {
                dequeueToken(program);
                break;
            }
            ERROR(token);
            break;
        case SET:
        case STRCON:
        case NUMCON:
        case BRACKET:
        case COMMA:
        case ERROR:
        default:
            ERROR(token);
            break;
    }
}

/* FIXME super dirty filenmae hack this can be fixed with symbol table*/
void file(prog_t* program, symbol_t* symbols, mvm* files) {
    char filename[500] = "./Files/";

    prog_t* next_program;
    token_t* token = program->instr[1];

    if (token->type == STRCON) {
        getSTRCON(token->attrib);
        strcat(filename, token->attrib);

        if (!getFilename(symbols, filename) || INTERP) {
            next_program = tokenizeFile(filename, symbols);
            addFilename(symbols, filename, NULL);

            prog(next_program, symbols, files);

            freeProgQueue(next_program);
            printf("Finished %s\n", filename);
        }

        instr(program, symbols, files);

    } else {
        ERROR(token);
    }
}

/* Is this the right condition for prog_abort? */
void prog_abort(prog_t* program, symbol_t* symbols, mvm* files) {
    token_t* token = dequeueToken(program);

    if (token->type == SECTION && !strcmp(token->attrib, "}")) {
        return;
    } else {
        ERROR(token);
    }

    if (INTERP) {
        printf("PROGRAM ENDED\n");
        exit(EXIT_SUCCESS);
    }
}

void in2str(prog_t* program, symbol_t* symbols, mvm* files) {
    if (parseBracketsEdit(program->instr + 1, STRVAR, TWO_ARG_LEN)) {
        /* FIXME not sure about this */
        ERROR(program->instr[0]);
    }
    if (INTERP) {
        inter_in2str(program, symbols);
    }
    instr(program, symbols, files);
}

void innum(prog_t* program, symbol_t* symbols, mvm* files) {
    if (parseBracketsEdit(program->instr + 1, NUMVAR, ONE_ARG_LEN)) {
        ERROR(program->instr[0]);
    }
    if (INTERP) {
        inter_innum(program, symbols);
    }
    instr(program, symbols, files);
}

/* FIXME ifequal and ifgreater are identical */
/* QUESTION Does this need two sets of instr for {} */
void ifequal(prog_t* program, symbol_t* symbols, mvm* files) {
    token_t* token = program->instr[0];

    if (parseCondBracketEdit(program->instr + 1)) {
        ERROR(token);
    }

    if (!INTERP || inter_ifequal(program, symbols)) {
        prog(program, symbols, files);
    }

    instr(program, symbols, files);
}

void ifgreater(prog_t* program, symbol_t* symbols, mvm* files) {
    token_t* token = program->instr[0];

    if (parseCondBracketEdit(program->instr + 1)) {
        ERROR(token);
    }

    /* Relies on evaluation order */
    if (!INTERP || inter_ifgreater(program, symbols)) {
        prog(program, symbols, files);
    }

    instr(program, symbols, files);
}

void inc(prog_t* program, symbol_t* symbols, mvm* files) {
    if (parseBracketsEdit(program->instr + 1, NUMVAR, ONE_ARG_LEN)) {
        ERROR(peekToken(program, 0));
    }
    if (INTERP) {
        inter_inc(program, symbols);
    }

    instr(program, symbols, files);
}

void jump(prog_t* program, symbol_t* symbols, mvm* files) {
    token_t* token = program->instr[1];

    if (token->type != NUMCON) {
        ERROR(token);
    }

    if (INTERP) {
        inter_jump(program);
    }

    instr(program, symbols, files);
}

void print(prog_t* program, symbol_t* symbols, mvm* files) {
    token_t* token = program->instr[1];

    if (token->type != STRVAR && token->type != NUMVAR &&
        token->type != STRCON && token->type != NUMCON) {
        ERROR(token);
    }

    if (INTERP) {
        inter_print(program, symbols);
    }

    instr(program, symbols, files);
}

void rnd(prog_t* program, symbol_t* symbols, mvm* files) {
    if (parseBracketsEdit(program->instr + 1, NUMVAR, ONE_ARG_LEN)) {
        ERROR(program->instr[0]);
    }
    if (INTERP) {
        inter_rnd(program, symbols);
    }

    instr(program, symbols, files);
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

void set(prog_t* program, symbol_t* symbols, mvm* files) {
    if (!parseSetVals(program)) {
        ERROR(program->instr[0]);
    }

    if (INTERP) {
        inter_set(program, symbols);
    }

    instr(program, symbols, files);
}

token_t* parseCondBracketEdit(token_t* tokens[TWO_ARG_LEN]) {
    if (strcmp(tokens[0]->attrib, "(")) {
        return tokens[0];
    }

    if (!(tokens[1]->type == STRVAR || tokens[1]->type == NUMVAR ||
          tokens[1]->type == STRCON || tokens[1]->type == NUMCON)) {
        return tokens[1];
    }

    if (tokens[2]->type != COMMA) {
        return tokens[2];
    }

    switch (tokens[1]->type) {
        case STRVAR:
        case STRCON:
            if (!(tokens[3]->type == STRVAR || tokens[3]->type == STRCON)) {
                return tokens[3];
            }
            break;
        case NUMVAR:
        case NUMCON:
            if (!(tokens[3]->type == NUMVAR || tokens[3]->type == NUMCON)) {
                return tokens[3];
            }
            break;
        default:
            break;
    }

    if (strcmp(tokens[4]->attrib, ")")) {
        return tokens[4];
    }

    return NULL;
}

/* FIXME rename this function when ready */
token_t* parseBracketsEdit(token_t* tokens[], type_t arg, int len) {
    int i;
    if (strcmp(tokens[0]->attrib, "(")) {
        return tokens[0];
    }

    for (i = 1; i < len - 2; i += 2) {
        if (tokens[i]->type != arg) {
            return tokens[i];
        }
        if (tokens[i + 1]->type != COMMA) {
            return tokens[i + 1];
        }
    }

    if (tokens[len - 2]->type != arg) {
        return tokens[len - 2];
    }
    if (strcmp(tokens[len - 1]->attrib, ")")) {
        return tokens[len - 1];
    }
    return NULL;
}

void fillTokenString(prog_t* program, token_t* tokens[], int len) {
    int i;
    for (i = 0; i < len; i++) {
        tokens[i] = dequeueToken(program);
    }
}

void handleError(void) {
    ON_ERROR("PANIC ERROR\n");
}

/* decode strconsts in place */
void getSTRCON(char* word) {
    int len;
    if (word[0] == '"' || word[0] == '#') {
        len = strlen(word);
        word[len - 1] = '\0';

        if (word[0] == '#') {
            rot18(word + 1);
        }

        memmove(word, word + 1, sizeof(char) * len - 1);
    }
}

void rot18(char* s) {
    int i;
    for (i = 0; s[i] != '\0'; i++) {
        if (isupper(s[i])) {
            s[i] = 'A' + (s[i] - 'A' + ROT13) % ALPHA;
        } else if (islower(s[i])) {
            s[i] = 'a' + (s[i] - 'a' + ROT13) % ALPHA;
        } else if (isdigit(s[i])) {
            s[i] = (s[i] + ROT5) % DIGIT;
        }
    }
}
