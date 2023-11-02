#include <assert.h>
#include <stdlib.h>
#include <string.h>

char *strdup(const char *str) {
    char *res = malloc(strlen(str) + 1);
    assert(res && "strdup: malloc failed");
    strcpy(res, str);
    return res;
}

char *strndup(const char *str, size_t n) {
    size_t len;
    for (len = 0; len < n && str[len]; len++);
    char *res = malloc(len + 1);
    assert(res && "strndup: malloc failed");
    memcpy(res, str, len);
    res[len] = 0;
    return res;
}