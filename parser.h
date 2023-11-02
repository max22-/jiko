#include "lexer.h"
#include "types.h"

typedef struct parser {
    lexer_t *lexer;
    token_t *look;
} parser_t;

parser_t *parser_new(lexer_t *);
void parser_free(parser_t*);
jk_object_t parser_parse(parser_t*);
