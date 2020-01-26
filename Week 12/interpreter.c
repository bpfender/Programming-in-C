
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "error.h"
#include "interpreter.h"
#include "parser.h"

#define RND_MOD 100

#define INPUT_LEN 256

/* -------- INTERPRETER FUNCTIONS ------- */

void inter_abort(void) {
    printf("\nPROGRAM END: Program has ended\n");
    exit(EXIT_SUCCESS);
}

/* FIXME this input is not fab */
void inter_in2str(prog_t* program, symbol_t* symbols) {
    char line[INPUT_LEN];
    char word1[INPUT_LEN];
    char word2[INPUT_LEN];

    char *w1, *w2;

    if (fgets(line, sizeof(line), stdin)) {
        if (sscanf(line, " %s %s ", word1, word2) == 2) {
            w1 = allocString(strlen(word1));
            w2 = allocString(strlen(word2));

            strcpy(w1, word1);
            strcpy(w2, word2);
        } else {
            ON_ERROR("INTERPRETER: Expected two input strings\n");
        }
    }

    updateVariable(symbols, program->instr[BRKT_ARG1]->attrib, w1);
    updateVariable(symbols, program->instr[BRKT_ARG2]->attrib, w2);
}

void inter_innum(prog_t* program, symbol_t* symbols) {
    char line[INPUT_LEN];

    double* num = allocNumber();

    if (fgets(line, sizeof(line), stdin)) {
        if (sscanf(line, "%lf", num) == 1) {
        } else {
            ON_ERROR("INTERPRETER: Expected single number input\n");
        }
    }

    updateVariable(symbols, program->instr[BRKT_ARG1]->attrib, num);
}

bool_t inter_ifequal(prog_t* program, symbol_t* symbols) {
    void* arg1 = getArg(program->instr[BRKT_ARG1], symbols);
    void* arg2 = getArg(program->instr[BRKT_ARG2], symbols);

    switch (program->instr[BRKT_ARG1]->type) {
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
            /* Admittedly a bit naughty but double can express integers int the same 
            range as an int type (or) more so for the purposes here this comparison
            of floating point numbers is good enough */
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

bool_t inter_ifgreater(prog_t* program, symbol_t* symbols) {
    /* Temporary number variable required in case of NUMCON. getArg() updates
    this by converting the attribute string and writing into tmp_num */
    double tmp_num;

    void* arg1 = getArg(program->instr[BRKT_ARG1], symbols, &tmp_num);
    void* arg2 = getArg(program->instr[BRKT_ARG2], symbols, &tmp_num);

    switch (program->instr[BRKT_ARG1]->type) {
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

void inter_inc(prog_t* program, symbol_t* symbols) {
    mvmcell* cell = getVariable(symbols, program->instr[BRKT_ARG1]->attrib);
    if (!cell) {
        printf("%s ", program->instr[BRKT_ARG1]->attrib);
        fprintf(stderr, "Variable not initialised\n");
        exit(EXIT_FAILURE);
    }

    *(double*)cell->data += 1;
}

void inter_rnd(prog_t* program, symbol_t* symbols) {
    double* rnd = allocNumber();
    *rnd = rand() % RND_MOD;

    updateVariable(symbols, program->instr[BRKT_ARG1]->attrib, rnd);
}

void inter_jump(prog_t* program) {
    int pos;
    if (!checkJumpValue(program->instr[ARG_INDEX]->attrib)) {
        ON_ERROR("INTERPRETER: Expected an integer value for jump");
    }

    pos = atoi(program->instr[ARG_INDEX]->attrib);

    if (!checkValidJump(program, pos)) {
        ON_ERROR("INTERPRETER: JUMP does not go to valid point in program\n");
    }
    program->pos = pos;
}

void inter_print(prog_t* program, symbol_t* symbols) {
    mvmcell* cell;

    switch (program->instr[1]->type) {
        case NUMVAR:
            cell = getVariable(symbols, program->instr[ARG_INDEX]->attrib);
            if (!cell) {
                printf("%s ", program->instr[ARG_INDEX]->attrib);
                ON_ERROR("Variable has not been initialised\n");
            }
            printf("%f", *(double*)cell->data);
            break;
        case STRVAR:
            cell = getVariable(symbols, program->instr[ARG_INDEX]->attrib);
            if (!cell) {
                printf("%s ", program->instr[ARG_INDEX]->attrib);
                ON_ERROR("Variable has not been initialised\n");
            }
            printf("%s ", (char*)cell->data);
            break;
        case NUMCON:
        case STRCON:
            printf("%s ", program->instr[ARG_INDEX]->attrib);
            break;
        default:
            ON_ERROR("Something went wrong\n");
    }

    if (program->instr[0]->type == PRINTN) {
        printf("\n");
    }
}

void inter_set(prog_t* program, symbol_t* symbols) {
    mvmcell* cell;
    char* str;
    double* num;

    switch (program->instr[SET_VAL]->type) {
        case STRCON:
            str = allocString(strlen(program->instr[SET_VAL]->attrib));
            strcpy(str, program->instr[SET_VAL]->attrib);
            updateVariable(symbols, program->instr[SET_VAR]->attrib, str);
            break;
        case STRVAR:
            cell = getVariable(symbols, program->instr[SET_VAL]->attrib);
            if (!cell) {
                printf("%s ", program->instr[SET_VAL]->attrib);
                ON_ERROR("Variable not initialised\n");
            }
            str = allocString(strlen((char*)cell->data));
            strcpy(str, (char*)cell->data);
            updateVariable(symbols, program->instr[SET_VAR]->attrib, str);
            break;
        case NUMCON:
            num = allocNumber();
            *num = strtod(program->instr[SET_VAL]->attrib, NULL);
            updateVariable(symbols, program->instr[SET_VAR]->attrib, num);
            break;
        case NUMVAR:
            cell = getVariable(symbols, program->instr[SET_VAL]->attrib);
            if (!cell) {
                printf("%s ", program->instr[SET_VAL]->attrib);
                ON_ERROR("Variable not initialised\n");
            }
            num = allocNumber();
            *num = *(double*)cell->data;
            updateVariable(symbols, program->instr[SET_VAR]->attrib, num);
            break;
        default:
            ON_ERROR("Something went wrong\n");
    }
}

/* ------ HELPER FUNCTIONS ------ */

void* getArg(token_t* token, symbol_t* symbols, double* num) {
    mvmcell* cell;

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
            /* For a NUMCON, the value is converted and then written to the num
            pointer. The pointer to this is then returned as the argument */
            *num = strtod(token->attrib, NULL);
            return num;
            break;
        case STRCON:
            return token->attrib;
            break;
        default:
            ON_ERROR("Something went wrong\n");
            break;
    }
}

void findElseJump(prog_t* program) {
    int nest = 1;
    token_t* token = nextToken(program);

    if (strcmp(token->attrib, "{")) {
        ON_ERROR("If else statement error\n");
    }

    while (strcmp(token->attrib, "}") || nest) {
        /*FIXME extra error handling */
        token = nextToken(program);
        if (!strcmp(token->attrib, "}")) {
            nest--;
        }
        if (!strcmp(token->attrib, "{")) {
            nest++;
        }
    }
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

bool_t checkValidJump(prog_t* program, int pos) {
    if (pos >= program->len) {
        return FALSE;
    }

    if (program->token[pos].word != 0) {
        return FALSE;
    }
    return TRUE;
}

/* ------- UTILITY FUNCTIONS -------- */

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

void inter_rndSeed(void) {
    srand(time(NULL));
}
