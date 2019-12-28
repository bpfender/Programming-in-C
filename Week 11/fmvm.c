#include "fmvm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define AVE_CHARS 6 /*FIXME could actually functionise this now */
#define MULTI_SEARCH_LIST 2
#define FACTOR 2
#define FORMAT_LEN strlen("[]() ")

/* FIXME no resizing of hash table implemented yet */

/* djb2 Hash constants, defined based on reference below
 * http://www.cse.yorku.ca/~oz/hash.html
 */
#define DJB2_HASH 5381
#define DJB2_MAGIC 33

#define LIST_LEN 10

mvm* mvm_init(void) {
    return mvm_initHelper(HASH_SIZE);
}

mvm* mvm_initHelper(size_t size) {
    mvm* tmp = (mvm*)malloc(sizeof(mvm));
    if (!tmp) {
        ON_ERROR("Error allocating memory for MVM\n");
    }

    tmp->hash_table = initHashTable(size);
    tmp->table_size = size;
    tmp->num_keys = 0;
    tmp->num_buckets = 0;

    return tmp;
}

int mvm_size(mvm* m) {
    return m ? m->num_keys : 0;
}

void mvm_insert(mvm* m, char* key, char* data) {
    hash_t* cell;

    if (m->num_buckets / m->table_size > FILL_FACTOR) {
        expandHashTable(m);
    }

    cell = insertKey(m, key, djb2Hash(key));

    insertData(cell, data);
    m->num_keys++;
}

void expandHashTable(mvm* m) {
    int i;
    hash_t* bucket;
    hash_t* table = m->hash_table;
    int size = m->table_size * HASH_FACTOR;

    mvm* tmp = mvm_initHelper(size);

    for (i = 0; i < m->table_size; i++) {
        if (table[i].key) {
            bucket = insertKey(tmp, table[i].key, table[i].hash);
            bucket->head = table[i].head;
        }
    }

    free(m->hash_table);

    m->hash_table = tmp->hash_table;
    free(m);
}

void insertData(hash_t* cell, char* data) {
    mvmcell* node = mvmcell_init(data);
    node->next = cell->head;
    cell->head = node;
}

/* FIXME doesn't resize yet */
/* Returns location for data to be stored in hash table */
hash_t* insertKey(mvm* m, char* key, unsigned long hash) {
    hash_t* table = m->hash_table;
    unsigned long index = hash % m->table_size;
    int offset = 0;

    for (;;) {
        if (!table[index].key) {
            fillBucket(table + index, key, hash, offset);
            m->num_buckets++;
            return table + index;
        }

        if (!strcmp(table[index].key, key)) {
            return table + index;
        }

        /* FIXME not 100% sure on the offset here */
        /* FIXME can we skip this evaluation on the first go? */
        if (offset > table[index].distance) {
            shiftBuckets(m, index);
            fillBucket(table + index, key, hash, offset);
            m->num_buckets++;
            return table + index;
        }

        offset++;
        index = (index + 1) % m->table_size;
    }
}

void shiftBuckets(mvm* m, unsigned long index) {
    hash_t* table = m->hash_table;
    hash_t tmp = table[index];

    for (;;) {
        index = (index + 1) % m->table_size;
        tmp.distance++;

        if (!table[index].key) {
            table[index] = tmp;
            return;
        }
        if (tmp.distance > table[index].distance) {
            swapBuckets(&tmp, table + index);
        }
    }
}

hash_t* findKey(mvm* m, char* key, unsigned long hash) {
    hash_t* table = m->hash_table;

    unsigned long index = hash % m->table_size;
    int offset = 0;

    while (table[index].key && offset <= table[index].distance) {
        if (!strcmp(table[index].key, key)) {
            return table + index;
        }

        offset++;
        index = (index + 1) % m->table_size;
    }
    return NULL;
}

void fillBucket(hash_t* bucket, char* key, unsigned long hash, unsigned long offset) {
    bucket->key = (char*)malloc(sizeof(char) * (strlen(key) + 1));
    if (!bucket->key) {
        ON_ERROR("Error allocating memory for key\n");
    }
    strcpy(bucket->key, key);

    bucket->hash = hash;
    bucket->distance = offset;
    bucket->head = NULL; /*FIXME this shouldn;t really need to be modified */
}

void swapBuckets(hash_t* b1, hash_t* b2) {
    hash_t tmp;
    tmp = *b1;
    *b1 = *b2;
    *b2 = tmp;
}

/* FIXME can this be broken down into functions a little bit more ? */
char* mvm_print(mvm* m) {
    int i;

    size_t buffer_size = AVE_CHARS * m->num_keys;
    char* buffer = initListBuffer(buffer_size);

    mvmcell* node;
    hash_t table;
    size_t curr_index = 0;
    size_t next_index = 0;

    for (i = 0; i < m->table_size; i++) {
        table = m->hash_table[i];
        if (table.key) {
            node = m->hash_table[i].head;

            while (node) {
                next_index += strlen(table.key) + strlen(node->data) + FORMAT_LEN;

                /* Check with "+ 1" to ensure there is space for NUll terminator if this
         * is the final appended string */
                if (next_index + 1 >= buffer_size) {
                    buffer_size = next_index * FACTOR;
                    expandListBuffer(&buffer, buffer_size);
                }

                sprintf(buffer + curr_index, "[%s](%s) ", table.key, node->data);

                curr_index = next_index;
                node = node->next;
            }
        }
    }
    return buffer;
}

void mvm_delete(mvm* m, char* key) {
    hash_t* bucket = findKey(m, key, djb2Hash(key));
    mvmcell* node;

    if (bucket) {
        node = bucket->head;
        bucket->head = node->next;
        mvmcell_unloadNode(node);
        m->num_keys--;

        if (bucket->head == NULL) {
            /* FIXME Not totally convinced by this indexing method */
            removeKey(m, bucket - m->hash_table);
        }
    }
}

void removeKey(mvm* m, ptrdiff_t base) {
    hash_t* table = m->hash_table;
    size_t curr = base;
    size_t next = (curr + 1) % m->table_size;

    free(table[curr].key);
    /* FIXME can this shift be made more efficient? */
    while (table[next].key && table[next].distance != 0) {
        table[curr] = table[next];
        table[curr].distance--;

        curr = next;
        next = (next + 1) % m->table_size;
    }

    clearBucket(&table[curr]);
    m->num_buckets--;
}

/* FIXME does everything actually need to be zeroed? */
void clearBucket(hash_t* bucket) {
    bucket->key = NULL;
    bucket->distance = 0;
    bucket->hash = 0;
    bucket->head = NULL;
}

char* mvm_search(mvm* m, char* key) {
    hash_t* bucket = findKey(m, key, djb2Hash(key));
    if (bucket) {
        return bucket->head->data;
    }
    return NULL;
}

char** mvm_multisearch(mvm* m, char* key, int* n) {
    int size = MULTI_SEARCH_LIST;
    int curr_index = 0;

    hash_t* bucket = findKey(m, key, djb2Hash(key));
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
            /*FIXME no error checking yet */
        }

        node = node->next;
    }

    *n = curr_index;
    return list;
}

void mvm_free(mvm** p) {
    mvm* m = *p;
    unloadTable(m->hash_table, m->table_size);
    free(m);

    *p = NULL;
}

void unloadTable(hash_t* table, size_t size) {
    size_t i;

    for (i = 0; i < size; i++) {
        if (table[i].key) {
            mvmcell_unloadList(table[i].head);
            free(table[i].key);
        }
    }
    free(table);
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

hash_t* initHashTable(int size) {
    hash_t* tmp = (hash_t*)calloc(size, sizeof(hash_t));
    if (!tmp) {
        ON_ERROR("Error allocating hash table\n");
    }
    return tmp;
}

mvmcell* mvmcell_init(char* data) {
    mvmcell* node = (mvmcell*)malloc(sizeof(mvmcell));
    if (!node) {
        ON_ERROR("Error allocating cell\n");
    }

    node->data = (char*)malloc(sizeof(char) * (strlen(data) + 1));
    if (!node->data) {
        ON_ERROR("Error allocating cell data\n");
    }
    strcpy(node->data, data);

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
