#ifndef SYMBOLS_H
#define SYMBOLS_H

#include "ast.h"
#include "mvmedit.h"

typedef struct symbol_t {
    mvm* vars;
    mvm* files;
} symbol_t;

symbol_t* initSymbolTable(void);
mvmcell* getVariable(symbol_t* symbols, char* var);
void addVariable(symbol_t* symbols, char* var);
mvmcell* getFilename(symbol_t* symbols, char* filename);
void addFilename(symbol_t* symbols, char* filename);

void freeSymbolTable(symbol_t* symbols);

#endif
