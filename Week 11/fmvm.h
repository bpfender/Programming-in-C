/* Multi-Value Map ADT : via Linked List
   Both key & data are strings (char*) 
   Multiple Keys may be stored
*/

/* I explore quite a few methods for approaching the speedup of fmvm, looking at
 * skip lists, DAWGS, (succinct tries), red-black trees, bloom filters and a few
 * others. Ideally I would have like to try a few different approaches to see the
 * benefits and disadvantages of each. However, each of these appeared to require
 * some prior knowledge about the type of data being fed to the structure.
 * 
 * Looking for a more general solution and after a bit of reserach I came to the
 * conclusion that hashing might be and effective way of structuring sets (i.e. 
 * the MVM can store multiple datas per key) in a generally applicable way, I 
 * came to the conclusion that a combined approach of open addressing (unique 
 * key-values) and closed addressing (identical key value i.e. multiple datas) 
 * in a hash table might be an effective appraoch.
 * 
 * Reseraching different approaches for open addressing, I was looking for an 
 * approach that would provide good performance in most situations. I eventually
 * settled on Robin Hood hashing for the open addressing scheme. In particular,
 * this method was appealing due to the caching benefits of the initial key
 * lookup, with collisions stored close to their original location.
 * 
 * There is still some room for improvement. I'm slightly concerned that there is
 * quite a lot of overhead in generating the hash table and it would be
 * interesting to compare the speed of different operations to other data
 * structures. I would also consider updating the hash to murmur2 as this appears
 * to deal better with a wider range of data.
 * 
 * On the whole, this method appears to offer very fast lookup with additional
 * overhead associated with building the table. It's likely that the performance
 * gains for this relatively small dictionary aren't massive, but that performance
 * increases likely improve with the size of the dataset.
 * 
 * References used during work:
 * 
 * https://softwareengineering.stackexchange.com/questions/49550/which-hashing-algorithm-is-best-for-uniqueness-and-speed/145633#145633
 * https://andre.arko.net/2017/08/24/robin-hood-hashing/
 * https://programming.guide/robin-hood-hashing.html
 * https://tessil.github.io/2016/08/29/benchmark-hopscotch-map.html
 * http://stevehanov.ca/blog/?id=120
 * http://stevehanov.ca/blog/?id=119
 * 
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
int cumulAverage(int curr_av, int val, int n);
int removAverage(int curr_av, int val, int n);
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
