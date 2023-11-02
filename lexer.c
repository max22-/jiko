#include "lexer.h"
#include "misc.h"
#include <assert.h>
#include <ctype.h> /* for isspace */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Token *********************************************************************/

token_t *token_new(token_type type, const char *value, unsigned int line,
                   unsigned int column) {
    token_t *res = malloc(sizeof(token_t));
    assert(res && "malloc failed");
    res->type = type;
    res->value = strdup(value);
    res->line = line;
    res->column = column;
    return res;
}

void token_free(token_t *token) {
    free((void *)token->value);
    free(token);
}

/* TODO: remove when the parser is implemented ? */
void token_print(token_t *token, void (*printer)(const char *)) {
    char buf_int[32]; /* enough to hold 64bit integers */
    printer("<");
    sprintf(buf_int, "%d", token->line);
    printer(buf_int);
    printer(":");
    sprintf(buf_int, "%d", token->column);
    printer(buf_int);
    printer(": \"");
    printer(token->value);
    printer("\" (");
    sprintf(buf_int, "%d", token->type);
    printer(buf_int);
    printer(")>");
}

/*****************************************************************************/

/* Lexer *********************************************************************/

static void next_char(lexer_t *lex) {
    if (lex->text[lex->pos] == '\n') {
        lex->pos_line++;
        lex->pos_column = 1;
    } else {
        lex->pos_column++;
    }
    lex->pos++;
}

static void update_start(lexer_t *lex) {
    lex->start_line = lex->pos_line;
    lex->start_column = lex->pos_column;
    lex->start = lex->pos;
}

static void skip_space(lexer_t *lex) {
    while (lex->pos < lex->text_len && isspace(lex->text[lex->pos]))
        next_char(lex);
    update_start(lex);
}

/* Duplicates text */
lexer_t *lexer_new(const char *text) {
    lexer_t *res = (lexer_t *)malloc(sizeof(lexer_t));
    assert(res && "lexer_new: malloc failed");
    res->text = strdup(text);
    res->text_len = strlen(text);
    res->start_line = res->start_column = 1;
    res->pos_line = res->pos_column = 1;
    res->start = res->pos = 0;
    return res;
}

void lexer_free(lexer_t *lex) {
    free((void*)lex->text);
    free(lex);
}

static token_t *open_bracket(lexer_t *lex) {
    token_t *res =
        token_new(TOK_OPEN_BRACKET, "[", lex->start_line, lex->start_column);
    next_char(lex);
    update_start(lex);
    return res;
}

static token_t *close_bracket(lexer_t *lex) {
    token_t *res =
        token_new(TOK_CLOSE_BRACKET, "]", lex->start_line, lex->start_column);
    next_char(lex);
    update_start(lex);
    return res;
}

static token_t *integer(lexer_t *lex) {
    while (lex->pos < lex->text_len && isdigit(lex->text[lex->pos]))
        next_char(lex);
    char *strval = strndup(&lex->text[lex->start], lex->pos - lex->start);
    token_t *res =
        token_new(TOK_INTEGER, strval, lex->start_line, lex->start_column);
    update_start(lex);
    free(strval);
    return res;
}

static token_t *string(lexer_t *lex) {
    next_char(lex); /* we match the '"' */
    while (1) {
        if (lex->pos >= lex->text_len)
            return token_new(TOK_ERROR, "Unexpected EOF in string literal",
                             lex->start_line, lex->start_column);
        if (lex->text[lex->pos] == '"')
            break;
        if (lex->text[lex->pos] == '\\') {
            next_char(lex);
            if (lex->pos >= lex->text_len)
                return token_new(TOK_ERROR, "Unexpected EOF in string literal",
                                 lex->start_line, lex->start_column);
            switch (lex->text[lex->pos]) {
            case 'n':
            case 'r':
            case 't':
            case '"':
            case '\\':
                break;
            default:
                return token_new(TOK_ERROR, "Invalid escaped character",
                                 lex->start_line, lex->start_column);
            }
        }
        next_char(lex);
    }
    assert(lex->text[lex->pos] == '"'); /* should always be the case */
    next_char(lex);
    char *strval = strndup(&lex->text[lex->start], lex->pos - lex->start);
    token_t *res =
        token_new(TOK_STRING, strval, lex->start_line, lex->start_column);
    update_start(lex);
    free(strval);
    return res;
}

static token_t *word(lexer_t *lex) {
    while (1) {
        if (lex->pos >= lex->text_len)
            break;
        char c = lex->text[lex->pos];
        if (isspace(c) || c == '[' || c == ']')
            break;
        next_char(lex);
    }
    char *strval = strndup(&lex->text[lex->start], lex->pos - lex->start);
    token_t *res =
        token_new(TOK_WORD, strval, lex->start_line, lex->start_column);
    lex->start = lex->pos;
    free(strval);
    return res;
}

token_t *lexer_next(lexer_t *lex) {
    skip_space(lex);
    if (lex->pos >= lex->text_len)
        return token_new(TOK_EOF, "", lex->start_line, lex->start_column);
    char c = lex->text[lex->pos];
    if (c == '[')
        return open_bracket(lex);
    else if (c == ']')
        return close_bracket(lex);
    else if (isdigit(c))
        return integer(lex);
    else if (c == '"')
        return string(lex);
    else
        return word(lex);
}