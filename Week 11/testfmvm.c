#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fmvm.h"

int main(void) {
    mvm* m;
    mvmcell* node;
    hash_t* table;
    hash_t* bucket;

    int i, j;
    char* str;
    char** av;
    char animals[5][10] = {"cat", "dog", "bird", "horse", "frog"};
    char noises[5][10] = {"meow", "bark", "tweet", "neigh", "croak"};

    printf("Basic MVM Tests ... Start\n");

    m = mvm_init();
    assert(m->table_size == HASH_SIZE);
    assert(m->num_keys == 0);
    assert(m->num_buckets == 0);

    assert(m->hash_table[0].key == NULL);
    assert(m->hash_table[0].hash == 0);
    assert(m->hash_table[0].distance == 0);
    assert(m->hash_table[0].head == NULL);

    assert(mvm_size(m) == 0);

    mvm_free(&m);
    assert(m == NULL);

    node = mvmcell_init("test");
    assert(strcmp(node->data, "test") == 0);
    mvmcell_unloadNode(node);

    table = initHashTable(5);
    fillBucket(table, "test_key", 1234, 3);
    assert(table[0].distance == 3);
    assert(table[0].hash == 1234);
    assert(table[0].head == NULL);
    assert(strcmp(table[0].key, "test_key") == 0);

    swapBuckets(&table[0], &table[1]);
    assert(table[1].distance == 3);
    assert(table[1].hash == 1234);
    assert(table[1].head == NULL);
    assert(strcmp(table[1].key, "test_key") == 0);

    assert(table[0].distance == 0);
    assert(table[0].hash == 0);
    assert(table[0].head == NULL);
    assert(table[0].key == NULL);

    free(table[1].key);
    clearBucket(&table[1]);
    assert(table[1].distance == 0);
    assert(table[1].hash == 0);
    assert(table[1].head == NULL);
    assert(table[1].key == NULL);

    unloadTable(table, 5);

    /* shift buckets testing */
    m = mvm_init();
    table = m->hash_table;

    fillBucket(&table[0], "test1", 0, 1);
    fillBucket(&table[1], "test2", 1, 1);
    fillBucket(&table[2], "test3", 2, 1);
    fillBucket(&table[3], "test4", 2, 2);

    shiftBuckets(m, 2);
    assert(strcmp(table[0].key, "test1") == 0);
    assert(strcmp(table[1].key, "test2") == 0);
    assert(strcmp(table[2].key, "test3") == 0);
    assert(strcmp(table[3].key, "test4") == 0);
    assert(strcmp(table[4].key, "test3") == 0);

    assert(table[0].distance == 1);
    assert(table[1].distance == 1);
    assert(table[2].distance == 1);
    assert(table[3].distance == 2);
    assert(table[4].distance == 3);

    /* To avoid double free */
    table[2].key = NULL;

    mvm_free(&m);

    /* Input key testing */
    m = mvm_init();
    table = m->hash_table;

    /* Dummy hash value passed for testing */
    bucket = insertKey(m, "test_key", 0);

    assert(strcmp(table[0].key, "test_key") == 0);
    assert(strcmp(bucket->key, "test_key") == 0);
    assert(m->num_buckets == 1);

    bucket = insertKey(m, "test_key", 0);
    assert(strcmp(table[0].key, "test_key") == 0);
    assert(strcmp(bucket->key, "test_key") == 0);
    assert(m->num_buckets == 1);

    bucket = insertKey(m, "test_key_2", 1);
    assert(strcmp(table[1].key, "test_key_2") == 0);
    assert(table[1].distance == 0);
    assert(m->num_buckets == 2);

    bucket = insertKey(m, "test_key_3", 0);
    assert(strcmp(table[1].key, "test_key_3") == 0);
    assert(table[1].distance == 1);
    assert(strcmp(table[2].key, "test_key_2") == 0);
    assert(table[2].distance == 1);

    bucket = insertKey(m, "test_key_4", 0);
    assert(strcmp(table[1].key, "test_key_3") == 0);
    assert(table[1].distance == 1);
    assert(strcmp(table[2].key, "test_key_4") == 0);
    assert(table[2].distance == 2);
    assert(strcmp(table[3].key, "test_key_2") == 0);
    assert(table[3].distance == 2);

    bucket = insertKey(m, "test_key_5", 1);
    assert(strcmp(table[1].key, "test_key_3") == 0);
    assert(table[1].distance == 1);
    assert(strcmp(table[2].key, "test_key_4") == 0);
    assert(table[2].distance == 2);
    assert(strcmp(table[3].key, "test_key_2") == 0);
    assert(table[3].distance == 2);
    assert(strcmp(table[4].key, "test_key_5") == 0);
    assert(table[4].distance == 3);

    bucket = insertKey(m, "test_key_6", 0);
    assert(strcmp(table[0].key, "test_key") == 0);
    assert(table[0].distance == 0);
    assert(strcmp(table[1].key, "test_key_3") == 0);
    assert(table[1].distance == 1);
    assert(strcmp(table[2].key, "test_key_4") == 0);
    assert(table[2].distance == 2);
    assert(strcmp(table[3].key, "test_key_6") == 0);
    assert(table[3].distance == 3);
    assert(strcmp(table[4].key, "test_key_5") == 0);
    assert(table[4].distance == 3);
    assert(strcmp(table[5].key, "test_key_2") == 0);
    assert(table[5].distance == 4);

    mvm_free(&m);

    /* Insert key testing, data addition and search functionality */
    m = mvm_init();
    table = m->hash_table;

    bucket = insertKey(m, "test1", 1);
    insertData(bucket, "43");
    bucket = insertKey(m, "test2", 1);
    insertData(bucket, "17");

    assert(strcmp(table[1].key, "test1") == 0);
    assert(table[1].distance == 0);
    assert(strcmp(table[2].key, "test2") == 0);
    assert(table[2].distance == 1);

    bucket = insertKey(m, "test3", 2);
    insertData(bucket, "31");
    assert(strcmp(table[3].key, "test3") == 0);
    assert(table[3].distance == 1);

    bucket = insertKey(m, "test4", 3);
    insertData(bucket, "10");
    assert(strcmp(table[4].key, "test4") == 0);
    assert(table[4].distance == 1);

    bucket = insertKey(m, "test5", 3);
    insertData(bucket, "28");
    assert(strcmp(table[5].key, "test5") == 0);
    assert(table[5].distance == 2);

    bucket = insertKey(m, "test6", 2);
    insertData(bucket, "11");
    insertData(bucket, "76");
    assert(strcmp(table[4].key, "test6") == 0);
    assert(table[4].distance == 2);
    assert(strcmp(table[6].key, "test4") == 0);
    assert(table[6].distance == 3);

    bucket = insertKey(m, "test7", 7);
    insertData(bucket, "7");
    bucket = insertKey(m, "test8", HASH_SIZE - 1);
    insertData(bucket, "8");

    bucket = insertKey(m, "test9", HASH_SIZE - 1);
    insertData(bucket, "9");

    assert(strcmp(table[7].key, "test7") == 0);
    assert(table[7].distance == 0);
    assert(strcmp(table[HASH_SIZE - 1].key, "test8") == 0);
    assert(table[HASH_SIZE - 1].distance == 0);
    assert(strcmp(table[0].key, "test9") == 0);
    assert(table[0].distance == 1);

    assert(strcmp(table[1].head->data, "43") == 0);
    assert(strcmp(table[2].head->data, "17") == 0);
    assert(strcmp(table[3].head->data, "31") == 0);
    assert(strcmp(table[4].head->data, "76") == 0);
    assert(strcmp(table[5].head->data, "28") == 0);
    assert(strcmp(table[6].head->data, "10") == 0);

    assert(strcmp(table[7].head->data, "7") == 0);
    assert(strcmp(table[HASH_SIZE - 1].head->data, "8") == 0);
    assert(strcmp(table[0].head->data, "9") == 0);

    /* Test that linked list works properly */
    assert(strcmp(table[4].head->next->data, "11") == 0);

    /* Testing search functionality */
    /* FIXME do i need to add a bit more testing? */
    assert(findKey(m, "test1", 1) == &table[1]);
    assert(findKey(m, "test2", 1) == &table[2]);
    assert(findKey(m, "test3", 2) == &table[3]);
    assert(findKey(m, "test4", 3) == &table[6]);
    assert(findKey(m, "test5", 3) == &table[5]);
    assert(findKey(m, "test6", 2) == &table[4]);
    assert(findKey(m, "test9", HASH_SIZE - 1) == &table[0]);

    assert(findKey(m, "invalid", 3) == NULL);
    assert(findKey(m, "invalid", 8) == NULL);
    assert(findKey(m, "invalid", 3) == NULL);
    assert(findKey(m, "invalid", 7) == NULL);

    /* Testing remove key */

    /* This shouldn't do naything */
    removeKey(m, 8);

    /* Remove key would only be called if linked list is empty */
    mvmcell_unloadList(table[1].head);
    removeKey(m, 1);

    assert(strcmp(table[1].key, "test2") == 0);
    assert(strcmp(table[1].head->data, "17") == 0);
    assert(table[1].distance == 0);

    assert(strcmp(table[2].key, "test3") == 0);
    assert(strcmp(table[2].head->data, "31") == 0);
    assert(table[2].distance == 0);

    assert(strcmp(table[3].key, "test6") == 0);
    assert(strcmp(table[3].head->data, "76") == 0);
    assert(strcmp(table[3].head->next->data, "11") == 0);
    assert(table[3].distance == 1);

    assert(strcmp(table[4].key, "test5") == 0);
    assert(strcmp(table[4].head->data, "28") == 0);
    assert(table[4].distance == 1);

    assert(strcmp(table[5].key, "test4") == 0);
    assert(strcmp(table[5].head->data, "10") == 0);
    assert(table[5].distance == 2);

    assert(table[6].key == NULL);
    assert(table[6].hash == 0);
    assert(table[6].head == NULL);
    assert(table[6].distance == 0);

    assert(strcmp(table[7].key, "test7") == 0);
    assert(strcmp(table[7].head->data, "7") == 0);
    assert(table[7].distance == 0);

    mvmcell_unloadList(table[HASH_SIZE - 1].head);
    removeKey(m, HASH_SIZE - 1);

    assert(table[0].key == NULL);
    assert(table[0].hash == 0);
    assert(table[0].head == NULL);
    assert(table[0].distance == 0);
    assert(strcmp(table[HASH_SIZE - 1].key, "test9") == 0);
    assert(strcmp(table[HASH_SIZE - 1].head->data, "9") == 0);
    assert(table[HASH_SIZE - 1].distance == 0);

    mvm_free(&m);

    /* MORE Testing */
    m = mvm_init();
    assert(m != NULL);
    assert(mvm_size(m) == 0);

    assert(m->num_buckets == 0);
    assert(m->hash_table != NULL);

    /* Building and Searching */
    for (j = 0; j < 5; j++) {
        mvm_insert(m, animals[j], noises[j]);
        assert(mvm_size(m) == j + 1);
        assert(m->num_buckets == j + 1);
        i = strcmp(mvm_search(m, animals[j]), noises[j]);
        assert(i == 0);
    }

    str = mvm_print(m);
    assert(strstr(str, "[frog](croak) "));
    assert(strstr(str, "[horse](neigh) "));
    assert(strstr(str, "[bird](tweet) "));
    assert(strstr(str, "[dog](bark) "));
    assert(strstr(str, "[cat](meow) "));
    free(str);

    assert(mvm_search(m, "fox") == NULL);

    mvm_delete(m, "dog");
    assert(mvm_size(m) == 4);
    assert(m->num_buckets == 4);
    str = mvm_print(m);
    assert(strstr(str, "[frog](croak) "));
    assert(strstr(str, "[horse](neigh) "));
    assert(strstr(str, "[bird](tweet) "));
    assert(strstr(str, "[dog](bark) ") == NULL);
    assert(strstr(str, "[cat](meow) "));
    free(str);

    mvm_delete(m, "frog");
    assert(mvm_size(m) == 3);
    assert(m->num_buckets == 3);
    str = mvm_print(m);
    assert(strstr(str, "[frog](croak) ") == NULL);
    assert(strstr(str, "[horse](neigh) "));
    assert(strstr(str, "[bird](tweet) "));
    assert(strstr(str, "[dog](bark) ") == NULL);
    assert(strstr(str, "[cat](meow) "));
    free(str);

    /* Insert Multiple Keys */
    mvm_insert(m, "frog", "croak");
    mvm_insert(m, "frog", "ribbit");
    assert(mvm_size(m) == 5);
    assert(m->num_buckets == 4);
    str = mvm_print(m);
    assert(strstr(str, "[frog](croak) "));
    assert(strstr(str, "[frog](ribbit) "));
    assert(strstr(str, "[horse](neigh) "));
    assert(strstr(str, "[bird](tweet) "));
    assert(strstr(str, "[dog](bark) ") == NULL);
    assert(strstr(str, "[cat](meow) "));
    free(str);

    /* Search Multiple Keys */
    str = mvm_search(m, "frog");
    assert(strcmp(str, "ribbit") == 0);

    /* Multisearching */
    av = mvm_multisearch(m, "cat", &i);
    assert(i == 1);
    i = strcmp(av[0], "meow");
    assert(i == 0);
    free(av);
    av = mvm_multisearch(m, "horse", &i);
    assert(i == 1);
    i = strcmp(av[0], "neigh");
    assert(i == 0);
    free(av);
    av = mvm_multisearch(m, "frog", &i);
    assert(i == 2);
    i = strcmp(av[0], "ribbit");
    j = strcmp(av[1], "croak");
    assert((i == 0) && (j == 0));
    free(av);

    /* Delete Multiple Keys */
    mvm_delete(m, "frog");
    assert(mvm_size(m) == 4);
    assert(m->num_buckets == 4);
    mvm_delete(m, "frog");
    assert(mvm_size(m) == 3);
    assert(m->num_buckets == 3);
    str = mvm_print(m);
    assert(strstr(str, "[frog](croak) ") == NULL);
    assert(strstr(str, "[frog](ribbit) ") == NULL);
    assert(strstr(str, "[horse](neigh) "));
    assert(strstr(str, "[bird](tweet) "));
    assert(strstr(str, "[dog](bark) ") == NULL);
    assert(strstr(str, "[cat](meow) "));

    mvm_insert(m, "frog", "ribbit");
    mvm_insert(m, "frog", "croak");
    mvm_insert(m, "dog", "bark");
    mvm_insert(m, "eagle", "cacaw");
    mvm_insert(m, "elephant", "toureu");
    mvm_insert(m, "lion", "roaaar");
    mvm_insert(m, "hippo", "baarp");
    mvm_insert(m, "snake", "ssssss");
    mvm_insert(m, "sheep", "baahh");
    mvm_insert(m, "owl", "tuweet tuwoo");
    mvm_insert(m, "cow", "moooo");

    assert(m->table_size == 47);
    assert(strcmp(findKey(m, "frog", djb2Hash("frog"))->head->next->data, "ribbit") == 0);

    assert(i == 0);
    free(str);

    mvm_free(&m);

    /*TODO all the NULL testing cases */

    printf("Basic MVM Tests ... Stop\n");
    return 0;
}
