#include "eval.h"
#include "env.h"
#include "heap.h"
#include "io.h"
#include "types.h"
#include <assert.h>
#include <stdio.h>

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

int jk_raise_error(jk_fiber_t *f, const char *str) {
    jk_push(f, jk_make_error(jk_make_string(str)));
    return 0;
}

int jk_error_raised(jk_fiber_t *f) {
    return f->stack != -1 && jk_get_type(CAR(f->stack)) == JK_ERROR;
}

int jk_pop(jk_fiber_t *f, jk_object_t *res) {
    if (f->stack == JK_NIL)
        return jk_raise_error(f, "stack underflow");
    assert(jk_get_type(f->stack) == JK_QUOTATION);
    *res = CAR(f->stack);
    jk_object_t garbage = f->stack;
    f->stack = CDR(f->stack);
    CAR(garbage) = JK_NIL;
    CDR(garbage) = JK_NIL;
    jk_object_free(garbage);
    return 1;
}

#define MAKE_JK_POP(name, type_check, error_msg) \
    int jk_pop_ ## name(jk_fiber_t *f, jk_object_t *res) { \
        jk_object_t j; \
        if (!jk_pop(f, &j)) \
            return 0; \
        if (!(type_check)) { \
            jk_object_free(j); \
            return jk_raise_error(f, error_msg); \
        } \
        *res = j; \
        return 1;\
    }

MAKE_JK_POP(int, jk_get_type(j) == JK_INT, "expected integer")
MAKE_JK_POP(word, jk_get_type(j) == JK_WORD, "expected word")
MAKE_JK_POP(quotation, jk_get_type(j) == JK_QUOTATION || j == JK_NIL, "expected quotation")

void jk_fiber_eval(jk_fiber_t *f, size_t limit) {
    while (limit--) {
        if (jk_error_raised(f))
            return;
        jk_object_t j = jk_fiber_dequeue(f);
        switch (jk_get_type(j)) {
        case JK_UNDEFINED:
            assert(0 && "unreachable");
            break;
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
        jk_printf("\n");
    }
loop_end:
    return;
}