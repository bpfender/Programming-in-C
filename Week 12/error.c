#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "interpreter.h"

typedef struct token_err_t {
    int err;
    char* token;
} token_err_t;

#define STR_END(s) (s == '\0' || s == '\n')

bool_t isINSTR(token_t* token) {
    switch (token->type) {
        case FILE_REF:
        case ABORT:
        case IN2STR:
        case INNUM:
        case JUMP:
        case PRINT:
        case PRINTN:
        case RND:
        case IFEQUAL:
        case IFGREATER:
        case INC:
        case STRVAR:
        case NUMVAR:
            return TRUE;
            break;
        default:
            return FALSE;
            break;
    }
}

void err_printLocation(token_t* token, char* filename) {
    fprintf(stderr, "ERROR: In file \"%s\", line %d, word %d, \'%s\'. ",
            filename, token->line + 1, token->word + 1, token->attrib);
}

void err_prog(prog_t* program) {
    err_printLocation(program->instr[0], program->filename);
    fprintf(stderr, "Expected \"{\"\n");

#if defined INTERP || !defined EXTENSION
    exit(EXIT_FAILURE);
#endif

    program->pos--;
}

void err_lex(prog_t* program, char* filename) {
    err_printLocation(program->instr[1], program->filename);
    fprintf(stderr, "Failed to open %s\n", filename);

#if defined INTERP || !defined EXTENSION
    exit(EXIT_FAILURE);
#endif

    err_recoverError(program);
}

void err_instr(prog_t* program) {
    err_printLocation(program->instr[0], program->filename);

    switch (program->instr[0]->type) {
        case ERROR:
            /*FIXME HERE Extra error handling*/
            suggestCorrectToken(program->instr[0]->attrib);
            break;
        case SECTION:
            fprintf(stderr, "Did you mean \"}\"?\n");
            break;
        case SET:
            fprintf(stderr, "Missing assignment variable\n");
            break;
        case STRCON:
        case NUMCON:
            fprintf(stderr, "Constants cannot be assigned a value\n");
            break;
        case BRACKET:
        case COMMA:
            fprintf(stderr, "Invalid character\n");
            break;
        default:
            break;
    }

#if defined INTERP || !defined EXTENSION
    exit(EXIT_FAILURE);
#endif

    /* Parse to next line, can the same line be interpreted? */
    /* FIXME functionise as recoved error */
    err_recoverError(program);
}

void err_recoverError(prog_t* program) {
    token_t* token;

    if (program->token[program->pos].word == 0) {
        program->pos++;
    }

    do {
        token = nextToken(program);
    } while (token->word != 0);
    program->pos--;
}

void err_file(prog_t* program) {
    char* attrib = program->instr[1]->attrib;

    err_printLocation(program->instr[1], program->filename);

    switch (program->instr[1]->type) {
        case STRCON:
            fprintf(stderr, "Unable to open file specified as '%s'\n", program->instr[1]->attrib);
            break;
        default:
            if (attrib[0] == '"' || attrib[strlen(attrib) - 1] == '"' ||
                attrib[0] == '#' || attrib[strlen(attrib) - 1] == '#') {
                fprintf(stderr, "String may be missing '\"' or '#'\n");
            } else {
                fprintf(stderr, "Expected string referencing filename\n");
            }
            break;
    }

#if defined INTERP || !defined EXTENSION
    exit(EXIT_FAILURE);
#endif

    /* FIXME is this correct */
    err_recoverError(program);
}

/* This is more of a warning */
void err_abort(prog_t* program) {
#if defined EXTENSION && !defined INTERP
    err_printLocation(program->instr[0], program->filename);
    fprintf(stderr, "WARNING. Code after abort may not be executed\n");
#endif
}

void err_bracket(prog_t* program, type_t expected, int index, int len) {
    err_printLocation(program->instr[index], program->filename);

    switch (expected) {
        case BRACKET:
            printf("Hello2\n");
            if (index == 0) {
                fprintf(stderr, "Expected '(' before statement\n");
            } else {
                fprintf(stderr, "Expected ')' at end of statement\n");
            }
            break;
        case NUMVAR:
            if (index == 1) {
                fprintf(stderr, "Expected NUMVAR argument\n");
            }
            break;
        case STRVAR:
            if (index == 1) {
                fprintf(stderr, "Expected STRVAR argument\n");
            } else {
                fprintf(stderr, "Arguments must match and both be STRVAR\n");
            }
            break;
        case COMMA:
            fprintf(stderr, "Arguments should be seperated with ','\n");
            break;
        default:
            fprintf(stderr, "Undefined error\n");
            printf("\n");
            break;
    }

#if defined INTERP || !defined EXTENSION
    exit(EXIT_FAILURE);
#endif

    program->pos -= len;
    err_recoverError(program);
}

/* Need to handle missing brackets */
void err_cond(prog_t* program, int index) {
    err_printLocation(program->instr[index], program->filename);

    switch (index) {
        case 0:
            fprintf(stderr, "Expected '(' before statement\n");
            break;
        case 1:
            fprintf(stderr, "Expected STR or NUM argument\n");
            break;
        case 2:
            fprintf(stderr, "Arguments should be seperated with ','\n");
            break;
        case 3:
            fprintf(stderr, "Arguments must match. ");
            if (program->instr[2]->type == STRCON || program->instr[2]->type == STRVAR) {
                fprintf(stderr, "Expected STR argument\n");
            } else {
                fprintf(stderr, "Expected NUM argument\n");
            }
            break;
        case 4:
            fprintf(stderr, "Expected ')' at end of statement\n");
            break;
        default:
            printf("\n");
            break;
    }

#if defined INTERP || !defined EXTENSION
    exit(EXIT_FAILURE);
#endif

    program->pos = program->pos - 4 + index;
}

/* FIXME can this be combined with other single args ? */
void err_jump(prog_t* program) {
    if (program->instr[1]->word == 0) {
        err_printLocation(program->instr[0], program->filename);
        fprintf(stderr, "Expect NUMCON jump value\n");
        program->pos -= 1;
    } else {
        err_printLocation(program->instr[1], program->filename);
        fprintf(stderr, "Expect NUMCON jump value\n");
        /*err_recoverError(program);*/
    }
}

void err_print(prog_t* program) {
    if (program->instr[1]->word == 0) {
        err_printLocation(program->instr[0], program->filename);
        fprintf(stderr, "Expected VARCON print value\n");
        program->pos -= 1;
    } else {
        err_printLocation(program->instr[1], program->filename);
        fprintf(stderr, "Expect VARCON print value\n");
        /*err_recoverError(program);*/
    }
}

void err_set(prog_t* program, int index) {
    err_printLocation(program->instr[index], program->filename);

    switch (index) {
        case 1:
            fprintf(stderr, "Expected assignment to VAR\n");
            break;
        case 2:
            if (program->instr[2]->type == STRCON || program->instr[2]->type == STRVAR) {
                fprintf(stderr, "Requires NUM value in assignment\n");
            } else if (program->instr[2]->type == NUMCON || program->instr[2]->type == NUMVAR) {
                fprintf(stderr, "Requires STR value in assignment\n");
            } else {
                fprintf(stderr, "Expect valid value to be assigned\n");
            }
            break;
        default:
            fprintf(stderr, "This is an error\n");
            break;
    }
}

void err_nextToken(prog_t* program) {
    fprintf(stderr, "No tokens left\n");
    exit(EXIT_FAILURE);
}

void err_extraTokens(prog_t* program) {
    fprintf(stderr, "Tokens left\n");
    exit(EXIT_FAILURE);
}

/* FIXME $ % incorrect capitalisation, "." missing number */

void suggestCorrectToken(char* word) {
    unsigned int i;
    int dist = 100;
    int new_dist;
    unsigned int index;
    size_t cost[MATR_SIZE][MATR_SIZE];
    const char instructions[][MATR_SIZE] = {"FILE", "ABORT", "IN2STR", "INNUM", "JUMP", "PRINT", "PRINTN", "RND", "IFEQUAL", "IFCOND", "INC"};

    if (word[0] == '"' || word[strlen(word) - 1] == '"') {
        printf("Did you mean to type a string\n");
        return;
    }
    if (word[0] == '#' || word[strlen(word) - 1] == '#') {
        printf("Did you mean to type a string\n");
        return;
    }
    if (strlen(word) == 1) {
        printf("Undefined character\n");
        return;
    }

    for (i = 0; i < (sizeof(instructions) / sizeof(instructions[0])); i++) {
        if ((new_dist = calculateMSD(instructions[i], word, cost)) < dist) {
            dist = new_dist;
            index = i;
        }
    }
    printf("Did you mean %s\n", instructions[index]);
}

/* Based on https://en.wikipedia.org/wiki/Wagner%E2%80%93Fischer_algorithm
 * and https://en.wikipedia.org/wiki/Levenshtein_distance
 */
size_t calculateMSD(const char* sample, char* data, size_t cost[MATR_SIZE][MATR_SIZE]) {
    size_t i, j;
    int substitution;

    /* Initialise top left corner of cost array */
    cost[0][0] = 0;

    for (i = 1; !STR_END(sample[i - 1]); i++) {
        cost[i][0] = i;
    }
    for (j = 1; !STR_END(data[j - 1]); j++) {
        cost[0][j] = j;
    }
    for (j = 1; !STR_END(data[j - 1]); j++) {
        for (i = 1; !STR_END(sample[i - 1]); i++) {
            substitution = (toupper(data[j - 1]) == sample[i - 1]) ? 0 : 1;

            cost[i][j] = getMin(cost[i - 1][j] + 1,
                                cost[i][j - 1] + 1,
                                cost[i - 1][j - 1] + substitution);
        }
    }

    /* Remove increment from last instance of for loop */
    return cost[i - 1][j - 1];
}

size_t getMin(size_t a, size_t b, size_t c) {
    size_t tmp = a < b ? a : b;
    return tmp < c ? tmp : c;
}

void err_interVAR(prog_t* program, int index) {
    err_printLocation(program->instr[index], program->filename);
    fprintf(stderr, "INTERPRETER: VAR %s not initialised.", program->instr[index]->attrib);
    exit(EXIT_FAILURE);
}

void err_interInput(prog_t* program, type_t type) {
    err_printLocation(program->instr[0], program->filename);

    switch (type) {
        case INNUM:
            fprintf(stderr, "INTERPRETER: Expected single number input.\n");
            break;
        case IN2STR:
            fprintf(stderr, "INTERPRETER: Expected two input strings.\n");
            break;
        case ERROR:
            fprintf(stderr, "INTERPRETER: Input string may not be longer than %d", MAX_INPUT_LEN);
            break;
        default:
            fprintf(stderr, "INTERPRETER: Undefined error.\n");
            break;
    }
    exit(EXIT_FAILURE);
}

void err_interUndef(prog_t* program) {
    err_printLocation(program->instr[0], program->filename);
    ON_ERROR("INTERPRETER: Undefined error.\n");
}

void err_interCond(prog_t* program) {
    err_printLocation(program->instr[0], program->filename);
    ON_ERROR("INTERPRETER: Error in conditional statement brackets.\n");
}
