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
    const char *input =
        "1 2 3 + dup * swap [ a b [c d] def ] \"ab\\\"c\\\\\n\" dup + []";
    parser_t *parser = parser_new(input);
    
    jk_fiber_t *f = jk_fiber_new();
    while(1) {
        jk_parse_result_t pr = parser_parse(parser);
        switch(pr.type) {
            case JK_PARSE_OK:
                jk_fiber_enqueue(f, pr.result.j);
                break;
            case JK_PARSE_EOF_OK:
                jk_parse_result_free(pr);
                jk_printf("evaluating...\n");
                jk_fiber_eval(f, 1000);
                jk_fiber_print(f);
                jk_printf("\n");
                goto cleanup;
                break;
            case JK_PARSE_ERROR_EOF:
                jk_printf(pr.result.error_msg);
                jk_printf("\n");
                jk_parse_result_free(pr);
                goto cleanup;
                break;
            case JK_PARSE_ERROR_UNRECOVERABLE:
                jk_printf(pr.result.error_msg);
                jk_printf("\n");
                jk_parse_result_free(pr);
                goto cleanup;
                break;
        }
    }
    jk_printf("evaluating...\n");
    jk_fiber_eval(f, 1000);
    jk_fiber_print(f);
    

cleanup:
    jk_printf("cleanup...\n");
    jk_fiber_free(f);
    parser_free(parser);
    jiko_cleanup();
}