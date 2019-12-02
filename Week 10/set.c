#include "set.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

set* set_init(void) {
    set* tmp = (set*)malloc(sizeof(set));
    if (!tmp) {
        ON_ERROR("Creation of set failed\n");
    }

    tmp->ua = arr_init();
    tmp->sz = 0;

    return tmp;
}

set* set_copy(set* s) {
    int i;
    set* tmp = set_init();

    for (i = 0; i < set_size(s); i++) {
        set_insert(tmp, arr_get(s->ua, i));
    }

    return tmp;
}

set* set_fromarray(arrtype* a, int n) {
    int i;
    set* tmp = set_init();

    for (i = 0; i < n; i++) {
        /* Set insert already checks if value is in set */
        set_insert(tmp, a[i]);
    }

    return tmp;
}

/* ------- Basic Operations ------- */
void set_insert(set* s, arrtype l) {
    if (!s) {
        return;
    }

    if (!set_contains(s, l)) {
        arr_set(s->ua, s->sz, l);
        s->sz++;
    }
}

int set_size(set* s) {
    return s == NULL ? 0 : s->sz;
}

int set_contains(set* s, arrtype l) {
    int i;

    if (!s) {
        return 0;
    }

    for (i = 0; i < set_size(s); i++) {
        if (arr_get(s->ua, i) == l) {
            return 1;
        }
    }
    return 0;
}

void set_remove(set* s, arrtype l) {
    int i, j = 0;
    arrtype val;
    arr* tmp = arr_init();

    if (!s) {
        return;
    }

    if (set_contains(s, l)) {
        for (i = 0; i < set_size(s); i++) {
            val = arr_get(s->ua, i);
            if (val != l) {
                arr_set(tmp, j, val);
                j++;
            }
        }
        arr_free(&s->ua);
        s->ua = tmp;
        s->sz--;
    }
}

arrtype set_removeone(set* s) {
    arrtype val = arr_get(s->ua, s->sz - 1);
    set_remove(s, val);
    return val;
}

/* ------- Operations on 2 sets ------- */

set* set_union(set* s1, set* s2) {
    int i;
    set* tmp = set_init();

    for (i = 0; i < set_size(s1); i++) {
        set_insert(tmp, arr_get(s1->ua, i));
    }
    for (i = 0; i < set_size(s2); i++) {
        set_insert(tmp, arr_get(s2->ua, i));
    }

    return tmp;
}

set* set_intersection(set* s1, set* s2) {
    int i;
    set* tmp = set_init();

    set *minset, *maxset;
    arrtype val;

    minset = set_size(s1) > set_size(s2) ? s2 : s1;
    maxset = s1 == minset ? s2 : s1;

    for (i = 0; i < set_size(minset); i++) {
        val = arr_get(minset->ua, i);
        if (set_contains(maxset, val)) {
            set_insert(tmp, val);
        }
    }

    return tmp;
}

/* ------- Finish up ------- */
void set_free(set** s) {
    if (s) {
        arr_free(&(*s)->ua);
        free(*s);
        *s = NULL;
    }
}
