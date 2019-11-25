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

int main(void) {
    char c;
    node_t* head_1 = makeNode('A');
    node_t* head_2 = makeNode('A');

    node_t* n;
    srand(time(NULL));
    for (c = 'B'; c < 'G'; c++) {
        n = makeNode(c);
        insertRandom(head_1, n);

        n = makeNode(c);
        insertRandom(head_2, n);
    }
    printf("%s\n", printTree(head_1));
    printf("%s\n", printTree(head_2));

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

int compareTrees(node_t* t1, node_t* t2) {
}
