#include "fmvm.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FRMT_CHARS strlen("[]() ")
#define MULTI_SEARCH_LEN 5
/* Universal resizing factor */
#define FACTOR 2

/* djb2 Hash constants, defined based on reference below
 * http://www.cse.yorku.ca/~oz/hash.html
 */
#define DJB2_HASH 5381
#define DJB2_MAGIC 33

/* ------ MAIN MVM FUNCTIONS ------ */
/* Effectively written as a wrapper of mvm_initHelper() with size set to initial
 * hash table size, to allow the initHelper function to be reused for resizing
 * the table in expandHashTable().
 */
mvm* mvm_init(void) {
    return mvm_initHelper(HASH_SIZE);
}

int mvm_size(mvm* m) {
    return m ? m->num_keys : 0;
}

/* Called to insert key-data pair into hash table. First checks the load factor
 * of the table based on the buckets used (not the number of keys) and expands
 * it if required. insertKey() is called which returns the hash table location
 * where the data is to be stored. insertData() then adds the data value to that
 * location. Requires valid input to do anything.
 */
void mvm_insert(mvm* m, char* key, char* data) {
    hash_t* cell;

    if (m && key && data) {
        if (m->num_buckets > (m->table_size * FILL_FACTOR)) {
            expandHashTable(m);
        }

        /*FIXME add size limiter */
        /* FIXME could add average char length calculator here */
        cell = insertKey(m, key, djb2Hash(key));
        insertData(cell, data);
        m->num_keys++;
        /* FIXME make neater */
        m->ave_len = updateAverage(m->ave_len, strlen(key) + strlen(data), m->num_keys);
    }
}

/* Compared to mvm, this function can allocate a buffer of the right size
 * straight away as the average char count is tracked in the mvm struct. The
 * print string is generated by finding keys in the hash table and traversing
 * the corresonding linked list for that entry.
 */
char* mvm_print(mvm* m) {
    if (m) {
        size_t size = (m->ave_len + FRMT_CHARS) * m->num_keys + 1;
        char* buffer = (char*)allocHandler(NULL, size, sizeof(char));
        size_t curr = 0;
        int i;

        for (i = 0; i < m->table_size; i++) {
            if (m->hash_table[i].key) {
                printList(&buffer, &curr, &m->hash_table[i]);
            }
        }
        return buffer;
    }
    return NULL;
}

/* Removes data corresponding to a key from the mvm struct. findKey() is called
 * to return the hash_table entry that the key is stored in. The data is removed
 * from the head of the linked list. If the list still has entries the function
 * returns. If the list is empty, the key also has to be removed from the hash
 * table by calling removeKey(). Invalid input does nothing.
 */
void mvm_delete(mvm* m, char* key) {
    if (m && key) {
        hash_t* bucket = findKey(m, key, djb2Hash(key));
        mvmcell* node;

        if (bucket) {
            node = bucket->head;
            bucket->head = node->next;
            mvmcell_unloadNode(node);
            m->num_keys--;

            if (bucket->head == NULL) {
                /* ptr arithmetic of bucket - m->hash_table results in bucket 
                index. This is needed for proper shifting in removeKey() */
                removeKey(m, bucket - m->hash_table);
            }
        }
    }
}

/* Just call findKey() which provides ptr to a bucket with the corresponding
 * key. mvm_search() then returns the head of the linked list associated with
 * that bucket. If the key isn't found or on invalid input, NULL is returned. As
 * before, return value just points to data in mvm, rather than copying it into
 * seperately allocated memory.
 */
char* mvm_search(mvm* m, char* key) {
    if (m && key) {
        hash_t* bucket = findKey(m, key, djb2Hash(key));
        if (bucket) {
            return bucket->head->data;
        }
    }
    return NULL;
}

/* Returns list of char* to data corresponding to a key. On invalid input NULL
 * is returned. If the key is not found, an empty list is returned and n is set
 * to zero.
 */
char** mvm_multisearch(mvm* m, char* key, int* n) {
    if (m && key && n) {
        int size = MULTI_SEARCH_LEN;
        char** list = (char**)allocHandler(NULL, size, sizeof(char*));

        hash_t* bucket = findKey(m, key, djb2Hash(key));
        /* If key isn't found, node is set to NULL */
        mvmcell* node = bucket ? bucket->head : NULL;
        int i = 0;

        while (node) {
            if (i >= size) {
                size *= FACTOR;
                list = (char**)allocHandler(list, size, sizeof(char*));
            }

            list[i++] = node->data;
            node = node->next;
        }

        *n = i;
        return list;
    }
    *n = 0;
    return NULL;
}

/* Frees mvm struct by freeing hash table followed by struct itself. 
 */
void mvm_free(mvm** p) {
    mvm* m = *p;
    unloadTable(m->hash_table, m->table_size);
    free(m);

    *p = NULL;
}

/* ------- HELPER FUNCTIONS FOR MVM FUNCTIONALITY ------ */
/* ------- HASH TABLE FUNCTIONS ------ */
/* Given a key, this function returns the location in the hash table that it
 * should be stored. Takes hash as value to make resizing quicker, as the value
 * can be passed directly from the value stored rather than being recalculated.
 * Probing starts at hash%table size and shifts according to Robin Hood hashing
 * scheme. If the key already exists, the hash_t* can simply be returned. On an
 * empty cell, the bucket first has to be filled, and on linear probing, entries
 * may have to be shifted in the table before the bucket is filled.
 */
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

        /* Strictly this evaluation is surplus when offset is 0, but code is
        neater like this. However order of evaluation has to stay like this */
        if (offset > table[index].offset) {
            shiftBuckets(m, index);
            fillBucket(table + index, key, hash, offset);
            m->num_buckets++;
            return table + index;
        }

        offset++;
        index = (index + 1) % m->table_size;
    }
}

/* Helper function to add data entry to head of the linked list of the
 * corresponding bucket in hash table */
void insertData(hash_t* bucket, char* data) {
    mvmcell* node = mvmcell_init(data);
    node->next = bucket->head;
    bucket->head = node;
}

/* Searches for key in hash table, starting at hash % m->table_size. Probes
 * according to Robin Hood scheme, until equivalent offset is greater than found
 * offset or empty cell is found. Returns ptr to key bucket.
 */
hash_t* findKey(mvm* m, char* key, unsigned long hash) {
    hash_t* table = m->hash_table;

    unsigned long index = hash % m->table_size;
    int offset = 0;

    while (table[index].key && offset <= table[index].offset) {
        if (!strcmp(table[index].key, key)) {
            return table + index;
        }

        offset++;
        index = (index + 1) % m->table_size;
    }
    return NULL;
}

/* Called when all data entries have been removed from hash table bucket.
 * Requires key to be freed (as the only malloced item) and buckets to be
 * shifted down the table until an empty cell or offset 0 cell is reached.
 * Clears the last bucket shifted from so it is free for insertion again.
 */
void removeKey(mvm* m, int base) {
    hash_t* table = m->hash_table;
    size_t curr = base;
    size_t next = (curr + 1) % m->table_size;

    free(table[curr].key);
    while (table[next].key && table[next].offset != 0) {
        table[curr] = table[next];
        table[curr].offset--;

        curr = next;
        next = (next + 1) % m->table_size;
    }
    clearBucket(&table[curr]);
    m->num_buckets--;
}

/* Populates bucket with values and allocates memory for key
 */
void fillBucket(hash_t* bucket, char* key, unsigned long hash, int offset) {
    bucket->key = (char*)allocHandler(NULL, strlen(key) + 1, sizeof(char));
    strcpy(bucket->key, key);

    bucket->hash = hash;
    bucket->offset = offset;
    bucket->head = NULL;
}

/* Shifts bucket up the table. Swaps buckets out when table offset is greater 
 * than effective offset in current bucket (a la Robin Hood), and then the 
 * swapped bucket gets shifted up the table, until an empty cell is reached.
 */
void shiftBuckets(mvm* m, unsigned long index) {
    hash_t* table = m->hash_table;
    hash_t tmp = table[index];

    for (;;) {
        index = (index + 1) % m->table_size;
        tmp.offset++;

        if (!table[index].key) {
            table[index] = tmp;
            return;
        }
        if (tmp.offset > table[index].offset) {
            swapBuckets(&tmp, table + index);
        }
    }
}

/* Helper function to swap out hash table buckets
 */
void swapBuckets(hash_t* b1, hash_t* b2) {
    hash_t tmp;
    tmp = *b1;
    *b1 = *b2;
    *b2 = tmp;
}

/* Zeros values in bucket. Only really needs key and head to be set to NULL to
 * work properly, but does all values for completeness
 */
void clearBucket(hash_t* bucket) {
    bucket->key = NULL;
    bucket->offset = 0;
    bucket->hash = 0;
    bucket->head = NULL;
}

/* FIXME requires expansion limiter */
void expandHashTable(mvm* m) {
    mvm* tmp;
    hash_t* table = m->hash_table;
    hash_t* bucket;
    int i;

    int size = nextTableSize(m->table_size);
    tmp = mvm_initHelper(size);

    for (i = 0; i < m->table_size; i++) {
        if (table[i].key) {
            bucket = insertKey(tmp, table[i].key, table[i].hash);
            bucket->head = table[i].head;
            free(table[i].key);
        }
    }

    free(m->hash_table);

    m->hash_table = tmp->hash_table;
    m->num_buckets = tmp->num_buckets;
    m->table_size = size;

    free(tmp);
}

/* Calculates next table size. If HASH_FACTOR is even, +1 is added to result
 * of multiplcation to make it odd (prime table sizes will only ever be odd). If
 * HASH_FACTOR is odd, no +1 will be added. Will break for initial size 2, but
 * that would be silly.
 */
int nextTableSize(int n) {
    int size = n * HASH_FACTOR + !(HASH_FACTOR % 2);
    /* =2 skips over even numbers */
    while (!isPrime(size)) {
        size += 2;
    }
    return size;
}

/* https://en.wikipedia.org/wiki/Primality_test
 * Could be optimised further but likley not the limiting factor during hash
 * table resizing, especially given that sizes aren't likely to get that huge
 */
int isPrime(int candidate) {
    int j;

    if (candidate == 2) {
        return 1;
    }
    if (candidate % 2 == 0 || candidate < 2) {
        return 0;
    }
    for (j = 3; j <= (int)sqrt(candidate); j += 2) {
        if (candidate % j == 0) {
            return 0;
        }
    }
    return 1;
}

/* http://www.cse.yorku.ca/~oz/hash.html */
unsigned long djb2Hash(char* s) {
    size_t i;
    unsigned long hash = DJB2_HASH;
    for (i = 0; s[i] != '\0'; i++) {
        hash += hash * DJB2_MAGIC ^ (unsigned long)s[i];
    }

    return hash;
}

/* ------ HELPER FUNCTIONS ------ */
/* https://en.wikipedia.org/wiki/Moving_average#Cumulative_moving_average 
 * https://stackoverflow.com/questions/2745074/fast-ceiling-of-an-integer-division-in-c-c
 * Calculates the cumulative average as described by the Wikiedia link and
 * returns a ceiled value.
 */
int updateAverage(int curr_av, int val, int n) {
    return curr_av + ((val - curr_av) + (n - 1)) / n;
}

/* Helper function that traverses linked list stored in hash table bucket and 
 * adds it to the print buffer
 */
void printList(char** buffer, size_t* curr, hash_t* bucket) {
    mvmcell* node = bucket->head;

    while (node) {
        sprintf(*buffer + *curr, "[%s](%s) ", bucket->key, node->data);

        *curr += strlen(bucket->key) + strlen(node->data) + FRMT_CHARS;
        node = node->next;
    }
}

/* ------ INITIALISATION/ALLOC FUNCTIONS ----- */
/* Defined to allow a new mvm to be allocated when table is resized 
 */
mvm* mvm_initHelper(size_t size) {
    mvm* tmp = (mvm*)allocHandler(NULL, 1, sizeof(mvm));

    tmp->hash_table = initHashTable(size);
    tmp->table_size = size;
    tmp->num_keys = 0;
    tmp->num_buckets = 0;
    tmp->ave_len = 0;

    return tmp;
}

/* Initialises an empty hash table 
 */
hash_t* initHashTable(int size) {
    hash_t* tmp = (hash_t*)calloc(size, sizeof(hash_t));
    if (!tmp) {
        ON_ERROR("Error allocating hash table\n");
    }
    return tmp;
}

/* Initialises linked list node with data
 */
mvmcell* mvmcell_init(char* data) {
    mvmcell* node = (mvmcell*)allocHandler(NULL, 1, sizeof(mvmcell));
    node->data = (char*)allocHandler(NULL, strlen(data)+1, sizeof(char));
    strcpy(node->data, data);

    return node;
}

/* Wrote a generic malloc/realloc function because the same structure was
 * repeating itself multiple times and I wanted to play with void*. Requires
 * ptr = NULL for initial malloc, and ptr value for resizing an existing block
 */
void* allocHandler(void* ptr, size_t nmemb, size_t size) {
    void* tmp = realloc(ptr, nmemb * size);
    if (!tmp) {
        ON_ERROR("Memory allocation error\n");
    }
    return tmp;
}

/* ------ UNLOAD FUNCTIONS ------ */
/* PAsses through table and frees and non-null entries */
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
