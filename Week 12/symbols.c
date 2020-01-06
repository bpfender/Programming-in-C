#include "symbols.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* FIXME might be nice to write an implementation that can actually store numbers */

symbol_t* initSymbolTable(void) {
    symbol_t* tmp = (symbol_t*)malloc(sizeof(symbol_t));
    if (!tmp) {
        ON_ERROR("Error allocating symbol table\n");
    }

    tmp->vars = mvm_init();
    tmp->files = mvm_init();
    return tmp;
}

mvmcell* getVariable(symbol_t* symbols, char* var) {
    return mvm_search(symbols->vars, var);
}

mvmcell* updateVariable(symbol_t* symbols, char* var, char* val) {
    mvmcell* cell = mvm_search(symbols->vars, var);
    if (cell) {
        /* QUESTION Ok to call free on nulled data? */
        free(cell->data);
        cell->data = (char*)malloc(sizeof(char) * (strlen(val) + 1));
        if (!cell->data) {
            ON_ERROR("Error allocating var\n");
        }
        return cell;
    }
    return NULL;
}

/* FIXME This is not pretty at the moment */
void addVariable(symbol_t* symbols, char* var) {
    if (!mvm_search(symbols->vars, var)) {
        mvm_insert(symbols->vars, var, "NULL");
    }
}

/* FIXME filename handling is not fantastic */
mvmcell* getFilename(symbol_t* symbols, char* filename) {
    return mvm_search(symbols->files, filename);
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
