#include "lexer.h"
#include "types.h"

enum jk_parse_result_type {
    JK_PARSE_OK,
    JK_PARSE_EOF_OK,
    JK_PARSE_ERROR_EOF,
    JK_PARSE_ERROR_UNRECOVERABLE
};

typedef struct {
    enum jk_parse_result_type type;
    union result {
        jk_object_t j;
        const char *error_msg;
    } result;
} jk_parse_result_t;

typedef struct parser {
    lexer_t *lexer;
    token_t *look;
} parser_t;

parser_t *parser_new(const char *input);
void parser_free(parser_t *);
jk_parse_result_t parser_parse(parser_t *);
void jk_parse_result_free(jk_parse_result_t);
