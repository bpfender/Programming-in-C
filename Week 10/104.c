#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../Week 9/fileio.c"

/* Hash table size doesn't need to be huge for good performance. 500 seems to 
 * be a fairly good compromise */
#define HASH_TABLE 499
/* djb2 Hash constants, defined based on reference below
 * http://www.cse.yorku.ca/~oz/hash.html
 */
#define HASH 5381
#define MAGIC 33
#define PROBE_HASH 7

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
hash_t* initHashTable(size_t size);
void addToHashTable(hash_t* hashed, char* s);
void expandHashTable(hash_t* hashed);
size_t isPrime(size_t candidate);
unsigned long secondaryHash(char* s);
void insertString(char** dest, char* s);

int main(void) {
    test();
    return 0;
}

void loadDictionary(hash_t* hashed, char* file){

}

void addToHashTable(hash_t* hashed, char* s) {
    unsigned long hash, probe;
    size_t index;

    if (hashed->elem >= (size_t)(hashed->size * 3 / 4)) {
        expandHashTable(hashed);
    }

    index = hash = djb2Hash(s, hashed->size);

    if (hashed->string[index]) {
        /* Duplicates get ignored */
        if (!strcmp(s, hashed->string[index])) {
            return;
        }

        /* QUESTION would it be worth checking if the probe has looped? */
        probe = secondaryHash(s);
        do {
            index = (index + probe) % hashed->size;
        } while (hashed->string[index]);
    }

    insertString(&hashed->string[index], s);
    hashed->elem++;
}

void insertString(char** dest, char* s) {
    *dest = (char*)malloc(sizeof(char) * (strlen(s) + 1));
    if (!(*dest)) {
        ON_ERROR("Failed to allocate memory for string\n");
    }
    strcpy(*dest, s);
}

/* TODO maybe rewrite this without a return value */
void expandHashTable(hash_t* hashed) {
    hash_t tmp;
    size_t i;
    size_t size = hashed->size * 4;
    while (!isPrime(size)) {
        size++;
    }

    tmp.string = hashed->string;
    tmp.elem = hashed->elem;
    tmp.size = hashed->size;

    hashed->string = (char**)calloc(size, sizeof(char*));
    if (!hashed->string) {
        ON_ERROR("Error resizing hash table\n");
    }
    hashed->elem = 0;
    hashed->size = size;

    for (i = 0; i < tmp.size; i++) {
        if (tmp.string[i]) {
            addToHashTable(hashed, tmp.string[i]);
            free(tmp.string[i]);
        }
    }
    free(tmp.string);
}

hash_t* initHashTable(size_t size) {
    hash_t* tmp = (hash_t*)malloc(sizeof(hash_t));
    if (!tmp) {
        ON_ERROR("Error allocating hash struct\n");
    }

    tmp->string = (char**)calloc(size, sizeof(char*));
    if (!tmp->string) {
        ON_ERROR("Error allocating hash table\n");
    }

    tmp->elem = 0;
    tmp->size = size;

    return tmp;
}

void freeHashTable(hash_t** hashed) {
    size_t i;
    hash_t* p = *hashed;

    for (i = 0; i < p->size; i++) {
        free(p->string[i]);
    }
    free(p->string);
    free(*hashed);
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

/* FIXME not totally sure about this */
unsigned long secondaryHash(char* s) {
    size_t i;
    unsigned long hash = 0;

    for (i = 0; s[i] != '\0'; i++) {
        hash += (unsigned long)s[i];
    }
    return hash - hash % PROBE_HASH;
}

/* TODO could do with some optimisation */
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

void test(void) {
    size_t i;
    hash_t* hashed = initHashTable(HASH_TABLE);

    assert(hashed->size == HASH_TABLE);
    assert(hashed->elem == 0);
    assert(hashed->string[0] == NULL);

    freeHashTable(&hashed);
    assert(hashed == NULL);

    hashed = initHashTable(11);
    addToHashTable(hashed, "hello");

    for (i = 0; i < hashed->size; i++) {
        printf("%li: ", i);
        if (hashed->string[i]) {
            printf("%s  ", hashed->string[i]);
        }
        printf("\n");
    }
    printf("\n");

    freeHashTable(&hashed);

    hashed = initHashTable(11);

    assert(hashed->size == 11);
    assert(hashed->elem == 0);
    assert(hashed->string[0] == NULL);

    addToHashTable(hashed, "hello");

    assert(hashed->size == 11);
    assert(hashed->elem == 1);

    addToHashTable(hashed, "hello");
    assert(hashed->size == 11);
    assert(hashed->elem == 1);

    addToHashTable(hashed, "a");
    addToHashTable(hashed, "b");
    addToHashTable(hashed, "c");
    addToHashTable(hashed, "d");
    addToHashTable(hashed, "e");
    addToHashTable(hashed, "f");
    addToHashTable(hashed, "g");
    addToHashTable(hashed, "h");
    addToHashTable(hashed, "i");
    addToHashTable(hashed, "j");
    assert(hashed->size == 47);

    for (i = 0; i < hashed->size; i++) {
        printf("%li: ", i);
        if (hashed->string[i]) {
            printf("%s  ", hashed->string[i]);
        }
        printf("\n");
    }
    printf("\n");

    freeHashTable(&hashed);
}