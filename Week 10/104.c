#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

/* Hash table size doesn't need to be huge for good performance. 500 seems to 
 * be a fairly good compromise */
#define HASH_TABLE 500
/* djb2 Hash constants, defined based on reference below
 * http://www.cse.yorku.ca/~oz/hash.html
 */
#define HASH 5381
#define MAGIC 33

#define ON_ERROR(str)     \
    fprintf(stderr, str); \
    exit(EXIT_FAILURE)

typedef struct hash_t {
    char** string;
    size_t elem;
    size_t size;
} hash_t;

void test(void);
unsigned long djb2Hash(char* s, size_t size);
void freeHashTable(hash_t** hashed);
hash_t* initHashTable(void);
void addToHashTable(hash_t* hashed, char* s);

int main(void) {
    return 0;
}

void addToHashTable(hash_t* hashed, char* s) {
}

hash_t* initHashTable(void) {
    hash_t* tmp = (hash_t*)malloc(sizeof(hash_t));
    if (!tmp) {
        ON_ERROR("Error allocating hash struct\n");
    }

    tmp->string = (char**)calloc(HASH_TABLE, sizeof(char*));
    if (!tmp->string) {
        ON_ERROR("Error allocating hash table\n");
    }

    tmp->elem = 0;
    tmp->size = HASH_TABLE;

    return tmp;
}

void freeHashTable(hash_t** hashed) {
    size_t i;
    hash_t* p = *hashed;

    for (i = 0; i < p->elem; i++) {
        free(p->string[i]);
    }
    free(p->string);
    free(p);
    *hashed = NULL;
}

/* http://www.cse.yorku.ca/~oz/hash.html */
unsigned long djb2Hash(char* s, size_t size) {
    size_t i;
    unsigned long hash = HASH;
    for (i = 0; s[i] != '\0'; i++) {
        hash += hash * MAGIC ^ (unsigned long)s[i];
    }

    return hash % size;
}

void test(void) {
    hash_t* hashed = initHashTable();

    assert(hashed->size == HASH_TABLE);
    assert(hashed->elem == 0);
    assert(hashed->string[0] == NULL);

    free(hashed);
    assert(hashed == NULL);
}