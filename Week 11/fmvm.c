#include "fmvm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HASH_SIZE 500
#define HASH_FACTOR 2
/* djb2 Hash constants, defined based on reference below
 * http://www.cse.yorku.ca/~oz/hash.html
 */
#define HASH 5381
#define MAGIC 33

unsigned long djb2Hash(char* s);

mvm* mvm_init(void) {
    mvm* tmp = (mvm*)calloc(1, sizeof(mvm));
    if (!tmp) {
        ON_ERROR("Error allocating memory for MVM\n");
    }

    tmp->hash_table = initHashTable(HASH_SIZE);
    tmp->table_size = HASH_SIZE;

    return tmp;
}

int mvm_size(mvm* m) {
    return m ? m->num_keys : 0;
}

void mvm_insert(mvm* m, char* key, char* data) {
    mvmcell* cell = findHashTableCell(m, key);
}

/* FIXME perhaps try and remove all this dereferencing */
mvmcell* findHashTableCell(mvm* m, char* key) {
    size_t i = 0;
    int offset;
    unsigned long hash = djb2Hash(key);
    unsigned long index = hash % m->table_size;

    if (!m->hash_table[index].key) {
        m->hash_table[index].key = initKey(strlen(key) + 1);
        strcpy(m->hash_table[index].key, key);

        m->hash_table[index].distance = 0;
        m->hash_table[index].hash = hash;
    } else {
        for (offset = 0; offset > m->hash_table[index + offset].distance; offset++) {
        }
    }
}

char* initKey(size_t size) {
    char* tmp = (char*)malloc(sizeof(char) * size);
    if (!tmp) {
        ON_ERROR("Error allocating space for key\n");
    }
    return tmp;
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

/* ------ HELPER FUNCTIONS ------ */
/* Interface does not need to be exposed to the user */

/* FIXME use murmur instead */
/* http://www.cse.yorku.ca/~oz/hash.html */
unsigned long djb2Hash(char* s) {
    size_t i;
    unsigned long hash = HASH;
    for (i = 0; s[i] != '\0'; i++) {
        hash += hash * MAGIC ^ (unsigned long)s[i];
    }

    return hash;
}

bucket_t* initHashTable(int size) {
    bucket_t* tmp = (bucket_t*)calloc(size, sizeof(bucket_t));
    if (!tmp) {
        ON_ERROR("Error allocating hash table\n");
    }

    return tmp;
}

mvmcell* mvmcell_init(size_t data_len) {
    mvmcell* node = (mvmcell*)malloc(sizeof(mvmcell));
    if (!node) {
        ON_ERROR("Error allocating cell\n");
    }

    node->data = (char*)malloc(sizeof(char) * data_len);

    if (!node->data) {
        ON_ERROR("Error allocating cell data\n");
    }
    return node;
}