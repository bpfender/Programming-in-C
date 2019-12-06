#include "varr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* QUESTION <> vs "" */

#define ARRDEFSZ 32

arr* arr_init(int elementsize) {
    arr* tmp = (arr*)malloc(sizeof(arr));
    if (!tmp) {
        ON_ERROR("Error creating array structure\n");
    }

    tmp->data = (void*)malloc(ARRDEFSZ * elementsize);
    if (!tmp->data) {
        ON_ERROR("Error initialising array\n");
    }

    tmp->pz = 0;
    tmp->elsz = elementsize;

    return tmp;
}

void arr_set(arr* l, int n, void* v) {
    void* tmp;
    if (n < 0) {
        ON_ERROR("Negative index accessed\n");
    }

    if (n >= l->pz) {
        tmp = (void*)realloc(l->data, l->elsz * (n + 1));
        if (!tmp) {
            ON_ERROR("Array resize failed\n");
        }
        /* QUESTION is this the correct way to do this? because char* is just 1 size*/
        memcpy((char*)tmp + n * l->elsz, v, l->elsz);
        l->data = tmp;
    }
    l->pz = n + 1;
}

void* arr_get(arr* l, int n) {
    if (n < 0 || n >= l->pz) {
        ON_ERROR("Array read out of bounds\n");
    }
    /* QUESTION as above.. is this right? */
    return (void*)((char*)l->data + n * l->elsz);
}

void arr_free(arr** l) {
    /* QEUSTION does double dereference costs extra processing steps? */
    free((*l)->data);
    free(*l);
    *l = NULL;
}
