#include "error.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

typedef struct token_err_t {
    int err;
    char* token;
} token_err_t;

/* TOKENIZER ERRORS */

#define STR_END(s) (s == '\0' || s == '\n')
/* FIXME super dirty cost definition at the moment */

bool_t isINSTR(token_t* token) {
    switch (token->type) {
        case FILE_:
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

void printLocation(token_t* token, char* filename) {
    fprintf(stderr, "ERROR: In file \"%s\", line %d, word %d, \'%s\'. ",
            filename, token->line + 1, token->word + 1, token->attrib);
}

void prog_error(prog_t* program) {
    printLocation(program->instr[0], program->filename);
    fprintf(stderr, "Expected \"{\"\n");

    if (INTERP) {
        exit(EXIT_FAILURE);
    }
    program->pos--;
}

void instr_error(prog_t* program) {
    printLocation(program->instr[0], program->filename);

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

    if (INTERP) {
        exit(EXIT_FAILURE);
    }

    /* Parse to next line, can the same line be interpreted? */
    /* FIXME functionise as recoved error */
    recoverError(program);
}

void recoverError(prog_t* program) {
    token_t* token;

    program->pos++;
    do {
        token = dequeueToken(program);
    } while (token->word != 0);
    program->pos--;
}

void file_error(prog_t* program) {
    char* attrib = program->instr[1]->attrib;

    printLocation(program->instr[1], program->filename);

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

    if (INTERP) {
        exit(EXIT_FAILURE);
    }

    recoverError(program);
}

/* This is more of a warning */
void abort_error(prog_t* program) {
    if (!INTERP) {
        printLocation(program->instr[0], program->filename);
        fprintf(stderr, "WARNING. Code after abort may not be executed\n");
    }
}

/* FIXME this offset is ugly at the moment */
void bracket_error(prog_t* program, type_t expected, int index) {
    printLocation(program->instr[index + 1], program->filename);

    switch (expected) {
        case BRACKET:
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
            break;
    }

    if (INTERP) {
        exit(EXIT_FAILURE);
    }

    recoverError(program);
}

void in2str_error(prog_t* program, int index) {
    printLocation(program->instr[index], program->filename);

}

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
