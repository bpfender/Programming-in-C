#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define STRSIZE 5000

struct node {
    char c;
    struct node* left;
    struct node* right;
};
typedef struct node node_t;

node_t*
makeNode(char c);
void insertRandom(node_t* t, node_t* n);
char* printTree(node_t* t);
unsigned int findTreeDepth(node_t* t);

int main(void) {
    char c;
    node_t* head = makeNode('A');
    node_t* n;
    srand(time(NULL));
    for (c = 'B'; c < 'G'; c++) {
        n = makeNode(c);
        insertRandom(head, n);
    }
    printf("%s\n", printTree(head));
    printf("Tree depth: %i\n", findTreeDepth(head));
    return 0;
}

node_t* makeNode(char c) {
    node_t* n = (node_t*)calloc(1, sizeof(node_t));
    assert(n != NULL);
    n->c = c;
    return n;
}

void insertRandom(node_t* t, node_t* n) {
    if ((rand() % 2) == 0) { /* Left */
        if (t->left == NULL) {
            t->left = n;
        } else {
            insertRandom(t->left, n);
        }
    } else { /* Right */
        if (t->right == NULL) {
            t->right = n;
        } else {
            insertRandom(t->right, n);
        }
    }
}

char* printTree(node_t* t) {
    char* str;
    assert((str = calloc(STRSIZE, sizeof(char))) != NULL);
    if (t == NULL) {
        strcpy(str, "*");
        return str;
    }
    sprintf(str, "%c (%s) (%s)", t->c, printTree(t->left), printTree(t->right));
    return str;
}

unsigned int findTreeDepth(node_t* t) {
    /* Initialise node value */
    unsigned int l = 1, r = 1;

    /* Traverse the tree */
    if (t->left) {
        l += findTreeDepth(t->left);
    }
    if (t->right) {
        r += findTreeDepth(t->right);
    }

    /* If end node, return 1 otherwise, reutrn bigger value child */
    if (!(t->right && t->left)) {
        return 1;
    } else {
        return (l > r) ? l : r;
    }
}