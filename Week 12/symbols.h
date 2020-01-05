#ifndef SYMBOLS_H
#define SYMBOLS_H

#include "../Week 11/mvm.h"

typedef struct symbol_t {
    mvm* vars;
    mvm* files;
} symbol_t;

symbol_t* initSymbolTable(void);
char* getVariable(symbol_t* symbols, char* var);
void deleteVariable(symbol_t* symbols, char* var);
void addVariable(symbol_t* symbols, char* var, char* val);
char* getFilename(symbol_t* symbols, char* filename);
void deleteFilename(symbol_t* symbols, char* filename);
void addFilename(symbol_t* symbols, char* filename);
void freeSymbolTable(symbol_t* symbols);

#endif
