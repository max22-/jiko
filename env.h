#include "types.h"


void jk_define(jk_fiber_t *f, const char *w, jk_object_t body);
jk_object_t jk_lookup(jk_fiber_t *f, word_t w);