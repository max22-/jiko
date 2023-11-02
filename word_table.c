#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "word_table.h"
#include "misc.h"
#include <assert.h>

static const char **word_table = NULL;
static size_t table_size = 0;
static size_t counter = 0;

void word_table_init(size_t s) {
    table_size = s;
    word_table = malloc(sizeof(const char*) * s);
    assert(word_table && "init_word_table: malloc failed");
    for(size_t i = 0; i < s; i++)
        word_table[i] = NULL;
}

void word_table_free() {
    for(size_t i = 0; i < counter; i++) {
        free((void*)word_table[i]);
    }
    free(word_table);
}

static void enlarge_table() {
    size_t new_size = table_size * 2;
    word_table = realloc(word_table, sizeof(const char*) * new_size);
    if(!word_table)
        jiko_panic("enlarge_table: realloc failed");
    table_size = new_size;
}

static word_t new_word(const char *str) {
    if(counter >= table_size)
        enlarge_table();
    word_table[counter] = strdup(str);
    word_t res = counter;
    counter++;
    return res;
}

word_t word_from_string(const char *str)  {
    for(size_t i = 0; i < counter; i++) {
        assert(word_table[i] && "word_table[i] shouldn't be null when i < counter");
        if(!strcmp(str, word_table[i]))
            return i;
    }
    /* not found: we allocate a new word */
    return new_word(str);
}

const char *word_to_string(word_t word) {
    if(word < counter)
        return word_table[word];
    else
        return NULL;
}