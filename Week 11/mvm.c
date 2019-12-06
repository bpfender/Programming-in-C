#include "mvm.h"
#include <stdio.h>
#include <stdlib.h>

mvm* mvm_init(void) {
    mvm* tmp = (mvm*)calloc(1, sizeof(mvm));
    if (!tmp) {
        ON_ERROR("Error allocating Multi-Value Map");
    }
}

int mvm_size(mvm* m) {
    return m->numkeys;
}

void mvm_insert(mvm* m, char* key, char* data) {
}

char* mvm_print(mvm* m) {
}

void mvm_delete(mvm* m, char* key) {
}

char* mvm_search(mvm* m, char* key) {
}

char** mvm_multisearch(mvm* m, char* key, int* n) {
}

void mvm_free(mvm** p) {
}