#include "parser.h"
#include "heap.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void next(parser_t *p);

parser_t *parser_new(const char *input) {
    parser_t *res = malloc(sizeof(parser_t));
    assert(res);
    res->lexer = lexer_new(input);
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

static jk_parse_result_t jk_gen_parse_error(parser_t *p,
                                            enum jk_parse_result_type type,
                                            const char *msg) {
    jk_parse_result_t res;
    char *err = malloc(strlen(msg) + 1 + 64);
    assert(err);
    sprintf(err, "%d:%d: %s", p->look->line, p->look->column, msg);
    res.type = type;
    res.result.error_msg = err;
    return res;
}

static void next(parser_t *p) {
    if (p->look)
        token_free(p->look);
    p->look = lexer_next(p->lexer);
}

static jk_parse_result_t integer(parser_t *p) {
    jk_parse_result_t res = {.type = JK_PARSE_OK,
                             .result.j = jk_make_int(atoi(p->look->value))};
    next(p);
    return res;
}

static char *jk_unescape_string(const char *str) {
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

static jk_parse_result_t string(parser_t *p) {
    char *str = jk_unescape_string(p->look->value);
    if (!str)
        return jk_gen_parse_error(p, JK_PARSE_ERROR_UNRECOVERABLE,
                                  "failed to unescape string");
    jk_object_t j = jk_make_string(str);
    jk_parse_result_t res = {.type = JK_PARSE_OK, .result.j = j};
    free(str);
    next(p);
    return res;
}

static jk_parse_result_t word(parser_t *p) {
    jk_parse_result_t res = {.type = JK_PARSE_OK,
                             .result.j =
                                 jk_make_word_from_string(p->look->value)};
    next(p);
    return res;
}

static jk_parse_result_t quotation(parser_t *p) {
    jk_object_t q = JK_NIL;
    next(p); // we match the '['
    while (1) {
        switch (p->look->type) {
        case TOK_ERROR:
            jk_object_free(q);
            return jk_gen_parse_error(p, JK_PARSE_ERROR_UNRECOVERABLE,
                                      p->look->value);
        case TOK_EOF:
            jk_object_free(q);
            return jk_gen_parse_error(p, JK_PARSE_ERROR_EOF,
                                      "unexpected EOF inside quotation");
        case TOK_CLOSE_BRACKET:
            goto parsed;
        default: {
            jk_parse_result_t pr = parser_parse(p);
            if (pr.type != JK_PARSE_OK) {
                jk_object_free(q);
                return pr;
            }
            q = jk_append(q, pr.result.j);
        }
        }
    }
parsed:
    next(p); // we match the ']'
    jk_parse_result_t res = {.type = JK_PARSE_OK, .result.j = q};
    return res;
}

jk_parse_result_t parser_parse(parser_t *p) {
    jk_parse_result_t res;
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
        return jk_gen_parse_error(p, JK_PARSE_ERROR_UNRECOVERABLE,
                                  "parse error: unexpected ']'");
    case TOK_ERROR:
        return jk_gen_parse_error(p, JK_PARSE_ERROR_UNRECOVERABLE,
                                  p->look->value);
    case TOK_EOF:
        res.type = JK_PARSE_EOF_OK;
        res.result.error_msg = NULL;
        return res;
    default:
        return jk_gen_parse_error(p, JK_PARSE_ERROR_UNRECOVERABLE,
                                  "unreachable");
    }
}

void jk_parse_result_free(jk_parse_result_t pr) {
    if (pr.type != JK_PARSE_OK)
        free((void *)pr.result.error_msg);
}