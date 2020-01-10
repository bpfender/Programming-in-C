#ifndef SYMBOLS_H
#define SYMBOLS_H

#include "mvmedit.h"

typedef struct ast_t ast_t;

typedef struct symbol_t {
    mvm* vars;
    mvm* files;
} symbol_t;

symbol_t* initSymbolTable(void);

mvmcell* getVariable(symbol_t* symbols, char* var);
void addVariable(symbol_t* symbols, char* var);

mvmcell* getFilename(symbol_t* symbols, char* filename);
void addFilename(symbol_t* symbols, char* filename, ast_t* ast);

void freeSymbolTable(symbol_t* symbols);

#endif
