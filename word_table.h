#ifndef WORD_TABLE_H
#define WORD_TABLE_H

typedef unsigned int word_t;

void word_table_init(size_t s);
void word_table_free();
word_t word_from_string(const char *str);
const char *word_to_string(word_t);

#endif