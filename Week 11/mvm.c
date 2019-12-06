#include "mvm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFF_SIZE 50
#define BRACKET_CHARS 2
#define MULTI_SEARCH_LIST 4

/* ------ HELPER FUNCTION DECLARATIONS ------ */
/* These functions do not need to be exposed to the user */
mvmcell* mvm_findKey(mvm* m, char* key);
mvmcell* mvmcell_init(size_t key_len, size_t data_len);
void expandListBuffer(char** buffer, size_t size);
char* initListBuffer(size_t size);

/* ------- FUNCTION BODIES ------ */
mvm* mvm_init(void) {
    mvm* tmp = (mvm*)calloc(1, sizeof(mvm));
    if (!tmp) {
        ON_ERROR("Error allocating Multi-Value Map\n");
    }
    return tmp;
}

int mvm_size(mvm* m) {
    return m->numkeys;
}

void mvm_insert(mvm* m, char* key, char* data) {
    mvmcell* node = mvmcell_init(strlen(key) + 1, strlen(data) + 1);

    strcpy(node->key, key);
    strcpy(node->data, data);
    node->next = m->head;

    m->head = node;
}

char* initListBuffer(size_t size) {
    /* FIXME is there a more clever way to initialise the buff size? */
    char* tmp = (char*)malloc(sizeof(char) * size);
    if (!tmp) {
        ON_ERROR("Error allocating print buffer\n");
    }
    return tmp;
}

void expandListBuffer(char** buffer, size_t size) {
    char* tmp = (char*)realloc(*buffer, size * sizeof(char));
    if (!tmp) {
        ON_ERROR("Error reallocating print buffer\n");
    }
    *buffer = tmp;
}

/* TODO not particularly happy with this at the moment */
char* mvm_print(mvm* m) {
    /* IS there a more clever way to initialise the buffer size */
    size_t buffer_size = BUFF_SIZE;
    char* buffer = initListBuffer(buffer_size);

    mvmcell* node = m->head;
    int index = 0;
    int increment;
    char* tmp;

    while (node) {
        increment = strlen(node->key) + strlen(node->data) + 2 * BRACKET_CHARS;

        /* FIXME check this resizing routine and error checking */
        if (index + increment > buffer_size) {
            buffer_size = index - buffer_size + buffer_size * 2;
            expandListBuffer(&buffer, buffer_size);
        }

        sprintf(buffer + index, "[%s](%s) ", node->key, node->key);
        index += increment;
        node = node->next;
    }
    return buffer;
}

void mvm_delete(mvm* m, char* key) {
    mvmcell* node = m->head;

    while (node) {
    }
}

char* mvm_search(mvm* m, char* key) {
    mvmcell* node = mvm_findKey(m, key);

    return node ? node->data : NULL;
}

/* return null pointer for robustness to mark end of list */
char** mvm_multisearch(mvm* m, char* key, int* n) {
    char** tmp;
    char** list = (char**)malloc(sizeof(char*) * MULTI_SEARCH_LIST);
    if (!list) {
        ON_ERROR("Error allocating multi-search list\n");
    }
    int size = MULTI_SEARCH_LIST;
    int index = 0;
    mvmcell* node = m->head;

    while (node) {
        if (!strcmp(node->key, key)) {
            strcpy(*(list + index), node->data);
            index++;
            if (index >= size) {
            }
        }
        node = node->next;
    }
}

/* QUESTION is it better to do these things recursively or not? In my mind
 * it's better to do this as a loop so you don't end up filling up the stack?
 */
void mvm_free(mvm** p) {
    mvm* m = *p;
    mvmcell* curr;
    mvmcell* next = m->head;

    while (next) {
        curr = next;
        next = curr->next;
        free(curr->key);
        free(curr->data);
        free(curr);
    }

    free(m);
    *p = NULL;
}

/* ------ HELPER FUNCTIONS ------ */

mvmcell* mvmcell_init(size_t key_len, size_t data_len) {
    mvmcell* node = (mvmcell*)malloc(sizeof(mvmcell));
    if (!node) {
        ON_ERROR("Error allocating cell\n");
    }

    node->key = (char*)malloc(sizeof(char) * key_len);
    node->data = (char*)malloc(sizeof(char) * data_len);
    /* FIXME check this condition */
    if (!node->key && !node->data) {
        ON_ERROR("Error allocating cell data\n");
    }
    return node;
}

mvmcell* mvm_findKey(mvm* m, char* key) {
    mvmcell* node = m->head;

    while (node) {
        if (!strcmp(node->key, key)) {
            return node;
        }
        node = node->next;
    }
    return NULL;
}

/* TODO is it worth using this function to cut down things a bit? */
void* mallocHandler(size_t nmemb, size_t size) {
    void* tmp = malloc(size * nmemb);
    if (!tmp) {
        ON_ERROR("Memory allocation error\n");
    }
    return tmp;
}