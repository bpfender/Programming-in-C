#include "fmvm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define AVE_CHARS 6 /*FIXME could actually functionise this now */
#define MULTI_SEARCH_LIST 2
#define FACTOR 2
#define PRNT_STR_CHARS "[]() "

/* FIXME no resizing of hash table implemented yet */
#define HASH_SIZE 6
#define HASH_FACTOR 2
/* djb2 Hash constants, defined based on reference below
 * http://www.cse.yorku.ca/~oz/hash.html
 */
#define DJB2_HASH 5381
#define DJB2_MAGIC 33

unsigned long djb2Hash(char* s);
bucket_t* buildBucket(char* key);
bucket_t* insertKey(mvm* m, char* key);
bucket_t* findKey(mvm* m, char* key);
bucket_t* buildBucket(char* key);
void swapBuckets(bucket_t* b1, bucket_t* b2);
bucket_t* initHashTable(int size);
mvmcell* mvmcell_init(size_t data_len);
char* initListBuffer(size_t size);
void expandListBuffer(char** buffer, size_t size);
void removeKey(mvm* m, char* key);
void clearBucket(bucket_t* bucket);
void mvmcell_unloadList(mvmcell* node);
void mvmcell_unloadNode(mvmcell* node);

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
    bucket_t* cell = insertKey(m, key);

    /* FIXME declare this as a whole function */
    mvmcell* node = mvmcell_init(strlen(data) + 1);
    strcpy(node->data, data);

    /* Update linked list */
    node->next = cell->head;
    cell->head = node;

    m->num_keys++;
}

/* FIXME need to include a resize table in here */
bucket_t* insertKey(mvm* m, char* key) {
    bucket_t* table = m->hash_table;
    bucket_t* bucket = buildBucket(key);
    unsigned long index = bucket->hash % m->table_size;

    bucket_t* location = NULL;
    unsigned long offset_index;

    printf("HASH INDEX: %li\n", index);

    if (!table[index].key) {
        memcpy(table + index, bucket, sizeof(bucket_t));
        printf("RETURNED INDEX: %li\n", index);
        return table + index;
    }

    if ((location = findKey(m, key))) {
        return location;
    }

    /* FIXME could this be neatened up a little bit with a function call */
    do {
        bucket->distance++;
        offset_index = (index + bucket->distance) % m->table_size;

        if (bucket->distance > table[offset_index].distance) {
            if (!location) {
                location = table + offset_index;
            }
            swapBuckets(bucket, table + offset_index);
        }

    } while (table[offset_index].key);
    printf("RETURNED INDEX: %li\n", offset_index);
    return location;
}

bucket_t* findKey(mvm* m, char* key) {
    /* FIXME duplication of hash calc above */
    unsigned long index = djb2Hash(key) % m->table_size;
    int offset = 0;
    size_t offset_index = index;

    if (!(m->hash_table[index].key)) {
        return NULL;
    }

    if (!strcmp(m->hash_table[index].key, key)) {
        return m->hash_table + index;
    }

    /* This can be combined into one loop with the above */
    while (!(m->hash_table[offset_index].distance < offset || !m->hash_table[offset_index].key)) {
        if (!strcmp(m->hash_table[offset_index].key, key)) {
            return m->hash_table + offset_index;
        }
        offset++;
        offset_index = (index + offset) % m->table_size;
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

/* TODO is this a good implementation, no other alternative really */
void swapBuckets(bucket_t* b1, bucket_t* b2) {
    bucket_t tmp;
    memcpy(&tmp, b1, sizeof(bucket_t));
    memcpy(b1, b2, sizeof(bucket_t));
    memcpy(b2, &tmp, sizeof(bucket_t));
}

char* mvm_print(mvm* m) {
    int i;
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

    mvmcell* node = bucket->head;
    bucket->head = node->next;

    free(node->data);
    free(node);

    if (bucket->head == NULL) {
        removeKey(bucket);
    }

    m->num_keys--;
}

bucket_t* removeKey(mvm* m, char* key) {
    bucket_t* bucket = findKey(m, key);

    if (!bucket) {
        size_t i = 0;
        size_t index;

        do {
            i++;
            index = i % m->table_size;
        } while (!(bucket[index].distance == 0 || bucket[index].key == NULL));

        if (i < m->table_size - 1) {
            memcpy(bucket, bucket + 1, i * sizeof(bucket));
        } else {
            memcpy(bucket + m->table_size - 1, m->hash_table);
                }

        clearBucket(bucket + i);
        return NULL
    }
    return bucket;
}

/* FIXME does everything actually need to be zeroed? */
void clearBucket(bucket_t* bucket) {
    bucket->key = NULL;
    bucket->distance = 0;
    bucket->hash = 0;
    bucket->key = NULL;
}

char* mvm_search(mvm* m, char* key) {
    bucket_t* bucket = findKey(m, key);
    if (bucket) {
        return bucket->head->data;
    }
    return NULL;
}

char** mvm_multisearch(mvm* m, char* key, int* n) {
    int size = MULTI_SEARCH_LIST;
    int curr_index = 0;

    bucket_t* bucket = findKey(m, key);
    mvmcell* node = bucket->head;

    char** list = (char**)malloc(sizeof(char*) * MULTI_SEARCH_LIST);
    if (!list) {
        ON_ERROR("Error allocating multi-search list\n");
    }

    while (node) {
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

void mvm_free(mvm** p) {
    int i;
    mvm* m = *p;

    for (i = 0; i < m->table_size; i++) {
        if (m->hash_table[i].key) {
            mvmcell_unloadList(m->hash_table[i].head);
        }
    }
}

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
    free(node);
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
