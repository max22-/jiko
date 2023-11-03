#include <assert.h>
#include "eval.h"
#include "heap.h"

void jk_fiber_enqueue(jk_object_t f, jk_object_t j) {
    AS_FIBER(f)->queue = jk_append(AS_FIBER(f)->queue, j);
}

jk_object_t jk_fiber_dequeue(jk_object_t f) {
    if(AS_FIBER(f)->queue == JK_NIL)
        return JK_EOF;
    else {
        assert(jk_get_type(AS_FIBER(f)->queue) == JK_QUOTATION);
        jk_object_t res = CAR(AS_FIBER(f)->queue);
        AS_FIBER(f)->queue = CDR(AS_FIBER(f)->queue);
        return res;
    }
}

static void push(jk_object_t f, jk_object_t j) {
    AS_FIBER(f)->stack = jk_make_pair(j, AS_FIBER(f)->stack);
}

static jk_object_t pop(jk_object_t f) {
    if(AS_FIBER(f)->stack == JK_NIL)
        return jk_make_error(jk_make_string("stack underflow"));
    assert(jk_get_type(AS_FIBER(f)->stack) == JK_QUOTATION);
    jk_object_t res = CAR(AS_FIBER(f)->stack);
    AS_FIBER(f)->stack = CDR(AS_FIBER(f)->stack);
    return res;
}

static jk_object_t pop_int(jk_object_t f) {
    jk_object_t res = pop(f);
    if(jk_get_type(res) == JK_ERROR)
        return res;
    if(jk_get_type(res) != JK_INT) {
        jk_object_free(res);
        return jk_make_error(jk_make_string("expected integer"));
    }
    return res;
}

void jk_fiber_eval(jk_object_t f, size_t limit) {
    assert(jk_get_type(f) == JK_FIBER);
    while(limit--) {
        jk_object_t j = jk_fiber_dequeue(f);
        switch(jk_get_type(j)) {
            case JK_EOF:
                goto loop_end;
            case JK_NIL:
            case JK_INT:
            case JK_BOOL:
            case JK_STRING:
            case JK_QUOTATION:
            case JK_FIBER:
            case JK_ERROR: // TODO: should we push it ??
                push(f, j);
                break;
            case JK_BUILTIN:
                AS_BUILTIN(j)(AS_FIBER(f));
                jk_object_free(j);
                break;
            
            case JK_WORD:
                // TODO: lookup
                jk_object_free(j);
                break;
        }
    }
    loop_end:
    return;
}