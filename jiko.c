#include "jiko.h"
#include "word_table.h"
#include "heap.h"

void jiko_init() {
    word_table_init(1024);
    heap_init(65536);
}

void jiko_cleanup() {
    heap_free();
    word_table_free();
}