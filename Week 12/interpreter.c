#include "interpreter.h"
#include <stdlib.h>
#include <time.h>

#define RND_RANGE 100

void inter_rndSeed(void){
    srand(time(NULL));
}

void inter_file(void) {}

void inter_abort(void) {
    printf("Program ended\n");
    exit(EXIT_SUCCESS);
}

/* FIXME not sure how best to get input from string */
void inter_in2str(mvmcell* arg1, mvmcell* arg2) {
    char* buffer = NULL;
    line_t size;
    getLine(&buffer, &size, stdin);
}

void inter_innum(void) {
}

void inter_jump(void) {}

void inter_print(type_t type, mvmcell* arg) {
    /*FIXME should this be functionised */
    printf("%s", arg->data);

    if (type == PRINTN) {
        printf("\n");
    }
}

void inter_rnd(mvmcell* arg) {
    int rnd = rand();
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
}

void inter_set(mvmcell* arg1, void* arg2) {}
