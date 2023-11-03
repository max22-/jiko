#include "jiko.h"
#include "heap.h"
#include "word_table.h"

void jiko_init() {
    word_table_init(1024);
    heap_init(65536);
}

void jiko_cleanup() {
    heap_free();
    word_table_free();
}