#include <assert.h>
#include "env.h"

/* env = [
    [
        [$3 . [a b c d]]
        [$1 . [1 2 3]]
    ]
    [
        [$5 . [[$3 $2] $4]]
        [$2 . []]
    ]
    
]

"type" : env = [[[ word . [] ]]]

words = {
    $1 "+"
    $2 "*"
    $3 "dup"
    $4 "call"
    $5 "square"
}

*/

/* Looks up in only one level of the environment stack */
static jk_object_t jk_lookup_env(jk_object_t env, word_t w) {
    for(jk_object_t ji = env; ji != JK_NIL; ji = CDR(ji)) {
        assert(CAR(ji) != JK_NIL);
        if(AS_WORD(CAAR(ji)) == w)
            return CDAR(ji);
    }
    return JK_UNDEFINED;
}

void jk_define(jk_fiber_t *f, const char *w, jk_object_t body) {
    jk_object_t env_stack = f->env_stack;
    assert(env_stack != JK_NIL);
    jk_object_t top_level = CAR(env_stack);
    /* TODO: implement old definition replacement */
    /*
    jk_object_t prev = jk_lookup_env(top_level, word_from_string(w));
    if(prev == JK_UNDEFINED) {
    */
        jk_object_t entry = jk_make_pair(jk_make_word_from_string(w), body);
        top_level = jk_make_pair(entry, top_level);
        CAR(env_stack) = top_level;
    /*
    }
    */
}

jk_object_t jk_lookup(jk_fiber_t *f, word_t w) {
    jk_object_t env_stack = f->env_stack;
    assert(env_stack != JK_NIL);
    for(jk_object_t ji = env_stack; ji != JK_NIL; ji = CDR(ji)) {
        jk_object_t res = jk_lookup_env(CAR(ji), w);
        if(res != JK_UNDEFINED)
            return res;
    }
    return JK_UNDEFINED;
}