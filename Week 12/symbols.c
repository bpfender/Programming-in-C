#include "symbols.h"
#include <stdio.h>
#include <string.h>

/* Average word length in english is 4.7 chars. With mvm_print() returning 2 
strings this can just be rounded to 10 */
#define AVE_CHARS 10
#define FRMT_CHARS strlen("[]() ")

#define PRNT_STR_LEN AVE_CHARS + FRMT_CHARS
#define MULTI_SEARCH_LEN 5
/* Universal resizing factor */
#define FACTOR 2

/* ------ SYMBOL TABLE HANDLING ------ */
symbol_t* initSymbolTable(void) {
    symbol_t* tmp = (symbol_t*)malloc(sizeof(symbol_t));
    if (!tmp) {
        ON_ERROR("Error allocating symbol table\n");
    }

    tmp->vars = mvm_init();
    tmp->files = mvm_init();
    return tmp;
}

void freeSymbolTable(symbol_t* symbols) {
    mvm_free(&symbols->files);
    mvm_free(&symbols->vars);
    free(symbols);
}

/* ------- VARIABLE HANDLING ------ */
mvmcell* getVariable(symbol_t* symbols, char* var) {
    return mvm_search(symbols->vars, var);
}

/* FIXME this needs to be modified */
void updateVariable(symbol_t* symbols, char* var, void* val) {
    mvmcell* cell = mvm_search(symbols->vars, var);
    if (cell) {
        /* QUESTION Ok to call free on nulled data? */
        if (cell->data) {
            free(cell->data);
        }
        cell->data = val;
    } else {
        mvm_insert(symbols->vars, var, val);
    }
}

/* FIXME This is not pretty at the moment */
void addVariable(symbol_t* symbols, char* var) {
    if (!mvm_search(symbols->vars, var)) {
        mvm_insert(symbols->vars, var, NULL);
    }
}

/* ------- FILENAME HANDLING ------- */
mvmcell* getFilename(symbol_t* symbols, char* filename) {
    return mvm_search(symbols->files, filename);
}

void addFilename(symbol_t* symbols, char* filename, void* prog) {
    mvm_insert(symbols->files, filename, prog);
}

/* ------- MVM UPDATE FUNCTIONS ------ */
/* Initialises mvm ADT and zeros all values
 */
mvm* mvm_init(void) {
    mvm* tmp = (mvm*)allocHandler(NULL, 1, sizeof(mvm));
    tmp->head = NULL;
    tmp->numkeys = 0;

    return tmp;
}

/* Returns 0 if m is null, or the "numkeys" value stored in m 
 */
int mvm_size(mvm* m) {
    return m ? m->numkeys : 0;
}

/* Insert element at head of linked list stored in mvm ADT. Does nothing when
 * given an invalid input */
void mvm_insert(mvm* m, char* key, void* data) {
    if (m && key) {
        mvmcell* node = mvmcell_init(key, data);

        node->next = m->head;
        m->head = node;
        m->numkeys++;
    }
}

/* Returns pointer to cell where key is stored
 */
mvmcell* mvm_search(mvm* m, char* key) {
    if (m && key) {
        return mvm_findKey(m->head, key);
    }
    return NULL;
}

void mvm_free(mvm** p) {
    mvm* m = *p;

    mvmcell_unloadList(m->head);
    free(m);
    *p = NULL;
}

/* ------ HELPER FUNCTIONS ------ */
/* Returns pointer to mvmcell node where key is found. If the key doesn't exist
 * returns NULL.
 */
mvmcell* mvm_findKey(mvmcell* head, char* key) {
    mvmcell* node = head;

    while (node) {
        if (!strcmp(node->key, key)) {
            return node;
        }
        node = node->next;
    }
    return NULL;
}

/* Allocates memory for mvmcell that will be added to LL and populates values.
 * next ptr doesn't have to be zeroed because elements are always added to head
 * of mvm list, which is initialised with NULL. Data is expected as an already
 * malloced piece of data.
 */
mvmcell* mvmcell_init(char* key, void* data) {
    mvmcell* node = (mvmcell*)allocHandler(NULL, 1, sizeof(mvmcell));

    node->key = (char*)allocHandler(NULL, strlen(key) + 1, sizeof(char));
    strcpy(node->key, key);

    node->data = data;

    return node;
}

/* Recursive helper function to delete element from linked list. Currently just
 * deletes the first item found to conform to testing in "testmvm.c". Function
 * returns address of next node to previous call, in turn removing the specified
 * node. 
 */
mvmcell* mvmcell_deleteHelper(mvmcell* node, char* key, mvm* m) {
    mvmcell* tmp;

    if (!node) {
        return NULL;
    } else if (!strcmp(node->key, key)) {
        tmp = node->next;
        mvmcell_unloadNode(node);
        m->numkeys--;
        return tmp;
    } else {
        node->next = mvmcell_deleteHelper(node->next, key, m);
        return node;
    }
}

/* Recursive function to unload linked list. Still not completely sure if it's
 * better to do this recursively or with a loop?
 */
void mvmcell_unloadList(mvmcell* node) {
    if (!node) {
        return;
    }
    mvmcell_unloadList(node->next);
    mvmcell_unloadNode(node);
}

/* Unload node helper that frees all parts of the mvmcell LL node
 */
void mvmcell_unloadNode(mvmcell* node) {
    if (node->data) {
        free(node->data);
    }
    free(node->key);
    free(node);
}

/* Wrote a generic malloc/realloc function because the same structure was
 * repeating itself multiple times and I wanted to play with void*. Requires
 * ptr = NULL for initial malloc, and ptr value for resizing an existing block
 */
void* allocHandler(void* ptr, size_t nmemb, size_t size) {
    void* tmp = realloc(ptr, nmemb * size);
    if (!tmp) {
        ON_ERROR("Memory allocation error\n");
    }
    return tmp;
}

/* Writes string into buffer that is dynamically resized as required. Would have
 * liked to use strncat() for easier buffer resizing but I believe this is a GNU
 * only extension. Buffer is initially sized based on average word lengths and
 * ()[] string, though string lengths will of course vary depending on the data
 * stored. 
 */
char* mvm_print(mvm* m) {
    if (m) {
        size_t buffer_size = PRNT_STR_LEN * m->numkeys;
        char* buffer = (char*)allocHandler(NULL, buffer_size, sizeof(char));

        mvmcell* node = m->head;
        size_t curr_index = 0;
        size_t next_index = 0;

        /* Loop first checks length of string to be appended to ensure it can 
        fit the buffer. If required, buffer is expanded and string is then added
        to the buffer. Loop continues to last node in linked list */
        while (node) {
            next_index += strlen(node->key) + 3;

            /* Check with "+ 1" to ensure there is space for NUll terminator if 
            this is the final appended string. Resize buffer if required */
            if (next_index + 1 >= buffer_size) {
                buffer_size = next_index * FACTOR;
                buffer = (char*)allocHandler(buffer, buffer_size, sizeof(char));
            }

            sprintf(buffer + curr_index, "[%s] ", node->key);
            curr_index = next_index;
            node = node->next;
        }
        return buffer;
    }
    return NULL;
}
