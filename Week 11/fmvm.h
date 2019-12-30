/* Multi-Value Map ADT : via Linked List
   Both key & data are strings (char*) 
   Multiple Keys may be stored
   FIXME explain the approach taken BLOOM FILTER, SKIP LIST, DAWG, vs. Hashing vs. Trie different option available
   New data is inserted at the front of the list:
   Hashing seems to make the most sense simply sa it's fairly robust regardless of the
   data fed to it. Succinct tries, radix trees, DAWGs etc all seem to require some prior
   knowledge about the data that will be fed to the problem. ALso we're only matching whole words, there's no need to find prefixes or the like
   Considered perfect hashing but don't know application of MVM, robin hood as general option, (PROVIDE REFS)   
   POssible that i'm destroying all the performance gains of caching through linked list
   Would be very interested to compare against performance of red black as have no idea about overhead

https://softwareengineering.stackexchange.com/questions/49550/which-hashing-algorithm-is-best-for-uniqueness-and-speed/145633#145633
https://andre.arko.net/2017/08/24/robin-hood-hashing/
https://programming.guide/robin-hood-hashing.html
https://tessil.github.io/2016/08/29/benchmark-hopscotch-map.html

   O(1) insertion
   O(n) search
   O(n) deletion
*/
#ifndef FMVM_H
#define FMVM_H

#include <stdlib.h>

#define HASH_SIZE 50021
#define HASH_FACTOR 4
#define FILL_FACTOR 0.7

#define ON_ERROR(STR)     \
    fprintf(stderr, STR); \
    exit(EXIT_FAILURE)

typedef struct mvmcell {
    char* data;
    struct mvmcell* next;
} mvmcell;

typedef struct hash_t {
    char* key;
    mvmcell* head;
    int offset;
    unsigned long hash;
} hash_t;

typedef struct mvm {
    hash_t* hash_table;
    int num_keys;
    int num_buckets;
    int table_size;
    int ave_len;
} mvm;

/* ------ MAIN MVM FUNCTIONS ------ */
mvm* mvm_init(void);
/* Number of key/value pairs stored */
int mvm_size(mvm* m);
/* Insert one key/value pair */
void mvm_insert(mvm* m, char* key, char* data);
/* Store list as a string "[key](value) [key](value) " etc.  */
char* mvm_print(mvm* m);
/* Remove one key/value */
void mvm_delete(mvm* m, char* key);
/* Return the corresponding value for a key */
char* mvm_search(mvm* m, char* key);
/* Return *argv[] list of pointers to all values stored with key, n is the number of values */
char** mvm_multisearch(mvm* m, char* key, int* n);
/* Free & set p to NULL */
void mvm_free(mvm** p);

/* Functions below are only declared here so that they can accessed by 
testfmvm. Wasn't sure how else to achieve this. */
/* ------ HELPER FUNCTIONS FOR MVM FUNCTIONALITY ------ */
/* ------ HASH TABLE FUNCTIONS ------ */
hash_t* insertKey(mvm* m, char* key, unsigned long hash);
void insertData(hash_t* bucket, char* data);
hash_t* findKey(mvm* m, char* key, unsigned long hash);
void removeKey(mvm* m, int base);

void fillBucket(hash_t* bucket, char* key, unsigned long hash, int offset);
void shiftBuckets(mvm* m, unsigned long index);
void swapBuckets(hash_t* b1, hash_t* b2);
void clearBucket(hash_t* bucket);

void expandHashTable(mvm* m);
int nextTableSize(int n);
char isPrime(int candidate);
unsigned long djb2Hash(char* s);

/* ------ HELPER FUNCTIONS ------- */
int updateAverage(int curr_av, int val, int n);
void printList(char** buffer, size_t* curr, hash_t* bucket);

/* ------ INITIALISATION/ALLOC FUNCTIONS ------- */
mvm* mvm_initHelper(int size);
hash_t* initHashTable(int size);
mvmcell* mvmcell_init(char* data);
void* allocHandler(void* ptr, size_t nmemb, size_t size);

/* ------ UNLOAD FUNCTIONS ------ */
void unloadTable(hash_t* table, int size);
void mvmcell_unloadList(mvmcell* node);
void mvmcell_unloadNode(mvmcell* node);

#endif
