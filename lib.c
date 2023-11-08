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
    JK_INT_CTYPE c = AS_INT(a) + AS_INT(b);
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
    JK_INT_CTYPE c = AS_INT(a) - AS_INT(b);
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
    JK_INT_CTYPE c = AS_INT(a) * AS_INT(b);
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
    JK_INT_CTYPE c = AS_INT(a) / AS_INT(b);
    jk_object_free(a);
    jk_object_free(b);
    jk_push(f, jk_make_int(c));
}

void mod(jk_fiber_t *f) {
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
    JK_INT_CTYPE c = AS_INT(a) % AS_INT(b);
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

void _true(jk_fiber_t *f) {
    jk_push(f, jk_make_bool(1));
}

void _false(jk_fiber_t *f) {
    jk_push(f, jk_make_bool(0));
}

void equal(jk_fiber_t *f) {
    jk_object_t a, b;
    if(!jk_pop_int(f, &b))
        return;
    if(!jk_pop_int(f, &a)) {
        jk_object_free(b);
        return;
    }
    jk_object_t j = jk_make_bool(AS_INT(a)==AS_INT(b));
    jk_push(f, j);
    jk_object_free(a);
    jk_object_free(b);
}

void ifte(jk_fiber_t *f) {
    jk_object_t cond, th, el;
    if(!jk_pop_quotation(f, &el))
        return;
    if(!jk_pop_quotation(f, &th)) {
        jk_object_free(el);
        return;
    }
    if(!jk_pop_bool(f, &cond)) {
        jk_object_free(th);
        jk_object_free(el);
        return;
    }
    if(AS_BOOL(cond)) {
        f->queue = jk_concat(th, f->queue);
        jk_object_free(el);
    } else {
        f->queue = jk_concat(el, f->queue);
        jk_object_free(th);
    }
    jk_object_free(cond);
}

void call(jk_fiber_t *f) {
    jk_object_t q;
    if(!jk_pop_quotation(f, &q))
        return;
    f->queue = jk_concat(q, f->queue);
}

void single_quote(jk_fiber_t *f) {
    jk_object_t j = jk_fiber_dequeue(f);
    if(j == JK_EOF) {
        jk_push(f, jk_make_error(jk_make_string("unexpected EOF after '")));
        return;
    }
    jk_push(f, j);
}

void def(jk_fiber_t *f) {
    jk_object_t name, body;
    if(!jk_pop_word(f, &name))
        return;
    if(!jk_pop(f, &body)) {
        jk_object_free(name);
        return;
    }
    jk_define(f, name, jk_make_pair(body, JK_NIL));
}

void defn(jk_fiber_t *f) {
    jk_object_t name, body;
    if(!jk_pop_word(f, &name))
        return;
    if(!jk_pop_quotation(f, &body)) {
        jk_object_free(name);
        return;
    }
    jk_define(f, name, jk_make_pair(body, jk_make_pair(jk_make_word_from_string("call"), JK_NIL)));
}

builtins_table_entry_t stdlib_builtins[] = {
    {"+", add},
    {"-", sub},
    {"*", mul},
    {"/", _div},
    {"%", mod},
    {"dup", dup},
    {"drop", drop},
    {"swap", swap},
    {"true", _true},
    {"false", _false},
    {"=", equal},
    {"ifte", ifte},
    {"call", call},
    {"'", single_quote},
    {"def", def},
    {"defn", defn},
    {NULL, NULL}
};

void register_lib(jk_fiber_t *f, builtins_table_entry_t *tbl) {
    int i = 0;
    while(tbl[i].name) {
        const char *name = tbl[i].name;
        void (*builtin)(jk_fiber_t*) = tbl[i].builtin;
        jk_define(f, jk_make_word_from_string(name), jk_make_pair(jk_make_builtin(builtin), JK_NIL));    
        i++;
    }
}