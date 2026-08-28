// FILENAME: test.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (lexer.h, lexer.c, string.c, test.h, vm.h)
// NEEDS: libkernel (vdev.h, vdev.c)
// Provides runtime implementation for the TEST statement in BASIC++.
//
// ---- Includes ----

#include <stdio.h>
#include <string.h>

#include "statements/debug/diagnostics/test.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "device/vdev.h"
#include "runtime/micro_lib_metadata.h"

BppError stmt_test_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_next(lex);
    char name_buf[128] = "Unit Test Suite";

    if (tok.type == TOK_STRING) {
        snprintf(name_buf, sizeof(name_buf), "%.*s", (int)tok.length, tok.start + 1);
        if (name_buf[0] && name_buf[strlen(name_buf) - 1] == '"') {
            name_buf[strlen(name_buf) - 1] = '\0';
        }
    } else if (tok.type == TOK_IDENT) {
        snprintf(name_buf, sizeof(name_buf), "%.*s", (int)tok.length, tok.start);
    }

    vm_set_in_test(vm, true, name_buf);

    VDevContext *vdev = vm_get_vdev(vm);
    vdev_printf(vdev, "=== TEST SUITE: %s ===\n", name_buf);

    return err;
}

BppError stmt_endtest_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    (void)lex;

    if (!vm_is_in_test(vm)) {
        err.code = 2; // Syntax Error
        err.message = "ENDTEST without matching TEST statement";
        return err;
    }

    int pass = 0, fail = 0, total = 0;
    vm_get_test_metrics(vm, &pass, &fail, &total);

    VDevContext *vdev = vm_get_vdev(vm);
    vdev_printf(vdev, "Test Suite Summary: %d passed, %d failed (Total %d assertions)\n\n",
                pass, fail, total);

    vm_set_in_test(vm, false, NULL);

    return err;
}

void stmt_test_register(void) {
    static const MicroLibMetadata meta = {
        .name = "TEST",
        .category = "Debug & Testing",
        .syntax = "TEST \"Suite Name\" | ENDTEST",
        .help_text = "Begins or ends a declarative unit test suite block and outputs assertion metrics.",
        .error_codes = "Error 2: Syntax Error"
    };
    microlib_register(&meta);
}
