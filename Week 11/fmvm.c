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
    mvm* tmp = (mvm*)malloc(sizeof(mvm));
    if (!tmp) {
        ON_ERROR("Error allocating memory for MVM\n");
    }

    tmp->hash_table = initHashTable(HASH_SIZE);
    tmp->table_size = HASH_SIZE;
    tmp->num_keys = 0;
    tmp->num_buckets = 0;

    return tmp;
}

int mvm_size(mvm* m) {
    return m ? m->num_keys : 0;
}

void mvm_insert(mvm* m, char* key, char* data) {
    hash_t* cell = insertKey(m, key);

    /* Add data entry to linked list */
    mvmcell* node = mvmcell_init(data);
    node->next = cell->head;
    cell->head = node;

    m->num_keys++;
}

/* FIXME doesn't resize yet */
/* Returns location for data to be stored in hash table */
hash_t* insertKey(mvm* m, char* key) {
    hash_t* table = m->hash_table;
    unsigned long hash = djb2Hash(key);
    unsigned long index = hash % m->table_size;
    int offset = 0;
    printf("INITIAL INDEX %li\n", index);

    for (;;) {
        if (!table[index].key) {
            fillBucket(table + index, key, hash, offset);
            m->num_buckets++;
            printf("HASH INDEX %li\n", index);
            return table + index;
        }

        if (!strcmp(table[index].key, key)) {
            printf("HASH INDEX %li\n", index);
            return table + index;
        }

        offset++;
        index = (index + 1) % m->table_size;

        /* FIXME not 100% sure on the offset here */
        if (offset > table[index].distance) {
            shiftBuckets(m, index);
            fillBucket(table + index, key, hash, offset);
            m->num_buckets++;
            printf("HASH INDEX %li\n", index);
            return table + index;
        }
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

hash_t* findKey(mvm* m, char* key) {
    hash_t* table = m->hash_table;

    unsigned long index = djb2Hash(key) % m->table_size;
    int offset = 0;

    /* Does this work with shortcircuit evaluation? */
    /* FIXME not 100% on offset calculation */
    while (table[index].key && table[index].distance >= offset) {
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
    hash_t* bucket = findKey(m, key);
    mvmcell* node;

    if (bucket) {
        printf("REMOVING %s\n", key);

        node = bucket->head;
        bucket->head = node->next;
        mvmcell_unloadNode(node);
        m->num_keys--;

        if (bucket->head == NULL) {
            printf("DELETING SHIT\n");
            /* FIXME Not totally convinced by this indexing method */
            removeKey(m, bucket - m->hash_table);
        }
    }
}

void removeKey(mvm* m, ptrdiff_t base) {
    hash_t* table = m->hash_table;
    size_t dest = base;
    size_t src = (base + 1) % m->table_size;

    /* FIXME can this shift be made more efficient? */
    while (table[src].key && table[src].distance != 0) {
        table[dest] = table[src];
        dest = (dest + 1) % m->table_size;
        src = (src + 1) % m->table_size;
        printf("%s ", table[dest].key);
    }
    printf("\n");

    clearBucket(&table[src]);
    m->num_buckets--;
}

/* FIXME does everything actually need to be zeroed? */
void clearBucket(hash_t* bucket) {
    free(bucket->key);
    bucket->key = NULL;
    bucket->distance = 0;
    bucket->hash = 0;
}

char* mvm_search(mvm* m, char* key) {
    hash_t* bucket = findKey(m, key);
    if (bucket) {
        return bucket->head->data;
    }
    return NULL;
}

char** mvm_multisearch(mvm* m, char* key, int* n) {
    int size = MULTI_SEARCH_LIST;
    int curr_index = 0;

    hash_t* bucket = findKey(m, key);
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
