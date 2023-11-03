#include "misc.h"
#include "types.h"
#include "word_table.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

struct jk_object *heap = NULL;
static size_t heap_size = 0;
jk_object_t free_list_head = JK_NIL;

void heap_init(size_t s) {
    heap = malloc(sizeof(struct jk_object) * s);
    heap_size = s;
    for (size_t i = 0; i < s; i++) {
        heap[i].type = JK_QUOTATION;
        CAR(i) = JK_NIL;
        CDR(i) = free_list_head;
        free_list_head = i;
    }
}

void heap_free() { free(heap); }

jk_object_t jk_object_alloc() {
    if (free_list_head == JK_NIL)
        jiko_panic("heap full"); // TODO: make the heap grow ?
    jk_object_t j = free_list_head;
    free_list_head = CDR(free_list_head);
    return j;
}

void jk_object_free(jk_object_t j) {
    if (j < 0)
        return;
    switch (jk_get_type(j)) {
    case JK_EOF:
        break;
    case JK_NIL:
        break;
    case JK_INT:
        break;
    case JK_BOOL:
        break;
    case JK_STRING:
        free((void *)AS_STRING(j));
        break;
    case JK_WORD:
        break;
    case JK_QUOTATION:
        jk_object_free(CAR(j));
        jk_object_free(CDR(j));
        break;
    case JK_BUILTIN:
        break;
    case JK_FIBER:
        jk_object_free(AS_FIBER(j)->stack);
        jk_object_free(AS_FIBER(j)->queue);
        jk_object_free(AS_FIBER(j)->env);
        break;
    case JK_ERROR:
        jk_object_free(AS_ERROR(j));
        break;
    }
    jk_set_type(j, JK_QUOTATION);
    CAR(j) = JK_NIL;
    CDR(j) = free_list_head;
    free_list_head = j;
}

void jk_set_type(jk_object_t j, jk_type t) {
    if (j >= 0)
        heap[j].type = t;
    else {
        assert(j == t);
    }
}

jk_type jk_get_type(jk_object_t j) {
    if (j >= 0)
        return heap[j].type;
    else
        return j;
}

/* Constructors **************************************************************/

jk_object_t jk_make_int(int i) {
    jk_object_t res = jk_object_alloc();
    jk_set_type(res, JK_INT);
    AS_INT(res) = i;
    return res;
}

jk_object_t jk_make_bool(int b) {
    jk_object_t res = jk_object_alloc();
    jk_set_type(res, JK_BOOL);
    AS_BOOL(res) = b;
    return res;
}

jk_object_t jk_make_string(const char *str) {
    jk_object_t res = jk_object_alloc();
    jk_set_type(res, JK_STRING);
    AS_STRING(res) = strdup(str);
    return res;
}

jk_object_t jk_make_word(const char *w) {
    jk_object_t res = jk_object_alloc();
    jk_set_type(res, JK_WORD);
    AS_WORD(res) = word_from_string(w);
    return res;
}

jk_object_t jk_make_pair(jk_object_t car, jk_object_t cdr) {
    jk_object_t res = jk_object_alloc();
    jk_set_type(res, JK_QUOTATION);
    CAR(res) = car;
    CDR(res) = cdr;
    return res;
}

jk_object_t jk_append(jk_object_t q, jk_object_t j) {
    if (q == JK_NIL)
        return jk_make_pair(j, JK_NIL);
    jk_object_t qi;
    for (qi = q; CDR(qi) != JK_NIL; qi = CDR(qi))
        ;
    CDR(qi) = jk_make_pair(j, JK_NIL);
    return q;
}

jk_object_t jk_make_builtin(void (*f)(jk_fiber_t *)) {
    jk_object_t res = jk_object_alloc();
    jk_set_type(res, JK_BUILTIN);
    AS_BUILTIN(res) = f;
    return res;
}

jk_object_t jk_make_fiber(jk_fiber_t *f) {
    jk_object_t res = jk_object_alloc();
    jk_set_type(res, JK_FIBER);
    AS_FIBER(res) = f;
    return res;
}

jk_object_t jk_make_error(jk_object_t j) {
    jk_object_t res = jk_object_alloc();
    jk_set_type(res, JK_ERROR);
    AS_ERROR(res) = j;
    return res;
}

// TODO: transform it to a jk_to_string function ?
void jk_print(jk_object_t j) {
    switch (jk_get_type(j)) {
    case JK_INT:
        printf("%d", AS_INT(j));
        break;
    case JK_BOOL:
        printf("%s", AS_BOOL(j) ? "true" : "false");
        break;
    case JK_STRING:
        printf("\"%s\"", AS_STRING(j));
        break;
    case JK_WORD:
        printf("%s", word_to_string(AS_WORD(j)));
        break;
    case JK_NIL:
        printf("[]");
        break;
    case JK_QUOTATION: {
        printf("[");
        for (jk_object_t ji = j; ji != JK_NIL; ji = CDR(ji)) {
            jk_print(CAR(ji));
            if (CDR(ji) != JK_NIL)
                printf(" ");
        }
        printf("]");
        break;
    }
    case JK_BUILTIN:
        printf("<builtin 0x%lx>", (intptr_t)AS_BUILTIN(j));
        break;
    case JK_FIBER:
        printf("<fiber 0x%lx>", (intptr_t)AS_FIBER(j));
        break;
    case JK_ERROR:
        printf("<error ");
        jk_print(AS_ERROR(j));
        printf(">");
        break;
    case JK_EOF:
        break;
    }
}

void jk_print_fiber(jk_object_t j) {
    assert(jk_get_type(j) == JK_FIBER);
    jk_print(AS_FIBER(j)->stack);
    printf(" : ");
    jk_print(AS_FIBER(j)->queue);
}