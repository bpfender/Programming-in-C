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
    fillBucket(table, "test_key", 1234, 0);
    unloadTable(table, 5);

    /* TODO check all testing conditions from mvm.c are included as well */
    printf("Basic MVM Tests ... Stop\n");
    return 0;
}
