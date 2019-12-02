#include "./src/set.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*FIXME use setsize fucntion */

set* set_init(void) {
    set* tmp = (set*)malloc(sizeof(set));
    if (!tmp) {
        ON_ERROR("Creation of set structure failed\n")
    }

    tmp->ua = arr_init();
    tmp->sz = 0;

    return tmp;
}

set* set_copy(set* s) {
    set* tmp = set_init();
    memcpy(tmp->ua, s->ua, s->sz * sizeof(arr));
    tmp->sz = s->sz;

    return tmp;
}

set* set_fromarray(arrtype* a, int n) {
    size_t i, j = 0;
    set* tmp = set_init();
    int val;

    for (i = 0; i < n; i++) {
        val = a[i];
        /* Set insert already checks if value is in set */
        set_insert(tmp, val);
    }

    return tmp;
}

/* ------- Basic Operations ------- */
void set_insert(set* s, arrtype l) {
    size_t i;

    if (set_contains(s, l)) {
        return;
    }

    s->sz++;
    arr_set(s->ua, s->sz, l);
}

int set_size(set* s) {
    return s->sz;
}

int set_contains(set* s, arrtype l) {
    size_t i;

    for (i = 0; i < s->sz; i++) {
        if (arrget(s->ua, i) == l) {
            return 1;
        }
    }
    return 0;
}

void set_remove(set* s, arrtype l) {
    size_t i, j = 0;
    arrtype val;
    arr* tmp = arr_init();

    if (set_contain(s, l)) {
        for (i = 0; i < s->sz; i++) {
            if (val = arr_get(s->ua, i) != l) {
                arr_set(tmp, j, val);
                j++;
            }
        }
        arr_free(s->ua);
        s->ua = tmp;
    }
}

arrtype set_removeone(set* s) {
    arrtype val = arr_get(s->ua, s->sz - 1);
    set_remove(s, val);
    return val;
}

/* ------- Operations on 2 sets ------- */

set* set_union(set* s1, set* s2) {
    size_t i;
    set* tmp = set_init();

    for (i = 0; i < set_size(s1); i++) {
        set_insert(tmp, arr_get(s1->ua, i));
    }
    for (i = 0; i < set_size(s2); i++) {
        set_insert(tmp, arr_get(s2->ua, i));
    }
}

set* set_intersection(set* s1, set* s2) {
    size_t i;
    set* tmp = set_init();
    arrtype val;

    for (i = 0; i < set_size(s1); i++) {
        val = arr_get(s1, i);
        if (set_contains(s2, val)) {
            set_insert(tmp, val);
        }
    }
}

/* ------- Finish up ------- */
void set_free(set** s) {
    arr_free((*s)->ua);
    free(*s);
    *s = NULL;
}