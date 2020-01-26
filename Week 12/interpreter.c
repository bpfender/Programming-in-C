#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "error.h"
#include "interpreter.h"
#include "parser.h"

/* Error handling for the interpreter is done in this file rather than expanding
 * the extension file needlessly. Given that the interpreter will quit as soon
 * as an error is encountered, it seemed pointless to call extra functions that
 * would just call exit() straight away
 */

#define RND_MOD 100

#define INPUT_LEN 256

/* -------- INTERPRETER FUNCTIONS ------- */

void inter_abort(void) {
    printf("\nPROGRAM END: Program has ended\n");
    exit(EXIT_SUCCESS);
}

/* Input checking on this is not currently fantastic. But for the scope of this
 * it will do. Would have like to do a dynamic buffer but got held up on other
 * things. However, my getLine() function demonstrates the concept 
 */
/* FIXME use strtok instead */
void inter_in2str(prog_t* program, symbol_t* symbols) {
    char line[INPUT_LEN];
    char word1[INPUT_LEN];
    char word2[INPUT_LEN];

    char *w1, *w2;

    if (fgets(line, sizeof(line), stdin)) {
        if (sscanf(line, " %s %s ", word1, word2) == 2) {
            if (strlen(word1) > MAX_INPUT_LEN || strlen(word2) > MAX_INPUT_LEN) {
                err_interInput(program, ERROR);
            }
            w1 = allocString(strlen(word1));
            w2 = allocString(strlen(word2));

            strcpy(w1, word1);
            strcpy(w2, word2);

            updateVariable(symbols, program->instr[BRKT_ARG1]->attrib, w1);
            updateVariable(symbols, program->instr[BRKT_ARG2]->attrib, w2);
        }
    } else {
        err_interInput(program, IN2STR);
    }
}

void inter_innum(prog_t* program, symbol_t* symbols) {
    char line[INPUT_LEN];

    double* num = allocNumber();

    if (fgets(line, sizeof(line), stdin)) {
        if (sscanf(line, "%lf", num) == 1) {
            if (strlen(line) > MAX_INPUT_LEN) {
                err_interInput(program, ERROR);
            }
            updateVariable(symbols, program->instr[BRKT_ARG1]->attrib, num);
        } else {
            err_interInput(program, INNUM);
        }
    }
}

bool_t inter_ifequal(prog_t* program, symbol_t* symbols) {
    double tmp_num;

    void* arg1 = getArg(program->instr[BRKT_ARG1], symbols, &tmp_num);
    void* arg2 = getArg(program->instr[BRKT_ARG2], symbols, &tmp_num);

    if (!arg1) {
        err_interVAR(program, BRKT_ARG1);
    }
    if (!arg2) {
        err_interVAR(program, BRKT_ARG2);
    }

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
            err_interUndef(program);
            exit(EXIT_FAILURE);
            break;
    }
}

bool_t inter_ifgreater(prog_t* program, symbol_t* symbols) {
    /* Temporary number variable required in case of NUMCON. getArg() updates
    this by converting the attribute string and writing into tmp_num */
    double tmp_num;

    void* arg1 = getArg(program->instr[BRKT_ARG1], symbols, &tmp_num);
    void* arg2 = getArg(program->instr[BRKT_ARG2], symbols, &tmp_num);

    if (!arg1) {
        err_interVAR(program, BRKT_ARG1);
    }
    if (!arg2) {
        err_interVAR(program, BRKT_ARG2);
    }

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
            err_interUndef(program);
            exit(EXIT_FAILURE);
            break;
    }
}

void inter_inc(prog_t* program, symbol_t* symbols) {
    mvmcell* cell = getVariable(symbols, program->instr[BRKT_ARG1]->attrib);
    if (!cell) {
        err_interVAR(program, BRKT_ARG1);
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
        err_printLocation(program->instr[ARG_INDEX], program->filename);
        ON_ERROR("INTERPRETER: Expected an integer value for jump");
    }

    pos = atoi(program->instr[ARG_INDEX]->attrib);

    if (!checkValidJump(program, pos)) {
        err_printLocation(program->instr[ARG_INDEX], program->filename);
        ON_ERROR("INTERPRETER: JUMP does not go to valid INSTRUCT in program\n");
    }
    program->pos = pos;
}

void inter_print(prog_t* program, symbol_t* symbols) {
    mvmcell* cell;

    switch (program->instr[1]->type) {
        case NUMVAR:
            cell = getVariable(symbols, program->instr[ARG_INDEX]->attrib);
            if (!cell) {
                err_interVAR(program, ARG_INDEX);
            }

            printf("%f", *(double*)cell->data);
            break;
        case STRVAR:
            cell = getVariable(symbols, program->instr[ARG_INDEX]->attrib);
            if (!cell) {
                err_interVAR(program, ARG_INDEX);
            }

            printf("%s ", (char*)cell->data);
            break;
        case NUMCON:
        case STRCON:
            printf("%s ", program->instr[ARG_INDEX]->attrib);
            break;
        default:
            err_interUndef(program);
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
                err_interVAR(program, SET_VAL);
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
                err_interVAR(program, SET_VAL);
            }

            num = allocNumber();
            *num = *(double*)cell->data;

            updateVariable(symbols, program->instr[SET_VAR]->attrib, num);
            break;
        default:
            err_interUndef(program);
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
        err_interCond(program);
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
