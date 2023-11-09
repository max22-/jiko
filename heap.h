#ifndef HEAP_H
#define HEAP_H

#include "types.h"

extern struct jk_object *heap;

void heap_init(size_t s);
void heap_free();
jk_object_t jk_object_alloc();
void jk_object_free(jk_object_t j);
size_t heap_free_objects_count();
jk_object_t jk_object_clone(jk_object_t j);

#endif