#include <stdio.h>
#define JIKO_IMPLEMENTATION
#include "jiko.h"

void print(const char *s) { printf("%s", s); }

void jiko_panic(const char *msg) {
    printf("panic: %s\n", msg);
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
    register_lib(f);
    for (j = parser_parse(parser);
         jk_get_type(j) != JK_ERROR && jk_get_type(j) != JK_EOF;
         j = parser_parse(parser)) {
        jk_fiber_enqueue(f, j);
    }
    printf("evaluating...\n");
    jk_fiber_eval(f, 1000);
    jk_fiber_print(f);
    printf("\n");
    printf("freeing fiber\n");
    jk_fiber_free(f);
    printf("j=%d\n", j);
    parser_free(parser);
    jiko_cleanup();
}