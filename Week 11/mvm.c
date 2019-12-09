#include "mvm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* FIXME check types used */
/* TODO check error handling on NULL inputs, can this be cleaned up? */
/* TODO generic malloc function? */

/* TODO REDEFINE TO MORE SENSIBLE SIZE */
#define AVE_CHARS 10
#define FACTOR 2
#define PRNT_STR_CHARS "()[] "
#define MULTI_SEARCH_LIST 5

/* ------ HELPER FUNCTION DECLARATIONS ------ */
/* These functions do not need to be exposed to the user */
mvmcell* mvm_findKey(mvmcell* head, char* key);
mvmcell* mvmcell_init(size_t key_len, size_t data_len);
void expandListBuffer(char** buffer, size_t size);
char* initListBuffer(size_t size);
mvmcell* mvmcell_deleteHelper(mvmcell* node, char* key, mvm* m);
void mvmcell_unloadNode(mvmcell* node);
void mvmcell_unloadList(mvmcell* node);

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
    if (m) {
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
                buffer_size = next_index * FACTOR;
                expandListBuffer(&buffer, buffer_size);
            }

            sprintf(buffer + curr_index, "[%s](%s) ", node->key, node->data);

            curr_index = next_index;
            node = node->next;
        }

        return buffer;
    }
    return NULL;
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

/* A little unclear from the spec if this is supposed o copy. Based on the fact
 * the testing doesn't free this, indicated that it is just supposed to be a
 * pointed to the data stores in the MVM. Returns NULL if the key is not found
 */
char* mvm_search(mvm* m, char* key) {
    mvmcell* node = mvm_findKey(m->head, key);

    return node ? node->data : NULL;
}

/* return null pointer for robustness to mark end of list */
char** mvm_multisearch(mvm* m, char* key, int* n) {
    /* Sense check on input */
    if (m && key && n) {
        int size = MULTI_SEARCH_LIST;
        int curr_index = 0;

        mvmcell* node = m->head;

        char** list = (char**)malloc(sizeof(char*) * MULTI_SEARCH_LIST);
        if (!list) {
            ON_ERROR("Error allocating multi-search list\n");
        }

        while ((node = mvm_findKey(node, key))) {
            *(list + curr_index) = node->data;
            curr_index++;

            if (curr_index >= size) {
                size *= FACTOR;
                list = (char**)realloc(list, sizeof(char*) * size);
            }

            /* Move to next node so mvm_findKey() doesn't search same node
               again */
            node = node->next;
        }

        *n = curr_index;
        return list;
    }

    return NULL;
}

/* QUESTION is it better to do these things recursively or not? 
 */
void mvm_free(mvm** p) {
    mvm* m = *p;

    mvmcell_unloadList(m->head);
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

    if (!(node->key && node->data)) {
        ON_ERROR("Error allocating cell data\n");
    }
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
mvmcell* mvmcell_deleteHelper(mvmcell* node, char* key, mvm* m) {
    mvmcell* tmp;

    if (node == NULL) {
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

/* Recursive function to unload linked list
 */
void mvmcell_unloadList(mvmcell* node) {
    if (node == NULL) {
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

/* TODO is it worth using this function to cut down things a bit? */
void* mallocHandler(size_t nmemb, size_t size) {
    void* tmp = malloc(size * nmemb);
    if (!tmp) {
        ON_ERROR("Memory allocation error\n");
    }
    return tmp;
}
