#ifndef SYMBOLS_H
#define SYMBOLS_H

#include <stdlib.h>

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

/* Checks whether variable already exists and updates value if it does. If not
 * add the variable to the symbol table */
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
/* Initialises mvm ADT and zeros all values
 */
mvm* mvm_init(void);

/* Insert element at head of linked list stored in mvm ADT. Does nothing when
 * given an invalid input 
 */
void mvm_insert(mvm* m, char* key, void* data);

/* Returns pointer to cell where key is stored or NULL if it doesn't exist
 */
mvmcell* mvm_search(mvm* m, char* key);

/* Free & set p to NULL 
 */
void mvm_free(mvm** p);

/* ------ HELPER FUNCTION DECLARATIONS ------ */
/* Returns pointer to mvmcell node where key is found. If the key doesn't exist
 * returns NULL.
 */
mvmcell* mvm_findKey(mvmcell* head, char* key);

/* Allocates memory for mvmcell that will be added to LL and populates values.
 * next ptr doesn't have to be zeroed because elements are always added to head
 * of mvm list, which is initialised with NULL. Data is expected as an already
 * malloced piece of data.
 */
mvmcell* mvmcell_init(char* key, void* data);

/* Recursive helper function to delete element from linked list. Currently just
 * deletes the first item found to conform to testing in "testmvm.c". Function
 * returns address of next node to previous call, in turn removing the specified
 * node. 
 */
mvmcell* mvmcell_deleteHelper(mvmcell* node, char* key, mvm* m);

/* Recursive function to unload linked list. Still not completely sure if it's
 * better to do this recursively or with a loop?
 */
void mvmcell_unloadList(mvmcell* node);

/* Unload node helper that frees all parts of the mvmcell LL node
 */
void mvmcell_unloadNode(mvmcell* node);

/* Wrote a generic malloc/realloc function because the same structure was
 * repeating itself multiple times and I wanted to play with void*. Requires
 * ptr = NULL for initial malloc, and ptr value for resizing an existing block
 */
void* allocHandler(void* ptr, size_t nmemb, size_t size);

#endif
