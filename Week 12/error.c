#include <ctype.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "interpreter.h"
#include "parser.h"

typedef struct token_err_t {
    int err;
    char* token;
} token_err_t;

#define STR_END(s) (s == '\0' || s == '\n')

/* ------- PARSING ERROR FUNCTIONS ------- */

void err_prog(prog_t* program) {
    err_printLocation(program->instr[0], program->filename);
    fprintf(stderr, "Expected \"{\"\n");

#if defined INTERP || !defined EXTENSION
    exit(EXIT_FAILURE);
#endif

    program->pos--;
}

void err_instr(prog_t* program) {
    err_printLocation(program->instr[0], program->filename);

    switch (program->instr[0]->type) {
        case ERROR:
            if (strlen(program->instr[0]->attrib) < MATR_SIZE) {
                suggestCorrectToken(program->instr[0]->attrib);
            } else {
                fprintf(stderr, "Invalid token string\n");
            }
            break;
        case SECTION:
            fprintf(stderr, "Did you mean \"}\"?\n");
            break;
        case SET:
            /* If first token is =, might be missing the VAR */
            fprintf(stderr, "Missing assignment variable\n");
            break;
        case STRCON:
        case NUMCON:
            fprintf(stderr, "Constants cannot be assigned a value\n");
            break;
        case BRACKET:
        case COMMA:
            fprintf(stderr, "Invalid character here\n");
            break;
        default:
            fprintf(stderr, "Undefined error\n");
            break;
    }

#if defined INTERP || !defined EXTENSION
    exit(EXIT_FAILURE);
#endif

    /* Parse to next line and continue parsing */
    err_recoverError(program);
}

void err_file(prog_t* program) {
    char* attrib = program->instr[ARG_INDEX]->attrib;

    err_printLocation(program->instr[ARG_INDEX], program->filename);

    switch (program->instr[ARG_INDEX]->type) {
        case STRCON:
            fprintf(stderr, "Unable to open file specified as '%s'\n",
                    program->instr[1]->attrib);
            break;
        default:
            if (attrib[0] == '"' || attrib[strlen(attrib) - 1] == '"' ||
                attrib[0] == '#' || attrib[strlen(attrib) - 1] == '#') {
                fprintf(stderr, "String may be missing '\"' or '#'\n");
            } else {
                fprintf(stderr, "FILE expected STRVAR referencing filename\n");
            }
            break;
    }

#if defined INTERP || !defined EXTENSION
    exit(EXIT_FAILURE);
#endif

    err_recoverError(program);
}

void err_cond(prog_t* program, int index) {
    err_printLocation(program->instr[index], program->filename);

    switch (index) {
        case BRKT_OPEN:
            fprintf(stderr, "Expected '(' before statement\n");
            break;
        case BRKT_ARG1:
            fprintf(stderr, "Expected STR or NUM argument\n");
            break;
        case BRKT_COMMA:
            fprintf(stderr, "Arguments should be seperated with ','\n");
            break;
        case BRKT_ARG2:
            fprintf(stderr, "Arguments must match. ");
            if (program->instr[2]->type == STRCON || program->instr[2]->type == STRVAR) {
                fprintf(stderr, "Expected STR argument\n");
            } else {
                fprintf(stderr, "Expected NUM argument\n");
            }
            break;
        case BRKT_2ARGCLOSE:
            fprintf(stderr, "Expected ')' at end of statement\n");
            break;
        default:
            printf("Undefined condition error\n");
            break;
    }

#if defined INTERP || !defined EXTENSION
    exit(EXIT_FAILURE);
#endif

    err_recoverError(program);
}

void err_bracket(prog_t* program, type_t expected, int index) {
    err_printLocation(program->instr[index], program->filename);

    switch (expected) {
        case BRACKET:
            if (index == BRKT_OPEN) {
                fprintf(stderr, "Expected '(' before statement\n");
            } else {
                fprintf(stderr, "Expected ')' at end of statement\n");
            }
            break;
        case NUMVAR:
            if (index == BRKT_ARG1 || index == BRKT_ARG2) {
                fprintf(stderr, "Expected NUMVAR argument\n");
            }
            break;
        case STRVAR:
            if (index == BRKT_ARG1 || index == BRKT_ARG2) {
                fprintf(stderr, "Expected STRVAR argument\n");
            } else {
                fprintf(stderr, "Arguments must match and both be STRVAR\n");
            }
            break;
        case COMMA:
            fprintf(stderr, "Arguments should be seperated with ','\n");
            break;
        default:
            fprintf(stderr, "Undefined bracket error\n");
            printf("\n");
            break;
    }

#if defined INTERP || !defined EXTENSION
    exit(EXIT_FAILURE);
#endif

    err_recoverError(program);
}

void err_jump(prog_t* program) {
    err_printLocation(program->instr[ARG_INDEX], program->filename);
    fprintf(stderr, "Expected NUMCON jump value\n");
    err_recoverError(program);
}

void err_print(prog_t* program) {
    err_printLocation(program->instr[ARG_INDEX], program->filename);
    fprintf(stderr, "Expected VARCON print value\n");
    err_recoverError(program);
}

void err_set(prog_t* program, int index) {
    err_printLocation(program->instr[index], program->filename);

    switch (index) {
        case SET_OP:
            fprintf(stderr, "Expected VAR to assign value to\n");
            break;
        case SET_VAL:
            if (program->instr[SET_VAL]->type == STRCON ||
                program->instr[SET_VAL]->type == STRVAR) {
                fprintf(stderr, "Requires NUM value in assignment\n");
            } else if (program->instr[SET_VAL]->type == NUMCON ||
                       program->instr[SET_VAL]->type == NUMVAR) {
                fprintf(stderr, "Requires STR value in assignment\n");
            } else {
                fprintf(stderr, "Expected valid value to be assigned\n");
            }
            break;
        default:
            fprintf(stderr, "Undefined SET error\n");
            break;
    }
    err_recoverError(program);
}

void err_nextToken(void) {
    fprintf(stderr, "ERROR: No tokens left. SECTION brackets missing in file\n");
    exit(EXIT_FAILURE);
}

void err_extraTokens(void) {
    fprintf(stderr, "ERROR: Tokens left in file to parse\n");
    exit(EXIT_FAILURE);
}

/* ------- INTERPRETER ERROR FUNCTIONS ------- */
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

/* ------- GENERAL ERROR FUNCTIONS -------- */

void err_printLocation(token_t* token, char* filename) {
    fprintf(stderr, "ERROR: In file \"%s\", line %d, word %d. ",
            filename, token->line + 1, token->word + 1);
}

void err_recoverError(prog_t* program) {
    token_t* token;

    /* Rewind to beginning of line */
    while (program->token[program->pos].word != 0) {
        program->pos--;
    }
    program->pos++;

    /* Move through tokens until new line is reached */
    do {
        token = nextToken(program);
    } while (token->word != 0);
    program->pos--;
}

void suggestCorrectToken(char* word) {
    unsigned int i;
    int dist = INT_MAX;
    int new_dist;
    unsigned int index;
    size_t cost[MATR_SIZE][MATR_SIZE];
    const char instructions[][MATR_SIZE] =
        {"FILE", "ABORT", "IN2STR", "INNUM", "JUMP",
         "PRINT", "PRINTN", "RND", "IFEQUAL", "IFCOND", "INC"};

    if (word[0] == '"' || word[strlen(word) - 1] == '"') {
        fprintf(stderr, "Did you mean to type a string\n");
        return;
    }
    if (word[0] == '#' || word[strlen(word) - 1] == '#') {
        fprintf(stderr, "Did you mean to type a string\n");
        return;
    }
    if (strlen(word) == 1) {
        fprintf(stderr, "Undefined character\n");
        return;
    }

    for (i = 0; i < (sizeof(instructions) / sizeof(instructions[0])); i++) {
        if ((new_dist = calculateMSD(instructions[i], word, cost)) < dist) {
            dist = new_dist;
            index = i;
        }
    }
    fprintf(stderr, "Did you mean %s\n", instructions[index]);
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
