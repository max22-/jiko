#include "eval.h"
#include "env.h"
#include "heap.h"
#include "types.h"
#include <assert.h>

void jk_fiber_enqueue(jk_fiber_t *f, jk_object_t j) {
    f->queue = jk_append(f->queue, j);
}

jk_object_t jk_fiber_dequeue(jk_fiber_t *f) {
    if (f->queue == JK_NIL)
        return JK_EOF;
    else {
        assert(jk_get_type(f->queue) == JK_QUOTATION);
        jk_object_t res = CAR(f->queue);
        f->queue = CDR(f->queue);
        return res;
    }
}

void jk_fiber_prepend(jk_fiber_t *f, jk_object_t j) {
    f->queue = jk_make_pair(j, f->queue);
}

void jk_push(jk_fiber_t *f, jk_object_t j) {
    f->stack = jk_make_pair(j, f->stack);
}

jk_object_t jk_pop(jk_fiber_t *f) {
    if (f->stack == JK_NIL)
        return jk_make_error(jk_make_string("stack underflow"));
    assert(jk_get_type(f->stack) == JK_QUOTATION);
    jk_object_t res = CAR(f->stack);
    f->stack = CDR(f->stack);
    return res;
}

int jk_error_raised(jk_fiber_t *f) {
    return f->stack != -1 && jk_get_type(CAR(f->stack)) == JK_ERROR;
}

void jk_fiber_eval(jk_fiber_t *f, size_t limit) {
    while (limit--) {
        if (jk_error_raised(f))
            return;
        jk_object_t j = jk_fiber_dequeue(f);
        switch (jk_get_type(j)) {
        case JK_EOF:
            goto loop_end;
        case JK_NIL:
        case JK_INT:
        case JK_BOOL:
        case JK_STRING:
        case JK_QUOTATION:
        case JK_FIBER:
        case JK_ERROR: // TODO: should we push it ??
            jk_push(f, j);
            break;
        case JK_BUILTIN:
            AS_BUILTIN(j)(f);
            jk_object_free(j);
            break;
        case JK_WORD: {
            jk_object_t body = jk_lookup(f, AS_WORD(j));
            if (body == JK_UNDEFINED) {
                jk_push(f, jk_make_error(jk_make_string("undefined word")));
                goto loop_end;
            } else {
                assert(jk_get_type(body) == JK_QUOTATION);
                jk_object_t body_clone = jk_object_clone(body);
                jk_object_t ji;
                for (ji = body_clone; CDR(ji) != JK_NIL; ji = CDR(ji)) {
                }
                CDR(ji) = f->queue;
                f->queue = body_clone;
            }
            jk_object_free(j);
            break;
        }
        }
        jk_fiber_print(f);
        printf("\n");
    }
loop_end:
    return;
}