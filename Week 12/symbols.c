#include <stdio.h>
#include <string.h>

#include "symbols.h"

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

void updateVariable(symbol_t* symbols, char* var, void* val) {
    mvmcell* cell = mvm_search(symbols->vars, var);
    if (cell) {
        if (cell->data) {
            free(cell->data);
        }
        cell->data = val;
    } else {
        mvm_insert(symbols->vars, var, val);
    }
}

/* ------- FILENAME HANDLING ------- */
mvmcell* getFilename(symbol_t* symbols, char* filename) {
    return mvm_search(symbols->files, filename);
}

/* This was defined like this in the hopes of doing multi-pass lexical and 
 * syntactic analysis. Void* was supposed to point to beginning of program queue, but
 * it doesn't actually matter what the value is for current implementation
*/
void addFilename(symbol_t* symbols, char* filename, void* prog) {
    mvm_insert(symbols->files, filename, prog);
}

/* ------- MVM UPDATE FUNCTIONS ------ */
mvm* mvm_init(void) {
    mvm* tmp = (mvm*)allocHandler(NULL, 1, sizeof(mvm));
    tmp->head = NULL;
    tmp->numkeys = 0;

    return tmp;
}

void mvm_insert(mvm* m, char* key, void* data) {
    if (m && key) {
        mvmcell* node = mvmcell_init(key, data);

        node->next = m->head;
        m->head = node;
        m->numkeys++;
    }
}

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

mvmcell* mvmcell_init(char* key, void* data) {
    mvmcell* node = (mvmcell*)allocHandler(NULL, 1, sizeof(mvmcell));

    node->key = (char*)allocHandler(NULL, strlen(key) + 1, sizeof(char));
    strcpy(node->key, key);

    node->data = data;

    return node;
}


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

void mvmcell_unloadList(mvmcell* node) {
    if (!node) {
        return;
    }
    mvmcell_unloadList(node->next);
    mvmcell_unloadNode(node);
}

void mvmcell_unloadNode(mvmcell* node) {
    if (node->data) {
        free(node->data);
    }
    free(node->key);
    free(node);
}

void* allocHandler(void* ptr, size_t nmemb, size_t size) {
    void* tmp = realloc(ptr, nmemb * size);
    if (!tmp) {
        ON_ERROR("Memory allocation error\n");
    }
    return tmp;
}

