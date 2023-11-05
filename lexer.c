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

token_t *token_clone(token_t* t) {
    return token_new(
            t->type, 
            t->value,
            t->line,
            t->column);
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
lexer_t *lexer_new() {
    lexer_t *res = (lexer_t *)malloc(sizeof(lexer_t));
    assert(res && "lexer_new: malloc failed");
    res->text = NULL;
    res->text_len = 0;
    res->start_line = res->start_column = 1;
    res->pos_line = res->pos_column = 1;
    res->start = res->pos = 0;
    return res;
}

void lexer_free(lexer_t *lex) {
    free((void *)lex->text);
    free(lex);
}

void lexer_set_text(lexer_t *lex, const char *text) {
    if(lex->text != NULL)
        free((void*)lex->text);
    lex->text = strdup(text);
    lex->text_len = strlen(lex->text);
    lex->start_column = 1;
    lex->pos_column = 1;
    lex->start = lex->pos = 0;
}

void lexer_add(lexer_t *lex, const char *text) {
    lex->text = realloc((char*)lex->text, lex->text_len + strlen(text) + 1);
    assert(lex->text);
    strcat((char*)lex->text, text);
    lex->text_len = strlen(lex->text);
}

static token_t *lexer_open_bracket(lexer_t *lex) {
    token_t *res =
        token_new(TOK_OPEN_BRACKET, "[", lex->start_line, lex->start_column);
    next_char(lex);
    update_start(lex);
    return res;
}

static token_t *lexer_close_bracket(lexer_t *lex) {
    token_t *res =
        token_new(TOK_CLOSE_BRACKET, "]", lex->start_line, lex->start_column);
    next_char(lex);
    update_start(lex);
    return res;
}

static token_t *lexer_integer(lexer_t *lex) {
    while (lex->pos < lex->text_len && isdigit(lex->text[lex->pos]))
        next_char(lex);
    char *strval = strndup(&lex->text[lex->start], lex->pos - lex->start);
    token_t *res =
        token_new(TOK_INTEGER, strval, lex->start_line, lex->start_column);
    update_start(lex);
    free(strval);
    return res;
}

static token_t *lexer_string(lexer_t *lex) {
    next_char(lex); /* we match the '"' */
    while (1) {
        if (lex->pos >= lex->text_len)
            return token_new(TOK_ERROR, "Unexpected end of line in string literal",
                             lex->start_line, lex->start_column);
        if (lex->text[lex->pos] == '"')
            break;
        if (lex->text[lex->pos] == '\\') {
            next_char(lex);
            if (lex->pos >= lex->text_len)
                return token_new(TOK_ERROR, "Unexpected end of line in string literal",
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

static token_t *lexer_word(lexer_t *lex) {
    while (1) {
        if (lex->pos >= lex->text_len)
            break;
        char c = lex->text[lex->pos];
        if (isspace(c) || c == '[' || c == ']' || c == '"')
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
        return lexer_open_bracket(lex);
    else if (c == ']')
        return lexer_close_bracket(lex);
    else if (isdigit(c))
        return lexer_integer(lex);
    else if (c == '"')
        return lexer_string(lex);
    else
        return lexer_word(lex);
}