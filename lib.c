#include <assert.h>
#include "lib.h"
#include "env.h"
#include "eval.h"
#include "heap.h"

#define JK_POP_CHECK(f, x)  \
    do { \
        x = jk_pop(f); \
        if(jk_get_type(x) == JK_ERROR) { \
            jk_push(f, x); \
            return; \
        } \
    } while(0)

#define JK_POP_CHECK_TYPE(f, x, type) \
    do { \
        JK_POP_CHECK(f, x); \
        if(jk_get_type(x) != type) { \
            jk_object_free(x); \
            jk_push(f, jk_make_error(jk_make_string("expected " #type))); \
            return; \
        } \
    } while(0)

void add(jk_fiber_t *f) {
    jk_object_t a, b;
    JK_POP_CHECK_TYPE(f, b, JK_INT);
    JK_POP_CHECK_TYPE(f, a, JK_INT);
    int c = AS_INT(a) + AS_INT(b);
    jk_object_free(a);
    jk_object_free(b);
    jk_push(f, jk_make_int(c));
}

void mul(jk_fiber_t *f) {
    jk_object_t a, b;
    JK_POP_CHECK_TYPE(f, b, JK_INT);
    JK_POP_CHECK_TYPE(f, a, JK_INT);
    int c = AS_INT(a) * AS_INT(b);
    jk_object_free(a);
    jk_object_free(b);
    jk_push(f, jk_make_int(c));
}


void dup(jk_fiber_t *f) {
    jk_object_t j;
    JK_POP_CHECK(f, j);
    jk_object_t jc = jk_object_clone(j);
    jk_push(f, j);
    jk_push(f, jc);
}

void drop(jk_fiber_t *f) {
    jk_object_free(jk_pop(f));
}

void swap(jk_fiber_t *f) {
    jk_object_t a, b;
    JK_POP_CHECK(f, b);
    JK_POP_CHECK(f, a);
    jk_push(f, b);
    jk_push(f, a);
}

void register_lib(jk_fiber_t *f) {
    jk_define(f, "+", jk_make_pair(jk_make_builtin(add), JK_NIL));
    jk_define(f, "*", jk_make_pair(jk_make_builtin(mul), JK_NIL));
    jk_define(f, "dup", jk_make_pair(jk_make_builtin(dup), JK_NIL));
    jk_define(f, "drop", jk_make_pair(jk_make_builtin(drop), JK_NIL));
    jk_define(f, "swap", jk_make_pair(jk_make_builtin(swap), JK_NIL));
}