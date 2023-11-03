#include "eval.h"

void jk_fiber_enqueue(jk_object_t f, jk_object_t j) {
    AS_FIBER(f)->queue = jk_append(AS_FIBER(f)->queue, j);
}