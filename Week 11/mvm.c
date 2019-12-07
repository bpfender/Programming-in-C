#include "mvm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* TODO REDEFINE TO MORE SENSIBLE SIZE */
#define BUFF_SIZE 5
#define BRACKET_CHARS 2
#define SPACE 1
#define MULTI_SEARCH_LIST 2

/* ------ HELPER FUNCTION DECLARATIONS ------ */
/* These functions do not need to be exposed to the user */
mvmcell* mvm_findKey(mvm* m, char* key);
mvmcell* mvmcell_init(size_t key_len, size_t data_len);
void expandListBuffer(char** buffer, size_t size);
char* initListBuffer(size_t size);
mvmcell* mvmcell_deleteHelper(mvmcell* node, char* key, int* deleted);
void unloadNode(mvmcell* node);
void unloadList(mvmcell* node);

/* ------- FUNCTION BODIES ------ */
mvm* mvm_init(void) {
    mvm* tmp = (mvm*)calloc(1, sizeof(mvm));
    if (!tmp) {
        ON_ERROR("Error allocating Multi-Value Map\n");
    }
    return tmp;
}

int mvm_size(mvm* m) {
    return m ? m->numkeys : 0;
}

void mvm_insert(mvm* m, char* key, char* data) {
    mvmcell* node;

    /* FIXME error checking is not pretty */
    if (!(m && key && data)) {
        return;
    }

    node = mvmcell_init(strlen(key) + 1, strlen(data) + 1);

    strcpy(node->key, key);
    strcpy(node->data, data);
    node->next = m->head;

    m->head = node;
    m->numkeys++;
}

/* TODO not particularly happy with this at the moment */
char* mvm_print(mvm* m) {
    /* IS there a more clever way to initialise the buffer size */
    size_t buffer_size = BUFF_SIZE;
    char* buffer = initListBuffer(buffer_size);

    mvmcell* node = m->head;
    size_t index = 0;
    size_t increment;

    while (node) {
        increment = strlen(node->key) + strlen(node->data) + 2 * BRACKET_CHARS + SPACE;

        /* FIXME check this resizing routine and error checking */
        if (index + increment >= buffer_size) {
            buffer_size = (index + increment) * 2;
            expandListBuffer(&buffer, buffer_size);
        }

        sprintf(buffer + index, "[%s](%s) ", node->key, node->data);
        index += increment;
        node = node->next;
    }
    printf("%s\n", buffer);
    return buffer;
}

void mvm_delete(mvm* m, char* key) {
    int deleted = 0;

    if (!(key && m)) {
        return;
    }

    m->head = mvmcell_deleteHelper(m->head, key, &deleted);
    m->numkeys -= deleted;
}

char* mvm_search(mvm* m, char* key) {
    mvmcell* node = mvm_findKey(m, key);
    return node ? node->data : NULL;
}

/* return null pointer for robustness to mark end of list */
char** mvm_multisearch(mvm* m, char* key, int* n) {
    int size = MULTI_SEARCH_LIST;
    int index = 0;

    mvmcell* node = m->head;

    char** list = (char**)malloc(sizeof(char*) * MULTI_SEARCH_LIST);
    if (!list) {
        ON_ERROR("Error allocating multi-search list\n");
    }

    while (node) {
        if (!strcmp(node->key, key)) {
            *(list + index) = node->data;
            index++;
            if (index >= size) {
                /* TODO reallocing list array */
            }
        }
        node = node->next;
    }
    *n = index;
    return list;
}

/* QUESTION is it better to do these things recursively or not? 
 */
void mvm_free(mvm** p) {
    mvm* m = *p;

    unloadList(m->head);
    free(m);
    *p = NULL;
}

void unloadList(mvmcell* node) {
    if (node == NULL) {
        return;
    }
    unloadList(node->next);
    unloadNode(node);
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

char* initListBuffer(size_t size) {
    /* FIXME is there a more clever way to initialise the buff size? */
    char* tmp = (char*)malloc(sizeof(char) * size);
    if (!tmp) {
        ON_ERROR("Error allocating print buffer\n");
    }
    return tmp;
}

void expandListBuffer(char** buffer, size_t size) {
    char* tmp = (char*)realloc(*buffer, sizeof(char) * size);
    if (!tmp) {
        ON_ERROR("Error reallocating print buffer\n");
    }
    *buffer = tmp;
}

/* FIXME is this the best solution */
/* THis just deleted the first item found to conform to testing in test fikle */
/* FIXME could deleted be passed as part of mvm* */
mvmcell* mvmcell_deleteHelper(mvmcell* node, char* key, int* deleted) {
    mvmcell* tmp;

    if (node == NULL) {
        return NULL;
    } else if (!strcmp(node->key, key)) {
        tmp = node->next;
        unloadNode(node);
        (*deleted)++;
        return tmp;
    } else {
        node->next = mvmcell_deleteHelper(node->next, key, deleted);
        return node;
    }
}

void unloadNode(mvmcell* node) {
    free(node->data);
    free(node->key);
    free(node);
}
