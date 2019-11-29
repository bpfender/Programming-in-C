#include <stdio.h>
#include <stdlib.h>

/* Priority queue definitions */
#define QUEUE_SIZE 500
#define BUFF_FACTOR 2

typedef enum bool { false = 0,
                    true = 1 } bool;

typedef struct node_t {
    unsigned long freq;
    char c;
    struct node_t* l;
    struct node_t* r;
} node_t;

/* Queue struct to be used for array implementation of binary heap priority
 * queue
 * https://bradfieldcs.com/algos/trees/priority-queues-with-binary-heaps/
 */
typedef struct queue_t {
    struct node_t** node;
    size_t end;
    size_t size;
} queue_t;

/* ------ PRIORITY QUEUE FUNCTIONS ------ */
void initPQueue(queue_t* p_queue);
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

/* ------- PRIORITY QUEUE FUNCTIONS ------- */
/* Priority queue is implemented as a binary heap with a dynamically sized 
 * array. Should offer O(1) retrieval of next (priority) node, O(log n) deletion
 * and insertion. Looking to minimise cost function so min f-value will be moved
 * to top
 */

/* Initiliases priority queue to array of size QUEUE_SIZE. Adds starting grid to
 * queue
 */
void initPQueue(queue_t* p_queue) {
    /* +1 for permanent sentinel value */
    p_queue->node = (node_t**)malloc((QUEUE_SIZE + 1) * sizeof(node_t*));
    if (p_queue->node == NULL) {
        fprintf(stderr, "Memory allocation error\n");
        exit(EXIT_FAILURE);
    }

    /* Set sentinel node value*/
    p_queue->node[0] = NULL;

    p_queue->size = QUEUE_SIZE;
    p_queue->end = 0;
}

/* Insert element into priority queue and put it in right position in binary
 * heap
 */
void insertPQueue(queue_t* p_queue, node_t* node) {
    if (p_queue->end == p_queue->size) {
        expandPQueue(p_queue);
    }

    p_queue->end++;

    p_queue->node[p_queue->end] = node;
    percolateUp(p_queue);
}

/* Retrieve the minimum f-value node from the queue. It does not get remove
 */
node_t* getMin(queue_t* p_queue) {
    return p_queue->node[1];
}

/* Delecte minimum f-value node from queue and bubble up next smallest f-value
 * node
 */
void delMin(queue_t* p_queue) {
    p_queue->node[1] = p_queue->node[p_queue->end];
    percolateDown(p_queue);

    p_queue->end--;
}

bool isEmpty(queue_t* p_queue) {
    if (p_queue->end == 0) {
        return true;
    }
    return false;
}

/* Reallocates array if there are too many elements in the queue. Expanded by
 * factor determined by BUFF_FACTOR
 */
void expandPQueue(queue_t* p_queue) {
    node_t** tmp;
    p_queue->size = p_queue->size * BUFF_FACTOR;

    tmp = (node_t**)realloc(p_queue->node, (p_queue->size + 1) * sizeof(node_t*));
    if (tmp == NULL) {
        fprintf(stderr, "Memory allocation error\n");
        /* QUESTION should i free previously allced memory?*/
        exit(EXIT_FAILURE);
    }
    p_queue->node = tmp;
}

/* Moves inserted element to corrrect place in tree
 */
void percolateUp(queue_t* p_queue) {
    node_t **child, **parent;
    size_t i = p_queue->end;

    while (i / 2 > 0) {
        child = &(p_queue->node[i]);
        parent = &(p_queue->node[i / 2]);
        if ((*child)->freq < (*parent)->freq) {
            swapNodePtr(child, parent);
        }
        i /= 2;
    }
}

/* Moves min element to top of tree when previous min is remove
 */
void percolateDown(queue_t* p_queue) {
    node_t **child, **parent;
    size_t i = 1;

    while (i * 2 <= p_queue->end) {
        parent = &p_queue->node[i];

        i = minChildIndex(p_queue, i);
        child = &p_queue->node[i];

        if ((*child)->freq < (*parent)->freq) {
            swapNodePtr(child, parent);
        }
    }
}

/* Find the minimum value child to determine which branch to move up a level
 */
size_t minChildIndex(queue_t* p_queue, size_t i) {
    int f1, f2;
    size_t child_index = i * 2;

    /* If final node only has one branch, can only return this */
    if (child_index == p_queue->end) {
        return child_index;
    }

    /* Get f-value of each child */
    f1 = p_queue->node[child_index]->freq;
    f2 = p_queue->node[child_index + 1]->freq;

    if (f1 < f2) {
        return child_index;
    } else {
        return child_index + 1;
    }
}

/* Each node in binary tree is a pointer to a node, so need to swap the pointers
 * by passing pointers to pointers into function
 */
void swapNodePtr(node_t** n1, node_t** n2) {
    node_t* tmp = *n1;
    *n1 = *n2;
    *n2 = tmp;
}

void unloadPQueue(queue_t* p_queue) {
    free(p_queue->node);
}
