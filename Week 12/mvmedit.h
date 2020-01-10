/* Multi-Value Map ADT : via Linked List
   Both key & data are strings (char*) 
   Multiple Keys may be stored
   New data is inserted at the front of the list:
   O(1) insertion
   O(n) search
   O(n) deletion
*/

/* NOTE modified to return mvmcell* rather than cahar*/

/* Error that can't be ignored */
#define ON_ERROR(STR)     \
    fprintf(stderr, STR); \
    exit(EXIT_FAILURE)

struct mvmcell {
    char* key;
    void* data;
    struct mvmcell* next;
};
typedef struct mvmcell mvmcell;

struct mvm {
    mvmcell* head;
    int numkeys;
};
typedef struct mvm mvm;

mvm* mvm_init(void);
/* Number of key/value pairs stored */
int mvm_size(mvm* m);
/* Insert one key/value pair */
void mvm_insert(mvm* m, char* key, void* data);
/* Remove one key/value */
void mvm_delete(mvm* m, char* key);
/* Return the corresponding value for a key */
mvmcell* mvm_search(mvm* m, char* key);
/* Free & set p to NULL */
void mvm_free(mvm** p);

char* mvm_print(mvm* m);
