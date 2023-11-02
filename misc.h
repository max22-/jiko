#include <stddef.h>

char *strdup(const char *);
char *strndup(const char *, size_t);
extern void jiko_panic(const char *msg); /* provided by the user */