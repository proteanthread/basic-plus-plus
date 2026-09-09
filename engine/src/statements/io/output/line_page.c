// FILENAME: line_page.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (exec_dispatch.c)
// NEEDS: libcore, libengine, libkernel
// Provides runtime implementation for LINE and PAGE output statements (JOSS / RAND P-2922).
//
// ---- Includes ----

#include "statements/io/output/line_page.h"
#include "vm/vm.h"
#include "device/vdev.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

BppError stmt_line_output_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    (void)lex;

    vdev_write_string(vm, "\n", 1);
    return err;
}

BppError stmt_page_output_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    (void)lex;

    vdev_write_string(vm, "\f", 1);
    return err;
}
