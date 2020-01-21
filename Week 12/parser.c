#include "parser.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interpreter.h"
#include "symbols.h"
#include "syntax.h"

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
    token_t* token = program->instr[0] = dequeueToken(program);

    if (strcmp(token->attrib, "{")) {
        prog_error(program);
    }
    instr(program, symbols, files);
}

/* FIXME GET RID OF MAGIC NUMBERS */
void instr(prog_t* program, symbol_t* symbols, mvm* files) {
    token_t* token = program->instr[0] = peekToken(program, 0);

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
            if (strcmp(token->attrib, "}")) {
                instr_error(program);
                instr(program, symbols, files);
                break;
            }
            /* FIXME is this good form */
            dequeueToken(program);
            break;
        case ERROR:
        case SET:
        case STRCON:
        case NUMCON:
        case BRACKET:
        case COMMA:
        default:
            instr_error(program);
            instr(program, symbols, files);
            break;
    }
}

/* FIXME super dirty filenmae hack this can be fixed with symbol table*/
void file(prog_t* program, symbol_t* symbols, mvm* files) {
    char filename[500] = "./Files/";

    prog_t* next_program;
    token_t* token = program->instr[1];

    if (token->type == STRCON) {
        /* getSTRCON(token->attrib);*/
        strcat(filename, token->attrib);

        if (!getFilename(symbols, filename) || INTERP) {
            if (!(next_program = tokenizeFile(filename, symbols))) {
                file_error(program);
            } else {
                addFilename(symbols, filename, NULL);

                prog(next_program, symbols, files);

                freeProgQueue(next_program);
                printf("\n\nFinished %s\n", filename);
            }
        }
    } else {
        file_error(program);
    }

    instr(program, symbols, files);
}

/* Is this the right condition for prog_abort? */
void prog_abort(prog_t* program, symbol_t* symbols, mvm* files) {
    token_t* token = peekToken(program, 0);

    if (strcmp(token->attrib, "}")) {
        abort_error(program);
    }

    if (INTERP) {
        printf("\nPROGRAM END: Program completed successfully\n");
        exit(EXIT_SUCCESS);
    }

    instr(program, symbols, files);
}

void in2str(prog_t* program, symbol_t* symbols, mvm* files) {
    parseBrackets(program, STRVAR, TWO_ARG_LEN);

    if (INTERP) {
        inter_in2str(program, symbols);
    }

    instr(program, symbols, files);
}

void innum(prog_t* program, symbol_t* symbols, mvm* files) {
    if (parseBrackets(program, NUMVAR, ONE_ARG_LEN)) {
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

    if (parseCondBracket(program)) {
        ERROR(token);
    }

    if (!INTERP || inter_ifequal(program, symbols)) {
        prog(program, symbols, files);
    }

    instr(program, symbols, files);
}

void ifgreater(prog_t* program, symbol_t* symbols, mvm* files) {
    token_t* token = program->instr[0];

    if (parseCondBracket(program)) {
        ERROR(token);
    }

    /* Relies on evaluation order */
    if (!INTERP || inter_ifgreater(program, symbols)) {
        prog(program, symbols, files);
    }

    instr(program, symbols, files);
}

void inc(prog_t* program, symbol_t* symbols, mvm* files) {
    if (parseBrackets(program, NUMVAR, ONE_ARG_LEN)) {
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
    if (parseBrackets(program, NUMVAR, ONE_ARG_LEN)) {
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

bool_t parseCondBracketEdit(token_t* tokens[TWO_ARG_LEN]) {
    bool_t error_flag = FALSE;

    if (strcmp(tokens[0]->attrib, "(")) {
        error_flag = TRUE;
    }

    if (!(tokens[1]->type == STRVAR || tokens[1]->type == NUMVAR ||
          tokens[1]->type == STRCON || tokens[1]->type == NUMCON)) {
        error_flag = TRUE;
    }

    if (tokens[2]->type != COMMA) {
        error_flag = TRUE;
    }

    switch (tokens[1]->type) {
        case STRVAR:
        case STRCON:
            if (!(tokens[3]->type == STRVAR || tokens[3]->type == STRCON)) {
                error_flag = TRUE;
            }
            break;
        case NUMVAR:
        case NUMCON:
            if (!(tokens[3]->type == NUMVAR || tokens[3]->type == NUMCON)) {
                error_flag = TRUE;
            }
            break;
        default:
            break;
    }

    if (strcmp(tokens[4]->attrib, ")")) {
        error_flag = TRUE;
    }

    return error_flag;
}

bool_t parseCondBracket(prog_t* program){
    token_t** tokens = program->instr+1;

    if (strcmp(tokens[0]->attrib, "(")) {
        return TRUE;
    }

    if (!(tokens[1]->type == STRVAR || tokens[1]->type == NUMVAR ||
          tokens[1]->type == STRCON || tokens[1]->type == NUMCON)) {
        return TRUE;
    }

    if (tokens[2]->type != COMMA) {
        return TRUE;
    }

    switch (tokens[1]->type) {
        case STRVAR:
        case STRCON:
            if (!(tokens[3]->type == STRVAR || tokens[3]->type == STRCON)) {
                return TRUE;
            }
            break;
        case NUMVAR:
        case NUMCON:
            if (!(tokens[3]->type == NUMVAR || tokens[3]->type == NUMCON)) {
                return TRUE;
            }
            break;
        default:
            break;
    }

    if (strcmp(tokens[4]->attrib, ")")) {
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
        bracket_error(program, BRACKET, 0);
        return TRUE;
    }

    for (i = 1; i < len - 2; i += 2) {
        if (tokens[i]->type != arg) {
            bracket_error(program, arg, i);
            return TRUE;
        }
        if (tokens[i + 1]->type != COMMA) {
            bracket_error(program, COMMA, i+1);
            return TRUE;
        }
    }

    if (tokens[len - 2]->type != arg) {
        bracket_error(program, arg, len-2);
        return TRUE;
    }
    if (strcmp(tokens[len - 1]->attrib, ")")) {
        bracket_error(program, BRACKET, len-1);
        return TRUE;
    }

    return FALSE;
}

/* FIXME rename this function when ready */
bool_t parseBracketsEdit(token_t* tokens[], type_t arg, int len) {
    int i;
    bool_t error_flag = FALSE;

    if (strcmp(tokens[0]->attrib, "(")) {
        error_flag = TRUE;
    }

    for (i = 1; i < len - 2; i += 2) {
        if (tokens[i]->type != arg) {
            error_flag = TRUE;
        }
        if (tokens[i + 1]->type != COMMA) {
            error_flag = TRUE;
        }
    }

    if (tokens[len - 2]->type != arg) {
        error_flag = TRUE;
    }
    if (strcmp(tokens[len - 1]->attrib, ")")) {
        error_flag = TRUE;
    }
    return error_flag;
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
