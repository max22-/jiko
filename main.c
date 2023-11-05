#include "jiko.h"
#include <stdio.h>
#include <stdlib.h>

int repl = 1;

int (*jk_printf)(const char *, ...) = printf;

void jiko_panic(const char *msg) {
    jk_printf("panic: %s\n", msg);
    jiko_cleanup();
    exit(1);
}

static int input(char *buffer, size_t size, int cont) {
    *buffer = 0;
    if(feof(stdin))
        return 0;
    if(repl) {
        if(!cont)
            printf("> ");
        else
            printf("... ");
    }
    fflush(stdout);
    fgets(buffer, size, stdin);
    return 1;
}

int main() {
    jiko_init();
    /* const char *input =
        "1 2 3 + dup * swap [ a b [c d] def ] \"ab\\\"c\\\\\n\" dup [] [";
    */
    char input_buffer[1024];
    parser_t *parser = parser_new();

    input(input_buffer, sizeof(input_buffer), 0);
    parser_set_text(parser, input_buffer);
    //parser_set_text(parser, input);

    jk_fiber_t *f = jk_fiber_new();
    while (1) {
        jk_parse_result_t pr = parser_parse(parser);
        switch (pr.type) {
        case JK_PARSE_OK:
            jk_parse_result_free(pr);
            jk_fiber_enqueue(f, pr.result.j);
            break;
        case JK_PARSE_EOF_OK:
            jk_parse_result_free(pr);
            jk_fiber_eval(f, 1000);
            jk_fiber_print(f);
            jk_printf("\n");
            if(!repl)
                goto cleanup;
            if(!input(input_buffer, sizeof(input_buffer), 0))
                goto cleanup;
            parser_set_text(parser, input_buffer);
            break;
        case JK_PARSE_ERROR_EOF:
            if(!repl) {
                jk_printf(pr.result.error_msg);
                jk_printf("\n");
                jk_parse_result_free(pr);
                goto cleanup;
            } else {
                if(!input(input_buffer, sizeof(input_buffer), 1)) {
                    jk_printf(pr.result.error_msg);
                    jk_printf("\n");
                    jk_parse_result_free(pr);
                    goto cleanup;
                }
                jk_parse_result_free(pr);
                parser_add(parser, input_buffer);
            }
            break;
        case JK_PARSE_ERROR_UNRECOVERABLE:
            jk_printf(pr.result.error_msg);
            jk_printf("\n");
            jk_parse_result_free(pr);
            goto cleanup;
            break;
        }
    }
    jk_fiber_eval(f, 1000);
    jk_fiber_print(f);

cleanup:
    jk_printf("cleanup...\n");
    jk_fiber_free(f);
    parser_free(parser);
    jiko_cleanup();
}