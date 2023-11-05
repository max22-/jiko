#ifndef LEXER_H
#define LEXER_H

#include <stddef.h>

/* Token ******************************************************************* */

typedef enum token_type {
    TOK_OPEN_BRACKET,
    TOK_CLOSE_BRACKET,
    TOK_INTEGER,
    TOK_STRING,
    TOK_WORD,
    TOK_ERROR,
    TOK_EOF,
} token_type;

typedef struct token {
    enum token_type type;
    const char *value;
    unsigned int line, column;
} token_t;

token_t *token_new(token_type type, const char *value, unsigned int line,
                   unsigned int column);
token_t *token_clone(token_t*);
void token_free(token_t *token);
void token_print(token_t *token, void (*printer)(const char *));

/* Lexer ******************************************************************* */

typedef struct lexer {
    const char *text;
    size_t text_len;
    unsigned int start, pos;
    unsigned int start_line, start_column, pos_line, pos_column;
} lexer_t;

lexer_t *lexer_new();
void lexer_free(lexer_t *lex);
void lexer_set_text(lexer_t *lex, const char *text) ;
void lexer_add(lexer_t *lex, const char *text);
token_t *lexer_next(lexer_t *lexer);

#endif