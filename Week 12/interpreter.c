#include "interpreter.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define RND_RANGE 100

void inter_rndSeed(void) {
    srand(time(NULL));
}

void inter_file(void) {
}

void inter_abort(void) {
    printf("Program ended\n");
    exit(EXIT_SUCCESS);
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

/* FIXME not doing any handling on buffer lengths currently */
void inter_in2str(mvmcell* arg1, mvmcell* arg2) {
    char line[256];
    char word1[256];
    char word2[256];
    char *w1, *w2;

    if (fgets(line, sizeof(line), stdin)) {
        if (sscanf(line, "%s %s", word1, word2) == 2) {
            w1 = allocString(strlen(word1));
            w2 = allocString(strlen(word2));

            printf("%s %s\n", word1, word2);

            strcpy(w1, word1);
            strcpy(w2, word2);

            printf("%s %s\n", w1, w2);
        } else {
            ON_ERROR("Input error\n");
        }
    }

    arg1->data = w1;
    arg2->data = w2;
}

void inter_innum(mvmcell* arg) {
    char line[256];

    double* num = allocNumber();

    if (fgets(line, sizeof(line), stdin)) {
        if (sscanf(line, "%lf", num) == 1) {
            printf("%lf\n", *num);
        } else {
            ON_ERROR("Input error\n");
        }
    }

    arg->data = num;
}

void inter_jump(void) {}

void inter_print(type_t type, type_t arg_type, void* arg) {
    /*FIXME should this be functionised */

    switch (arg_type) {
        case NUMVAR:
            printf("%lf", ((mvmcell*)arg)->data);
            break;
        case STRVAR:
            printf("%s", ((mvmcell*)arg)->data);
            break;
        case NUMCON:
        case STRCON:
            printf("%s", arg);
            break;
    }

    if (type == PRINTN) {
        printf("\n");
    }
}

/* FIXME ints vs doubles? */
void inter_rnd(mvmcell* arg) {
    double* rnd = allocNumber();
    *rnd = (double)99 / (rand() % RND_RANGE);

    arg->data = rnd;
}

bool_t inter_ifequal(mvmcell* arg1, mvmcell* arg2) {
    if (strcmp(arg1->data, arg2->data)) {
        return FALSE;
    }
    return TRUE;
}

/* Not quite sure about number detection here */
bool_t inter_ifgreater(mvmcell* arg1, mvmcell* arg2) {
    if (arg1->key[0] == '%') {
        if (atof(arg1) > atof(arg2)) {
            return TRUE;
        }
        return FALSE;
    } else {
        if (strcmp(arg1, arg2) > 0) {
            return TRUE;
        }
        return FALSE;
    }
}

void inter_inc(mvmcell* arg) {
    *((double*)arg->data) += 1;
}

void inter_set(mvmcell* arg1, void* arg2, type_t type1, type_t type2) {
    if (type1 == NUMVAR) {
        if (type2 == NUMVAR) {
            *((double*)arg1->data) = *((double*)(((mvmcell*)arg2)->data));
        } else {
            *((double*)arg1->data) = *(double*)arg2;
        }
    } else {
        if (type2 == STRVAR) {
        } else {
        }
    }
}
