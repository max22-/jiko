#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include "types.h"
#include "misc.h"
#include "word_table.h"

struct jk_object *heap = NULL;
static size_t heap_size = 0; 
jk_object_t free_list_head = -1;

void heap_init(size_t s) {
    heap = malloc(sizeof(struct jk_object) * s);
    heap_size = s;
    for(size_t i = 0; i < s; i++) {
        heap[i].type = JK_QUOTATION;
        CAR(i) = -1;
        CDR(i) = free_list_head;        
        free_list_head = i;
    }
}

void heap_free() {
    free(heap);
}

jk_object_t jk_object_alloc() {
    if(free_list_head == -1)
        jiko_panic("heap full"); // TODO: make the heap grow ?
    jk_object_t j = free_list_head;
    free_list_head = CDR(free_list_head);
    return j;
}

void jk_object_free(jk_object_t j) {
    if(j < 0) return;
    switch(TYPE(j)) {
    case JK_INT:
        break;
    case JK_BOOL:
        break;
    case JK_STRING:
        free((void*)AS_STRING(j));
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
        jiko_panic("not implemented yet"); // TODO
        break;
    case JK_ERROR:
        jk_object_free(AS_ERROR(j));
        break;
    
    }
    TYPE(j) = JK_QUOTATION;
    CAR(j) = -1;
    CDR(j) = free_list_head;
    free_list_head = j;
}

/* Constructors **************************************************************/

jk_object_t jk_make_int(int i) {
    jk_object_t res = jk_object_alloc();
    TYPE(res) = JK_INT;
    AS_INT(res) = i;
    return res;
}

jk_object_t jk_make_bool(int b) {
    jk_object_t res = jk_object_alloc();
    TYPE(res) = JK_BOOL;
    AS_BOOL(res) = b;
    return res;
}

jk_object_t jk_make_string(const char* str) {
    jk_object_t res = jk_object_alloc();
    TYPE(res) = JK_STRING;
    AS_STRING(res) = strdup(str);
    return res;
}

jk_object_t jk_make_word(const char *w) {
    jk_object_t res = jk_object_alloc();
    TYPE(res) = JK_WORD;
    AS_WORD(res) = word_from_string(w);
    return res;
}

jk_object_t jk_make_pair(jk_object_t car, jk_object_t cdr) {
    jk_object_t res = jk_object_alloc();
    TYPE(res) = JK_QUOTATION;
    CAR(res) = car;
    CDR(res) = cdr;
    return res;
}

jk_object_t jk_append(jk_object_t q, jk_object_t j) {
    if(q == -1)
        return jk_make_pair(j, -1);
    jk_object_t qi;
    for(qi = q; CDR(qi) != -1; qi = CDR(qi));
    CDR(qi) = jk_make_pair(j, -1);
    return q;
}

jk_object_t jk_make_builtin(void (*f)(struct jk_fiber *)) {
    jk_object_t res = jk_object_alloc();
    TYPE(res) = JK_BUILTIN;
    AS_BUILTIN(res) = f;
    return res;
}

jk_object_t jk_make_fiber() {
    jk_object_t res = jk_object_alloc();
    #warning not implemented yet
}

jk_object_t jk_make_error(jk_object_t j) {
    jk_object_t res = jk_object_alloc();
    TYPE(res) = JK_ERROR;
    AS_ERROR(res) = j;
    return res;
}

// TODO: transform it to a jk_to_string function ?
void jk_print(jk_object_t j) {
    switch(TYPE(j)) {
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
    case JK_QUOTATION: {
        printf("[");
        for(jk_object_t ji = j; ji != -1; ji = CDR(ji)) {
            jk_print(CAR(ji));
            if(CDR(ji) != -1)
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
    }
}