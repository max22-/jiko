#include "jiko.h"
#include <stdio.h>
#include <stdlib.h>

int (*jk_printf)(const char *, ...) = printf;

void jiko_panic(const char *msg) {
    jk_printf("panic: %s\n", msg);
    jiko_cleanup();
    exit(1);
}

int main() {
    jiko_init();
    lexer_t *lex =
        lexer_new("1 2 3 + dup * swap [ a b [c d] def ] \"abc\" dup + []");
    parser_t *parser = parser_new(lex);
    jk_object_t j;
    jk_fiber_t *f = jk_fiber_new();
    for (j = parser_parse(parser); jk_get_type(j) != JK_EOF;
         j = parser_parse(parser)) {
        if (jk_get_type(j) == JK_ERROR) {
            printf("%s\n", AS_STRING(AS_ERROR(j)));
            goto cleanup;
        }
        jk_fiber_enqueue(f, j);
    }
    jk_printf("evaluating...\n");
    jk_fiber_eval(f, 1000);
    jk_fiber_print(f);
    jk_printf("\n");

cleanup:
    jk_printf("freeing fiber\n");
    jk_fiber_free(f);
    jk_printf("j=%d\n", j);
    parser_free(parser);
    jiko_cleanup();
}