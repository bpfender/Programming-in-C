#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Is it better to include .h */
#include "fmvm.h"

int main(void) {
    mvm* m;
    mvmcell* node;
    hash_t* table;
    hash_t* bucket;

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

    assert(strcmp(table[1].head->data, "43") == 0);
    assert(strcmp(table[2].head->data, "17") == 0);
    assert(strcmp(table[3].head->data, "31") == 0);
    assert(strcmp(table[4].head->data, "76") == 0);
    assert(strcmp(table[5].head->data, "28") == 0);
    assert(strcmp(table[6].head->data, "10") == 0);

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

    assert(findKey(m, "invalid", 3) == NULL);
    assert(findKey(m, "invalid", 8) == NULL);
    assert(findKey(m, "invalid", 3) == NULL);
    assert(findKey(m, "invalid", 7) == NULL);

    /* Testing remove key */
    bucket = insertKey(m, "test7", 7);
    insertData(bucket, "7");

    removeKey(m, 1);
    /*printf("%s\n", table[1].key);
    assert(strcmp(table[1].key, "test2")==0);*/

    mvm_free(&m);

    /* TODO check all testing conditions from mvm.c are included as well */
    printf("Basic MVM Tests ... Stop\n");
    return 0;
}
