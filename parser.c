#include "parser.h"
#include "heap.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    if (p->look)
        token_free(p->look);
    free(p);
}

static jk_object_t jk_gen_parse_error(parser_t *p, const char *msg) {
    char *err = malloc(strlen(msg) + 1 + 64);
    assert(err);
    sprintf(err, "%d:%d: %s", p->look->line, p->look->column, msg);
    jk_object_t res = jk_make_error(jk_make_string(err));
    free(err);
    return res;
}

static void next(parser_t *p) {
    if (p->look)
        token_free(p->look);
    p->look = lexer_next(p->lexer);
}

static jk_object_t integer(parser_t *p) {
    jk_object_t res = jk_make_int(atoi(p->look->value));
    next(p);
    return res;
}

static char *unescape_string(const char *str) {
    printf("unescaping %s\n", str);
    char *res = malloc(strlen(str) + 1);
    char *ptr = res;
    assert(*str == '"');
    str++;
    while (*str != '"') {
        assert(*str);
        if (*str == '\\') {
            str++;
            assert(*str);
            switch (*str) {
            case 'n':
                *ptr++ = '\n';
                break;
            case 'r':
                *ptr++ = '\r';
                break;
            case 't':
                *ptr++ = '\t';
                break;
            case '"':
                *ptr++ = '"';
                break;
            case '\\':
                *ptr++ = '\\';
                break;
            default:
                free(res);
                return NULL;
            }
        } else {
            *ptr++ = *str;
        }
        str++;
    }
    assert(*++str == 0);
    *ptr = 0;
    return res;
}

static jk_object_t string(parser_t *p) {
    char *str = unescape_string(p->look->value);
    if(!str)
        return jk_gen_parse_error(p, "failed to unescape string");
    jk_object_t res = jk_make_string(str);
    next(p);
    return res;
}

static jk_object_t word(parser_t *p) {
    jk_object_t res = jk_make_word_from_string(p->look->value);
    next(p);
    return res;
}

static jk_object_t quotation(parser_t *p) {
    jk_object_t res = JK_NIL;
    next(p); // we match the '['
    while (1) {
        switch (p->look->type) {
        case TOK_ERROR:
            jk_object_free(res);
            return jk_gen_parse_error(p, p->look->value);
        case TOK_EOF:
            jk_object_free(res);
            return jk_gen_parse_error(p, "unexpected EOF inside quotation");
        case TOK_CLOSE_BRACKET:
            goto parsed;
        default: {
            jk_object_t j = parser_parse(p);
            if (jk_get_type(j) == JK_ERROR) {
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

jk_object_t parser_parse(parser_t *p) {
    switch (p->look->type) {
    case TOK_INTEGER:
        return integer(p);
    case TOK_STRING:
        return string(p);
    case TOK_WORD:
        return word(p);
    case TOK_OPEN_BRACKET:
        return quotation(p);
    case TOK_CLOSE_BRACKET:
        return jk_gen_parse_error(p, "parse error: unexpected ']'");
    case TOK_ERROR:
        return jk_gen_parse_error(p, p->look->value);
    case TOK_EOF:
        return JK_EOF;
    default:
        return jk_gen_parse_error(p, "unreachable");
    }
}
