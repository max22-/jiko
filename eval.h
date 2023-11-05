#include "types.h"
#include <stddef.h>

void jk_fiber_enqueue(jk_fiber_t *f, jk_object_t j);
jk_object_t jk_fiber_dequeue(jk_fiber_t *f);
void jk_fiber_eval(jk_fiber_t *f, size_t limit);
int jk_raise_error(jk_fiber_t *f, const char *str);
void jk_push(jk_fiber_t *f, jk_object_t j);

/* Pop with error handling
   return value :
   1 : success
   0 : failure
   result stored in *res in case of success
*/
int jk_pop(jk_fiber_t *f, jk_object_t *res);
int jk_pop_int(jk_fiber_t *f, jk_object_t *res);
int jk_pop_word(jk_fiber_t *f, jk_object_t *res);