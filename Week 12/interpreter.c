
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "error.h"
#include "interpreter.h"
#include "symbols.h"

#define RND_RANGE 100

/* FIXME need to convert escape characters */

void inter_abort(void) {
    printf("\nPROGRAM END: Program has ended\n");
    exit(EXIT_SUCCESS);
}

/* FIXME not doing any handling on buffer lengths currently */
void inter_in2str(prog_t* program, symbol_t* symbols) {
    /* FIXME could return cell */
    char line[256];
    char word1[256];
    char word2[256];

    char *w1, *w2;

    /* addVariable(symbols, program->instr[2]->attrib);
    addVariable(symbols, program->instr[4]->attrib);*/

    if (fgets(line, sizeof(line), stdin)) {
        if (sscanf(line, " %s %s ", word1, word2) == 2) {
            w1 = allocString(strlen(word1));
            w2 = allocString(strlen(word2));

            strcpy(w1, word1);
            strcpy(w2, word2);
        } else {
            /* FIXME proper error handlign */
            ON_ERROR("Input error\n");
        }
    }

    updateVariable(symbols, program->instr[2]->attrib, w1);
    updateVariable(symbols, program->instr[4]->attrib, w2);
}

/* FIXME no handling of buffer lengths currently */
void inter_innum(prog_t* program, symbol_t* symbols) {
    char line[256];

    double* num = allocNumber();

    /* addVariable(symbols, program->instr[2]->attrib);*/

    if (fgets(line, sizeof(line), stdin)) {
        if (sscanf(line, "%lf", num) == 1) {
        } else {
            ON_ERROR("Expected single number input\n");
        }
    }

    updateVariable(symbols, program->instr[2]->attrib, num);
}

void inter_jump(prog_t* program) {
    int pos;
    if (!checkJumpValue(program->instr[1]->attrib)) {
        ON_ERROR("Jump value should be integer");
    }

    pos = atoi(program->instr[1]->attrib);

    if (!checkValidJump(program, pos)) {
        ON_ERROR("Invalid jump\n");
    }
    program->pos = pos;
}

bool_t checkValidJump(prog_t* program, int pos) {
    if (pos >= program->len) {
        return FALSE;
    }

    if (!isINSTR(&program->token[pos])) {
        return FALSE;
    }
    return TRUE;
}

bool_t checkJumpValue(char* num) {
    int i;
    bool_t dot = FALSE;
    for (i = 0; num[i] != '\0'; i++) {
        if (num[i] == '.') {
            return FALSE;
        }
    }
    return TRUE;
}

void inter_print(prog_t* program, symbol_t* symbols) {
    /*FIXME should this be functionised */
    mvmcell* cell;

    switch (program->instr[1]->type) {
        case NUMVAR:
            cell = getVariable(symbols, program->instr[1]->attrib);
            if (!cell) {
                printf("%s ", program->instr[1]->attrib);
                ON_ERROR("Variable has not been initialised\n");
            }
            printf("%f", *(double*)cell->data);
            break;
        case STRVAR:
            cell = getVariable(symbols, program->instr[1]->attrib);
            if (!cell) {
                printf("%s ", program->instr[1]->attrib);
                ON_ERROR("Variable has not been initialised\n");
            }
            printf("%s ", (char*)cell->data);
            break;
        case NUMCON:
        case STRCON:
            printf("%s ", program->instr[1]->attrib);
            break;
        default:
            ON_ERROR("Something went wrong\n");
    }

    if (program->instr[0]->type == PRINTN) {
        printf("\n");
    }
}

/* FIXME ints vs doubles? */
void inter_rnd(prog_t* program, symbol_t* symbols) {
    double* rnd = allocNumber();

    addVariable(symbols, program->instr[2]->attrib);

    /* FIXME not sure about this generation also make sure to call seed fct */
    *rnd = rand() % RND_RANGE;

    updateVariable(symbols, program->instr[2]->attrib, rnd);
}

bool_t inter_ifequal(prog_t* program, symbol_t* symbols) {
    void* arg1 = getArg(program->instr[2], symbols);
    void* arg2 = getArg(program->instr[4], symbols);

    switch (program->instr[2]->type) {
        case STRVAR:
        case STRCON:
            if (!strcmp(arg1, arg2)) {
                return TRUE;
            } else {
                findElseJump(program);
                return FALSE;
            }
            break;
        case NUMVAR:
        case NUMCON:
            /*FIXME this might not be right. Add tolerance ? */
            if (*(double*)arg1 == *(double*)arg2) {
                return TRUE;
            } else {
                findElseJump(program);
                return FALSE;
            }
            break;
        default:
            ON_ERROR("Something went wrong\n");
            break;
    }
}

void findElseJump(prog_t* program) {
    int nest = 1;
    token_t* token = dequeueToken(program);

    if (strcmp(token->attrib, "{")) {
        ON_ERROR("If else statement error\n");
    }

    while (strcmp(token->attrib, "}") || nest) {
        /*FIXME extra error handling */
        token = dequeueToken(program);
        if (!strcmp(token->attrib, "}")) {
            nest--;
        }
        if (!strcmp(token->attrib, "{")) {
            nest++;
        }
    }
}

/* Not quite sure about number detection here */
bool_t inter_ifgreater(prog_t* program, symbol_t* symbols) {
    void* arg1 = getArg(program->instr[2], symbols);
    void* arg2 = getArg(program->instr[4], symbols);

    switch (program->instr[2]->type) {
        case STRVAR:
        case STRCON:
            if (strcmp(arg1, arg2) > 0) {
                return TRUE;
            } else {
                findElseJump(program);
                return FALSE;
            }
            break;
        case NUMVAR:
        case NUMCON:
            if (*(double*)arg1 > *(double*)arg2) {
                return TRUE;
            } else {
                findElseJump(program);
                return FALSE;
            }
            break;
        default:
            ON_ERROR("Something went wrong\n");
            break;
    }
}

void* getArg(token_t* token, symbol_t* symbols) {
    mvmcell* cell;
    /* FIXME this is soo fucking bodgy right now */
    static double num;

    switch (token->type) {
        case NUMVAR:
        case STRVAR:
            cell = getVariable(symbols, token->attrib);
            if (!cell) {
                printf("%s ", token->attrib);
                ON_ERROR("Variable not yet initialised\n");
            }
            return cell->data;
            break;
        case NUMCON:
            num = strtod(token->attrib, NULL);
            return &num;
            break;
        case STRCON:
            return token->attrib;
            break;
        default:
            ON_ERROR("Something went wrong\n");
            break;
    }
}

void inter_inc(prog_t* program, symbol_t* symbols) {
    mvmcell* cell = getVariable(symbols, program->instr[2]->attrib);
    if (!cell) {
        printf("%s ", program->instr[2]->attrib);
        fprintf(stderr, "Variable not initialised\n");
        exit(EXIT_FAILURE);
    }

    *(double*)cell->data += 1;
}

void inter_set(prog_t* program, symbol_t* symbols) {
    mvmcell* cell;
    char* str;
    double* num;

    addVariable(symbols, program->instr[0]->attrib);

    switch (program->instr[2]->type) {
        case STRCON:
            str = allocString(strlen(program->instr[2]->attrib));
            strcpy(str, program->instr[2]->attrib);
            updateVariable(symbols, program->instr[0]->attrib, str);
            break;
        case STRVAR:
            cell = getVariable(symbols, program->instr[2]->attrib);
            if (!cell) {
                printf("%s ", program->instr[2]->attrib);
                ON_ERROR("Variable not initialised\n");
            }
            str = allocString(strlen((char*)cell->data));
            strcpy(str, (char*)cell->data);
            updateVariable(symbols, program->instr[0]->attrib, str);
            break;
        case NUMCON:
            num = allocNumber();
            *num = strtod(program->instr[2]->attrib, NULL);
            updateVariable(symbols, program->instr[0]->attrib, num);
            break;
        case NUMVAR:
            cell = getVariable(symbols, program->instr[2]->attrib);
            if (!cell) {
                printf("%s ", program->instr[2]->attrib);
                ON_ERROR("Variable not initialised\n");
            }
            num = allocNumber();
            *num = *(double*)cell->data;
            updateVariable(symbols, program->instr[0]->attrib, num);
            break;
        default:
            ON_ERROR("Something went wrong\n");
    }
}

/* FIXME don't forget to call seed */
void inter_rndSeed(void) {
    srand(time(NULL));
}

double* allocNumber(void) {
    double* tmp = (double*)malloc(sizeof(double));
    if (!tmp) {
        ON_ERROR("Memory allocation error\n");
    }
    return tmp;
}

char* allocString(size_t len) {
    char* tmp = (char*)malloc(sizeof(char) * (len + 1));
    if (!tmp) {
        ON_ERROR("Memory allocation error\n");
    }
    return tmp;
}
