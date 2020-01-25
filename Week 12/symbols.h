#ifndef SYMBOLS_H
#define SYMBOLS_H

#include <stdlib.h>

#define ON_ERROR(STR)     \
    fprintf(stderr, STR); \
    exit(EXIT_FAILURE)

typedef struct mvmcell {
    char* key;
    void* data;
    struct mvmcell* next;
} mvmcell;

typedef struct mvm {
    mvmcell* head;
    int numkeys;
} mvm;

typedef struct symbol_t {
    mvm* vars;
    mvm* files;
} symbol_t;


/* ------- SYMBOL TABLE HANDLING ------ */
symbol_t* initSymbolTable(void);
void freeSymbolTable(symbol_t* symbols);

/* ------- VARIABLE HANDLING ------ */
/* The key trick here is that mvm has been modified to take a void* as the data
 * value. This requires a preinitialised pointer to be passed in updateVariable()
 * but allows strings or numbers to be stored. These are then cast, depending on
 * context to the correct type when being interpreted
 */

/* Return NULL if variable is not in symbol table, or pointer to cell if it is
 */
mvmcell* getVariable(symbol_t* symbols, char* var);

/* Adds key value to symbol table */
void addVariable(symbol_t* symbols, char* var);
void updateVariable(symbol_t* symbols, char* var, void* val);

/* ------ FILENAME HANDLING ------ */
/* Returns NULL if filename is not found in files map. Actual data value stored
 * has no effect here. Was kept in case of possible extension for multipass
 * parsing/semantics/interpreting but didn't have time to implement 
 */
mvmcell* getFilename(symbol_t* symbols, char* filename);

/* Add filename as key to map. As above, data value doesn't currently matter
 */
void addFilename(symbol_t* symbols, char* filename, void* prog);


/* ------- MVM UPDATED FUNCTIONS ------- */
mvm* mvm_init(void);
/* Number of key/value pairs stored */
int mvm_size(mvm* m);
/* Insert one key/value pair */
void mvm_insert(mvm* m, char* key, void* data);
/* Remove one key/value */
void mvm_delete(mvm* m, char* key);
/* Return the corresponding value for a key */
mvmcell* mvm_search(mvm* m, char* key);
/* Free & set p to NULL */
void mvm_free(mvm** p);

char* mvm_print(mvm* m);

/* ------ HELPER FUNCTION DECLARATIONS ------ */
/* These functions do not need to be exposed to the user */
mvmcell* mvm_findKey(mvmcell* head, char* key);
mvmcell* mvmcell_init(char* key, void* data);
mvmcell* mvmcell_deleteHelper(mvmcell* node, char* key, mvm* m);
void mvmcell_unloadList(mvmcell* node);
void mvmcell_unloadNode(mvmcell* node);
void* allocHandler(void* ptr, size_t nmemb, size_t size);

#endif
