#include "interpreter.h"
#include <stdlib.h>

void inter_abort(prog_t* program, symbol_t* symbols) {
    printf("Program ended\n");
    exit(EXIT_SUCCESS);
}

void inter_in2str(prog_t* program, symbol_t* symbols) {
}

void inter_innum(prog_t* program, symbol_t* symbols) {}

void inter_jump(prog_t* program, symbol_t* symbols) {}

void inter_print(prog_t* program, symbol_t* symbols) {
}

void inter_ifcond(prog_t* program, symbol_t* symbols) {}

void inter_inc(prog_t* program, symbol_t* symbols) {}

void inter_set(prog_t* program, symbol_t* symbols) {}
