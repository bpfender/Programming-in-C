#ifndef SYMBOLS_H
#define SYMBOLS_H

#include "mvmedit.h"

typedef struct ast_t ast_t;

typedef struct symbol_t {
    mvm* vars;
    mvm* nums;
    mvm* files;
} symbol_t;

symbol_t* initSymbolTable(void);

mvmcell* getVariable(symbol_t* symbols, char* var);
void addVariable(symbol_t* symbols, char* var);
mvmcell* updateVariable(symbol_t* symbols, char* var, void* val);

mvmcell* getFilename(symbol_t* symbols, char* filename);
void addFilename(symbol_t* symbols, char* filename, void* prog);

void freeSymbolTable(symbol_t* symbols);

#endif
