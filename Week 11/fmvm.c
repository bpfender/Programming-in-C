#include "fmvm.h"
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

        /* FIXME not 100% sure on the offset here */
        /* FIXME can we skip this evaluation on the first go? */
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


void insertData(hash_t* cell, char* data) {
    mvmcell* node = mvmcell_init(data);
    node->next = cell->head;
    cell->head = node;
}

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

void removeKey(mvm* m, ptrdiff_t base) {
    hash_t* table = m->hash_table;
    size_t curr = base;
    size_t next = (curr + 1) % m->table_size;

    free(table[curr].key);
    /* FIXME can this shift be made more efficient? */
    while (table[next].key && table[next].offset != 0) {
        table[curr] = table[next];
        table[curr].offset--;

        curr = next;
        next = (next + 1) % m->table_size;
    }

    clearBucket(&table[curr]);
    m->num_buckets--;
}


void fillBucket(hash_t* bucket, char* key, unsigned long hash, unsigned long offset) {
    bucket->key = (char*)malloc(sizeof(char) * (strlen(key) + 1));
    if (!bucket->key) {
        ON_ERROR("Error allocating memory for key\n");
    }
    strcpy(bucket->key, key);

    bucket->hash = hash;
    bucket->offset = offset;
    bucket->head = NULL; /*FIXME this shouldn;t really need to be modified */
}


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



void swapBuckets(hash_t* b1, hash_t* b2) {
    hash_t tmp;
    tmp = *b1;
    *b1 = *b2;
    *b2 = tmp;
}

/* FIXME does everything actually need to be zeroed? */
void clearBucket(hash_t* bucket) {
    bucket->key = NULL;
    bucket->offset = 0;
    bucket->hash = 0;
    bucket->head = NULL;
}

/* FIXME requires expansion limiter */
void expandHashTable(mvm* m) {
    int i;
    hash_t* bucket;
    hash_t* table = m->hash_table;
    mvm* tmp;

    int size = m->table_size * HASH_FACTOR;
    while (!isPrime(size)) {
        size++;
    }

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


size_t isPrime(size_t candidate) {
    size_t j;

    if (candidate == 2) {
        return 1;
    }
    if (candidate < 2 || candidate % 2 == 0) {
        return 0;
    }
    for (j = 3; j <= candidate / 2; j += 2) {
        if (candidate % j == 0) {
            return 0;
        }
    }
    return 1;
}




mvm* mvm_initHelper(size_t size) {
    mvm* tmp = (mvm*)allocHandler(NULL, 1, sizeof(mvm));

    tmp->hash_table = initHashTable(size);
    tmp->table_size = size;
    tmp->num_keys = 0;
    tmp->num_buckets = 0;
    tmp->ave_len = 0;

    return tmp;
}

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
