#include "fmvm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define AVE_CHARS 5 /*FIXME could actually functionise this now */
#define MULTI_SEARCH_LIST 2
#define FACTOR 2
#define PRNT_STR_CHARS "[]() "

#define HASH_SIZE 500
#define HASH_FACTOR 2
/* djb2 Hash constants, defined based on reference below
 * http://www.cse.yorku.ca/~oz/hash.html
 */
#define DJB2_HASH 5381
#define DJB2_MAGIC 33

unsigned long djb2Hash(char* s);
bucket_t* buildBucket(char* key);
mvmcell* insertKey(mvm* m, char* key);
bucket_t* findKey(mvm* m, char* key);
bucket_t* buildBucket(char* key);
void swapBuckets(bucket_t** b1, bucket_t** b2);
bucket_t* initHashTable(int size);
mvmcell* mvmcell_init(size_t data_len);
char* initListBuffer(size_t size);
void expandListBuffer(char** buffer, size_t size);

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
    mvmcell* node;
    bucket_t* cell = insertKey(m, key);

    node = mvmcell_init(strlen(data) + 1);
    strcpy(node->data, data);

    /* Update linked list */
    node->next = cell->head;
    cell->head = node;

    m->num_keys++;
}

mvmcell* insertKey(mvm* m, char* key) {
    bucket_t* table = m->hash_table;
    bucket_t* bucket = buildBucket(key);
    unsigned long index = bucket->hash % m->table_size;
    bucket_t* location = NULL;

    if (location = findKey(m, key)) {
        return location;
    }
    if (!table[index].key) {
        return &table[index];
    }

    /* FIXME could this be neatened up a little bit with a function call */
    /* Probably needs a preincrement of offset */
    while (table[index + bucket->distance].key) {
        if (bucket->distance > table[index + bucket->distance].distance) {
            if (!location) {
                location = table + index + bucket->distance;
            }
            swapBuckets(bucket, table + index + bucket->distance);
        }
        bucket->distance++;
    }
}

bucket_t* findKey(mvm* m, char* key) {
    unsigned long index = djb2Hash(key) % m->table_size;
    int offset = 0;

    if (!strcmp(m->hash_table[index].key, key)) {
        return m->hash_table + index;
    }

    /* This can be combined into one loop with the above */
    while (!(m->hash_table[index + offset].distance < offset || !m->hash_table[index + offset].key)) {
        if (!strcmp(m->hash_table[index + offset].key, key)) {
            return m->hash_table + index + offset;
        }
        offset++;
    }
    return NULL;
}

bucket_t* buildBucket(char* key) {
    bucket_t* tmp = (bucket_t*)malloc(sizeof(bucket_t));
    if (!tmp) {
        ON_ERROR("Error allocating memory for bucket\n");
    }

    tmp->key = (char*)malloc(sizeof(char) * (strlen(key) + 1));
    if (!tmp->key) {
        ON_ERROR("Error allocating memory for key\n");
    }
    strcpy(tmp->key, key);
    tmp->hash = djb2Hash(key);
    tmp->head = NULL;
    tmp->distance = 0;

    return tmp;
}

void swapBuckets(bucket_t** b1, bucket_t** b2) {
    bucket_t* tmp = *b1;
    *b1 = *b2;
    *b2 = tmp;
}

char* mvm_print(mvm* m) {
    size_t i;
    mvmcell* node;
    size_t buffer_size = AVE_CHARS * m->num_keys;
    size_t curr_index = 0, next_index = 0;

    char* buffer = initListBuffer(buffer_size);

    char** list = (char**)malloc(sizeof(char*) * MULTI_SEARCH_LIST);
    if (!list) {
        ON_ERROR("Error allocating multi-search list\n");
    }

    for (i = 0; i < m->table_size; i++) {
        if (m->hash_table[i].key) {
            node = m->hash_table[i].head;
            while (node) {
                next_index += strlen(m->hash_table[i].key) + strlen(node->data) + strlen(PRNT_STR_CHARS);

                /* Check with "+ 1" to ensure there is space for NUll terminator if this
         * is the final appended string */
                if (next_index + 1 >= buffer_size) {
                    buffer_size = next_index * FACTOR;
                    expandListBuffer(&buffer, buffer_size);
                }

                sprintf(buffer + curr_index, "[%s](%s) ", m->hash_table[i].key, node->data);

                curr_index = next_index;
                node = node->next;
            }
        }
    }
    return buffer;
}

void mvm_delete(mvm* m, char* key) {
    bucket_t* bucket = findKey(m, key);
    int offset = 0;

    mvmcell* node = bucket->head;
    bucket->head = node->next;

    free(node->data);
    free(node);

    if (bucket->head == NULL) {
        while ()
    }

    m->num_keys--;
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
    unsigned long hash = DJB2_HASH;
    for (i = 0; s[i] != '\0'; i++) {
        hash += hash * DJB2_MAGIC ^ (unsigned long)s[i];
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