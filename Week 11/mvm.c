#include "mvm.h"
#include <stdio.h>
#include <stdlib.h>

#define AVE_KEY_LENGTH 5

mvm* mvm_init(void) {
    mvm* tmp = (mvm*)calloc(1, sizeof(mvm));
    if (!tmp) {
        ON_ERROR("Error allocating Multi-Value Map\n");
    }
}

int mvm_size(mvm* m) {
    return m->numkeys;
}

void mvm_insert(mvm* m, char* key, char* data) {
    mvmcell* tmp = (mvmcell*)malloc(sizeof(mvmcell));
    if (!tmp) {
        ON_ERROR("Error allocating cell\n");
    }
    strcpy(tmp->key, key);
    strcpy(tmp->data, data);
    tmp->next = m->head;

    m->head = tmp;
}

char* mvm_print(mvm* m) {
    mvmcell* node = m->head;
    char* buff = (char*)malloc(m->numkeys * AVE_KEY_LENGTH);
    unsigned int buff_size = AVE_KEY_LENGTH;
    if (!buff) {
        ON_ERROR("Error allocating print buffer");
    }

    while (node) {
                node = node->next;
    }
}

void mvm_delete(mvm* m, char* key) {
}

char* mvm_search(mvm* m, char* key) {
}

/* return null pointer for robustness to mark end of list */
char** mvm_multisearch(mvm* m, char* key, int* n) {
}

void mvm_free(mvm** p) {
}