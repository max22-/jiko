#include "types.h"

typedef struct {
    const char *name;
    void (*builtin)(jk_fiber_t*);
} builtins_table_entry_t;

void register_lib(jk_fiber_t *f, builtins_table_entry_t *tbl);

extern builtins_table_entry_t stdlib_builtins[];