#include "mvm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* FIXME check types used */

/* TODO REDEFINE TO MORE SENSIBLE SIZE */
#define AVE_CHARS 10
#define BUFF_FACT 2
#define PRNT_STR_CHARS "()[] "
#define MULTI_SEARCH_LIST 2

/* ------ HELPER FUNCTION DECLARATIONS ------ */
/* These functions do not need to be exposed to the user */
mvmcell* mvm_findKey(mvm* m, char* key);
mvmcell* mvmcell_init(size_t key_len, size_t data_len);
void expandListBuffer(char** buffer, size_t size);
char* initListBuffer(size_t size);
mvmcell* mvmcell_deleteHelper(mvmcell* node, char* key, mvm* m);
void unloadNode(mvmcell* node);
void unloadList(mvmcell* node);

/* ------- FUNCTION BODIES ------ */
/* Initialises mvm ADT with calloc to make sure everything is zeroed
 */
mvm* mvm_init(void) {
    mvm* tmp = (mvm*)calloc(1, sizeof(mvm));
    if (!tmp) {
        ON_ERROR("Error allocating Multi-Value Map\n");
    }
    return tmp;
}

/* Returns 0 if m is null, or the "numkeys" value stored in m 
 */
int mvm_size(mvm* m) {
    return m ? m->numkeys : 0;
}

/* Insert element at head of linked list stored in mvm ADT. Does nothing when
 * given an invalid input */
void mvm_insert(mvm* m, char* key, char* data) {
    mvmcell* node;

    /* Check that no NULL values have been passed as arguments. If valid
       build the node, other wise just return and do nothing */
    if (m && key && data) {
        /* Build node */
        /* QUESTION does it make more sense to have +1 here or in init? */
        node = mvmcell_init(strlen(key) + 1, strlen(data) + 1);
        strcpy(node->key, key);
        strcpy(node->data, data);

        /* Update linked list */
        node->next = m->head;
        m->head = node;
        m->numkeys++;
    }
}

/* Writes string into buffer that is dynamically resized as required. Would have
 * liked to use strncat() for easier buffer resizing but I believe this is only
 * GNU extension 
 */
char* mvm_print(mvm* m) {
    /* TODO is there a more steamlined method for this initialisation ? */
    size_t buffer_size = AVE_CHARS * m->numkeys;
    char* buffer = initListBuffer(buffer_size);

    size_t curr_index = 0, next_index = 0;

    mvmcell* node = m->head;

    /* Loop first checks length of string to be appended to ensure it can fit
       the buffer. If required, buffer is expanded and string is then added to
       the buffer */
    while (node) {
        next_index += strlen(node->key) + strlen(node->data) + strlen(PRNT_STR_CHARS);

        /* Check with "+ 1" to ensure there is space for NUll terminator if this
         * is the final appended string */
        if (next_index + 1 >= buffer_size) {
            buffer_size = next_index * BUFF_FACT;
            expandListBuffer(&buffer, buffer_size);
        }

        sprintf(buffer + curr_index, "[%s](%s) ", node->key, node->data);

        curr_index = next_index;
        node = node->next;
    }

    return buffer;
}

/* This is currently defined to delete only one key at a time in order to past
 * test cases in "testmvm.c". Recursive helper "deleteHelper" could easily be
 * modified to delete all occurences of a key
 */
void mvm_delete(mvm* m, char* key) {
    /* Check for valid input before doing anything */
    if (key && m) {
        m->head = mvmcell_deleteHelper(m->head, key, m);
    }
}

/* FIXME unclear if this should return a pointer to origninal data or a copy */
char* mvm_search(mvm* m, char* key) {
    mvmcell* node = mvm_findKey(m, key);
    return node ? node->data : NULL;
}

/* return null pointer for robustness to mark end of list */
char** mvm_multisearch(mvm* m, char* key, int* n) {
    int size = MULTI_SEARCH_LIST;
    int curr_index = 0;

    mvmcell* node = m->head;

    char** list = (char**)malloc(sizeof(char*) * MULTI_SEARCH_LIST);
    if (!list) {
        ON_ERROR("Error allocating multi-search list\n");
    }

    while (node) {
        if (!strcmp(node->key, key)) {
            *(list + curr_index) = node->data;
            curr_index++;
            if (curr_index >= size) {
                /* TODO reallocing list array */
            }
        }
        node = node->next;
    }
    *n = curr_index;
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

    if (!(node->key && node->data)) {
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

/* FIXME this could probably just be concotenated into expand buffer */
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

/* Recursive helper function to delete element from linked list. Currently just
 * deletes the first item found to conform to testing in "testmvm.c". Function
 * returns address of next node to previous call, in turn removing the specified
 * node 
 */
/* FIXME could deleted be passed as part of mvm* */
mvmcell* mvmcell_deleteHelper(mvmcell* node, char* key, mvm* m) {
    mvmcell* tmp;

    if (node == NULL) {
        return NULL;
    } else if (!strcmp(node->key, key)) {
        tmp = node->next;
        unloadNode(node);
        m->numkeys--;
        return tmp;
    } else {
        node->next = mvmcell_deleteHelper(node->next, key, m);
        return node;
    }
}

void unloadNode(mvmcell* node) {
    free(node->data);
    free(node->key);
    free(node);
}

/* TODO is it worth using this function to cut down things a bit? */
void* mallocHandler(size_t nmemb, size_t size) {
    void* tmp = malloc(size * nmemb);
    if (!tmp) {
        ON_ERROR("Memory allocation error\n");
    }
    return tmp;
}
