

#include "symbols.h"
#include <stdio.h>
#include <stdlib.h>

symbol_t* initSymbolTable(void) {
    symbol_t* tmp = (symbol_t*)malloc(sizeof(symbol_t));
    if (!tmp) {
        ON_ERROR("Error allocating symbol table\n");
    }

    tmp->vars = mvm_init();
    tmp->files = mvm_init();
    return tmp;
}

char* getVariable(symbol_t* symbols, char* var) {
    return mvm_search(symbols->vars, var);
}

void deleteVariable(symbol_t* symbols, char* var) {
    mvm_delete(symbols->vars, var);
}

/* FIXME This is not pretty at the moment */
void addVariable(symbol_t* symbols, char* var, char* val) {
    deleteVariable(symbols, var);

    mvm_insert(symbols->vars, var, val);
}

/* FIXME filename handling is not fantastic */
char* getFilename(symbol_t* symbols, char* filename) {
    return mvm_search(symbols->files, filename);
}

void deleteFilename(symbol_t* symbols, char* filename) {
    mvm_delete(symbols->files, filename);
}

/* This is not pretty at the moment */
void addFilename(symbol_t* symbols, char* filename) {
    mvm_insert(symbols->files, filename, "OPEN");
}

void freeSymbolTable(symbol_t* symbols) {
    mvm_free(&symbols->files);
    mvm_free(&symbols->vars);
    free(symbols);
}

