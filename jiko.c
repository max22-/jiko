#include "jiko.h"
#include "word_table.h"

void jiko_init() {
    word_table_init(1024);
}

void jiko_cleanup() {
    word_table_free();
}