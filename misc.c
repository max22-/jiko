#include <assert.h>
#include <stdlib.h>
#include <string.h>

char *strdup(const char *str) {
    char *res = (char*)malloc(strlen(str) + 1);
    assert(res && "strdup: malloc failed");
    strcpy(res, str);
    return res;
}

char *strndup(const char *str, size_t n) {
    size_t len;
    /* clang-format off */
    for (len = 0; len < n && str[len]; len++);
    /* clang-format on */
    char *res = (char*)malloc(len + 1);
    assert(res && "strndup: malloc failed");
    memcpy(res, str, len);
    res[len] = 0;
    return res;
}