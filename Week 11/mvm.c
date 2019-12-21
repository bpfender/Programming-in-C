#include "mvm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* TODO consider generic malloc handler */
/* FIXME check types used */

/* TODO REDEFINE TO MORE SENSIBLE SIZE */
#define AVE_CHARS 1
#define FACTOR 2
#define FORMAT_LEN strlen("[]() ")
#define MULTI_SEARCH_LIST 5

/* ------ HELPER FUNCTION DECLARATIONS ------ */
/* These functions do not need to be exposed to the user */
mvmcell* mvm_findKey(mvmcell* head, char* key);
mvmcell* mvmcell_build(char* key, char* data);
mvmcell* mvmcell_deleteHelper(mvmcell* node, char* key, mvm* m);
void mvmcell_unloadNode(mvmcell* node);
void mvmcell_unloadList(mvmcell* node);
void* allocHandler(void* ptr, size_t nmemb, size_t size);

/* ------- FUNCTION BODIES ------ */
/* Initialises mvm ADT with calloc to make sure everything is zeroed
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
void mvm_insert(mvm* m, char* key, char* data) {
    if (m && key && data) {
        mvmcell* node = mvmcell_build(key, data);

        node->next = m->head;
        m->head = node;
        m->numkeys++;
    }
}

/* Writes string into buffer that is dynamically resized as required. Would have
 * liked to use strncat() for easier buffer resizing but I believe this is a GNU
 * only extension 
 */
char* mvm_print(mvm* m) {
    if (m) {
        /* FIXME more elegant initial sizing? */
        size_t buffer_size = AVE_CHARS * m->numkeys;
        char* buffer = (char*)allocHandler(NULL, buffer_size, sizeof(char));

        mvmcell* node = m->head;
        size_t curr_index = 0;
        size_t next_index = 0;

        /* Loop first checks length of string to be appended to ensure it can 
        fit the buffer. If required, buffer is expanded and string is then added
        to the buffer. Loop continues to last node in linked list */
        while (node) {
            next_index += strlen(node->key) + strlen(node->data) + FORMAT_LEN;

            /* Check with "+ 1" to ensure there is space for NUll terminator if 
            this is the final appended string. Resize buffer if required */
            if (next_index + 1 >= buffer_size) {
                buffer_size = next_index * FACTOR;
                buffer = (char*)allocHandler(buffer, buffer_size, sizeof(char));
            }

            sprintf(buffer + curr_index, "[%s](%s) ", node->key, node->data);

            curr_index = next_index;
            node = node->next;
        }

        return buffer;
    }
    return NULL;
}

/* This is currently defined to delete only one key at a time in order to pass
 * test cases in "testmvm.c". Recursive helper "deleteHelper" could easily be
 * modified to delete all occurences of a key. Does nothing on invalid input
 */
void mvm_delete(mvm* m, char* key) {
    if (key && m) {
        m->head = mvmcell_deleteHelper(m->head, key, m);
    }
}

/* A little unclear from the spec if this is supposed to copy. Based on the fact
 * the testing code doesn't free this, I assumed that it is just supposed to
 * point to the data stored in the MVM. Returns NULL if the key is not found
 */
char* mvm_search(mvm* m, char* key) {
    if (m && key) {
        mvmcell* node = mvm_findKey(m->head, key);
        return node ? node->data : NULL;
    }
    return NULL;
}

char** mvm_multisearch(mvm* m, char* key, int* n) {
    if (m && key && n) {
        size_t size = MULTI_SEARCH_LIST;
        char** list = (char**)allocHandler(NULL, size, sizeof(char*));

        mvmcell* node = m->head;
        size_t i = 0;

        while ((node = mvm_findKey(node, key))) {
            if (i >= size) {
                size *= FACTOR;
                list = (char**)allocHandler(list, size, sizeof(char*));
            }

            list[i] = node->data;
            i++;

            /* Move to next node so mvm_findKey() doesn't return same node */
            node = node->next;
        }

        *n = i;
        return list;
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
/* Allocates memory for mvmcell and populates values.
 */
mvmcell* mvmcell_build(char* key, char* data) {
    mvmcell* node = (mvmcell*)allocHandler(NULL, 1, sizeof(mvmcell));
    node->key = (char*)allocHandler(NULL, strlen(key) + 1, sizeof(char));
    node->data = (char*)allocHandler(NULL, strlen(data) + 1, sizeof(char));

    strcpy(node->key, key);
    strcpy(node->data, data);
    /* FIXME should next be NULLed to avoid any issues? */

    return node;
}

/* Returns pointer to mvmcell node where key is found. If the key doesn't exist
 * returns NULL. Pointing to the node, allows continue searching in the
 * multi-search function
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

/* Recursive helper function to delete element from linked list. Currently just
 * deletes the first item found to conform to testing in "testmvm.c". Function
 * returns address of next node to previous call, in turn removing the specified
 * node 
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
    free(node->data);
    free(node->key);
    free(node);
}

/* Wrote a generic malloc/realloc function because the same structure was
 * repeating itself multiple times and I wanted to play with void*. Requires
 * ptr = NULL for initial malloc, and ptr value for resizing
 */
void* allocHandler(void* ptr, size_t nmemb, size_t size) {
    void* tmp = realloc(ptr, nmemb * size);
    if (!tmp) {
        ON_ERROR("Memory allocation error\n");
    }
    return tmp;
}
