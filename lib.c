#include "lib.h"
#include "env.h"
#include "eval.h"
#include "heap.h"
#include <assert.h>

void add(jk_fiber_t *f) {
    jk_object_t a, b;
    if (!jk_pop_int(f, &b))
        return;
    if (!jk_pop_int(f, &a)) {
        jk_object_free(b);
        return;
    }
    int c = AS_INT(a) + AS_INT(b);
    jk_object_free(a);
    jk_object_free(b);
    jk_push(f, jk_make_int(c));
}

void sub(jk_fiber_t *f) {
    jk_object_t a, b;
    if (!jk_pop_int(f, &b))
        return;
    if (!jk_pop_int(f, &a)) {
        jk_object_free(b);
        return;
    }
    int c = AS_INT(a) - AS_INT(b);
    jk_object_free(a);
    jk_object_free(b);
    jk_push(f, jk_make_int(c));
}

void mul(jk_fiber_t *f) {
    jk_object_t a, b;
    if (!jk_pop_int(f, &b))
        return;
    if (!jk_pop_int(f, &a)) {
        jk_object_free(b);
        return;
    }
    int c = AS_INT(a) * AS_INT(b);
    jk_object_free(a);
    jk_object_free(b);
    jk_push(f, jk_make_int(c));
}

void _div(jk_fiber_t *f) {
    jk_object_t a, b;
    if (!jk_pop_int(f, &b))
        return;
    if (!jk_pop_int(f, &a)) {
        jk_object_free(b);
        return;
    }
    if (AS_INT(b) == 0) {
        jk_object_free(b);
        jk_object_free(a);
        jk_raise_error(f, "division by zero");
        return;
    }
    int c = AS_INT(a) + AS_INT(b);
    jk_object_free(a);
    jk_object_free(b);
    jk_push(f, jk_make_int(c));
}

void dup(jk_fiber_t *f) {
    jk_object_t j;
    if (!jk_pop(f, &j))
        return;
    jk_object_t jc = jk_object_clone(j);
    jk_push(f, j);
    jk_push(f, jc);
}

void drop(jk_fiber_t *f) {
    jk_object_t j;
    if (jk_pop(f, &j))
        jk_object_free(j);
}

void swap(jk_fiber_t *f) {
    jk_object_t a, b;
    if (!jk_pop(f, &b))
        return;
    if (!jk_pop(f, &a)) {
        jk_object_free(b);
        return;
    }
    jk_push(f, b);
    jk_push(f, a);
}

void register_lib(jk_fiber_t *f) {
    jk_define(f, "+", jk_make_pair(jk_make_builtin(add), JK_NIL));
    jk_define(f, "-", jk_make_pair(jk_make_builtin(sub), JK_NIL));
    jk_define(f, "*", jk_make_pair(jk_make_builtin(mul), JK_NIL));
    jk_define(f, "/", jk_make_pair(jk_make_builtin(_div), JK_NIL));
    jk_define(f, "dup", jk_make_pair(jk_make_builtin(dup), JK_NIL));
    jk_define(f, "drop", jk_make_pair(jk_make_builtin(drop), JK_NIL));
    jk_define(f, "swap", jk_make_pair(jk_make_builtin(swap), JK_NIL));
}