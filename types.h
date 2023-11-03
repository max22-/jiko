#ifndef TYPES_H
#define TYPES_H

#include "word_table.h" /* for word_t type declaration */

typedef enum jk_type {
    JK_UNDEFINED = -3,
    JK_EOF = -2,
    JK_NIL = -1, /* the empty list */
    JK_INT,
    JK_BOOL,
    JK_STRING,
    JK_WORD,
    JK_QUOTATION,
    JK_BUILTIN,
    JK_FIBER,
    JK_ERROR,
} jk_type;

struct jk_fiber;

typedef int jk_object_t;

struct jk_object {
    jk_type type;
    union value {
        int as_int;
        int as_bool;
        const char *as_string;
        word_t as_word;
        void (*as_builtin)(struct jk_fiber *);
        struct jk_fiber *as_fiber;
        jk_object_t as_error;
        struct pair {
            int car, cdr;
        } as_pair;
    } value;
};

typedef struct jk_fiber {
    jk_object_t stack, queue, env_stack;
} jk_fiber_t;

jk_fiber_t *jk_fiber_new();
void jk_fiber_free(jk_fiber_t *f);

extern struct jk_object *heap;

jk_type jk_get_type(jk_object_t);
void jk_set_type(jk_object_t, jk_type);

#define AS_INT(j) (heap[(j)].value.as_int)
#define AS_BOOL(j) (heap[(j)].value.as_bool)
#define AS_STRING(j) (heap[(j)].value.as_string)
#define AS_WORD(j) (heap[(j)].value.as_word)
#define AS_QUOTATION(j) (heap[(j)].value.as_pair)
#define CAR(j) (heap[(j)].value.as_pair.car)
#define CDR(j) (heap[(j)].value.as_pair.cdr)
#define CAAR(j) (CAR(CAR(j)))
#define CDAR(j) (CDR(CAR(j)))
#define AS_BUILTIN(j) (heap[(j)].value.as_builtin)
#define AS_FIBER(j) (heap[(j)].value.as_fiber)
#define AS_ERROR(j) (heap[(j)].value.as_error)

jk_object_t jk_make_int(int i);
jk_object_t jk_make_bool(int b);
jk_object_t jk_make_string(const char *str);
jk_object_t jk_make_word(word_t w);
jk_object_t jk_make_word_from_string(const char *w);
jk_object_t jk_make_pair(jk_object_t car, jk_object_t cdr);
jk_object_t jk_append(jk_object_t q, jk_object_t j);
jk_object_t jk_make_builtin(void (*f)(struct jk_fiber *));
jk_object_t jk_make_fiber();
jk_object_t jk_make_error(jk_object_t j);

void jk_print(jk_object_t);
void jk_fiber_print(jk_fiber_t *f);

#endif