#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

/* Queue struct to be used for array implementation of binary heap priority
 * queue
 * https://bradfieldcs.com/algos/trees/priority-queues-with-binary-heaps/
 */
typedef struct queue_t {
    int count;
    char c;
    size_t end;
} queue_t;

/* ------ PRIORITY QUEUE FUNCTIONS ------ */
void initPQueue(queue_t* p_queue, node_t* node);
void insertPQueue(queue_t* p_queue, node_t* node);
node_t* getMin(queue_t* p_queue);
void delMin(queue_t* p_queue);
bool isEmpty(queue_t* p_queue);

void expandPQueue(queue_t* p_queue);
void percolateUp(queue_t* p_queue);
void percolateDown(queue_t* p_queue);
size_t minChildIndex(queue_t* p_queue, size_t i);
void swapNodePtr(node_t** n1, node_t** n2);
void unloadPQueue(queue_t* p_queue);

void test(void);

int main(void) {
    test();
    return 0;
}

void test(void) {
}