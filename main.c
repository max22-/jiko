#include "jiko.h"
#include <stdio.h>
#include <stdlib.h>

void print(const char *s) { printf("%s", s); }

void jiko_panic(const char *msg) {
    printf("panic: %s\n", msg);
    jiko_cleanup();
    exit(1);
}

int main() {
    jiko_init();
    lexer_t *lex = lexer_new("1 2 3 [ a b c def ] \"abc\"");
    for (token_t *tok = lexer_next(lex);
         tok->type != TOK_EOF && tok->type != TOK_ERROR;
         tok = lexer_next(lex)) {
        token_print(tok, print);
        printf("\n");
    }
    jiko_cleanup();
}