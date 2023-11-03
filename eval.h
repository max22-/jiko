#include <stddef.h>
#include "types.h"

void jk_fiber_enqueue(jk_fiber_t *f, jk_object_t j);
void jk_push(jk_fiber_t *f, jk_object_t j) ;
jk_object_t jk_pop(jk_fiber_t *f);
int jk_error_raised(jk_fiber_t *f);
void jk_fiber_eval(jk_fiber_t *f, size_t limit);