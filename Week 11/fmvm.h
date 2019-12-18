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


   O(1) insertion
   O(n) search
   O(n) deletion
*/

/* Error that can't be ignored */
#define ON_ERROR(STR)     \
    fprintf(stderr, STR); \
    exit(EXIT_FAILURE)

typedef struct mvmcell {
    char* data;
    struct mvmcell* next;
} mvmcell;

typedef struct bucket_t {
    char* key;
    mvmcell* head;
    int distance;
    unsigned long hash;
} bucket_t;

/* FIXME types int vs size_t */
typedef struct mvm {
    bucket_t* hash_table;
    int num_keys;
    int num_buckets;
    int table_size;
} mvm;

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
