#include <stddef.h>
#include "types.h"

void jk_fiber_enqueue(jk_object_t f, jk_object_t j);
void jk_fiber_eval(jk_object_t f, size_t limit);