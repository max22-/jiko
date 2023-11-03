#include <stdlib.h>
#include <assert.h>
#include "parser.h"
#include "heap.h"

static void next(parser_t *p);

parser_t *parser_new(lexer_t *lex) {
    parser_t *res = malloc(sizeof(parser_t));
    assert(res);
    res->lexer = lex;
    res->look = NULL;
    next(res);
    return res;
}

void parser_free(parser_t *p) {
    lexer_free(p->lexer);
    if(p->look) token_free(p->look);
    free(p);
}

static void next(parser_t *p) {
    if(p->look)
        token_free(p->look);
    p->look = lexer_next(p->lexer);
}

static jk_object_t integer(parser_t *p) {
    jk_object_t res = jk_make_int(atoi(p->look->value));
    next(p);
    return res;
}

static jk_object_t string(parser_t *p) {
    jk_object_t res = jk_make_string(p->look->value);
    next(p);
    return res;
}

static jk_object_t word(parser_t *p) {
    jk_object_t res = jk_make_word(p->look->value);
    next(p);
    return res;
}

static jk_object_t quotation(parser_t *p) {
    jk_object_t res = JK_NIL;
    next(p); // we match the '['
    while(1) {
        switch(p->look->type) {
            case TOK_ERROR:
                jk_object_free(res);
                return jk_make_error(jk_make_string(p->look->value));
            case TOK_EOF:
                jk_object_free(res);
                return jk_make_error(jk_make_string("unexpected EOF inside quotation"));
            case TOK_CLOSE_BRACKET:
                goto parsed;
            default: {
                jk_object_t j = parser_parse(p);
                if(jk_get_type(j) == JK_ERROR) {
                    jk_object_free(res);
                    return j;
                }
                res = jk_append(res, j);
            }
        }
    }
    parsed:
    next(p); // we match the ']'
    return res; 
}


jk_object_t parser_parse(parser_t* p) {
    switch(p->look->type) {
    case TOK_INTEGER:
        return integer(p);
    case TOK_STRING:
        return string(p);
    case TOK_WORD:
        return word(p);
    case TOK_OPEN_BRACKET:
        return quotation(p);
    case TOK_CLOSE_BRACKET:
        return jk_make_error(jk_make_string("parse error: unexpected ']'"));
    case TOK_ERROR:
        return jk_make_error(jk_make_string(p->look->value));
    case TOK_EOF:
        return JK_EOF;
    default:
        return jk_make_error(jk_make_string("unreachable"));
    }
}

