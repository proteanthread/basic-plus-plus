// FILENAME: test.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (lexer.h, lexer.c, string.c, test.h, vm.h)
// NEEDS: libkernel (vdev.h, vdev.c)
// Provides runtime implementation for the TEST statement in BASIC++.
//
// ---- Includes ----

#include "runtime/format/snprintf.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

#include "statements/debug/diagnostics/test.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "device/vdev.h"
#include "runtime/language_descriptor.h"

static const LangDesc g_test_desc = {
    .name = "TEST",
    .category = "Debug & Testing",
    .syntax = "TEST \"suite_name\" [SUBCASE \"case_name\"]",
    .description = "Begins or ends a declarative unit test suite block and outputs assertion metrics.",
    .error_summary = "Error 2: Syntax Error",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_STATEMENT
};

BppError stmt_test_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    BppToken tok = lex_next(lex);
    char name_buf[128] = "Unit Test Suite";

    if (tok.type == TOK_STRING) {
        runtime_snprintf(name_buf, sizeof(name_buf), "%.*s", (int)tok.length, tok.start + 1);
        if (name_buf[0] && name_buf[runtime_strlen(name_buf) - 1] == '"') {
            name_buf[runtime_strlen(name_buf) - 1] = '\0';
        }
    } else if (tok.type == TOK_IDENT) {
        runtime_snprintf(name_buf, sizeof(name_buf), "%.*s", (int)tok.length, tok.start);
    }

    vm_set_in_test(vm, true, name_buf);

    VDevContext *vdev = vm_get_vdev(vm);
    vdev_printf(vdev, "=== TEST SUITE: %s ===\n", name_buf);

    return err;
}

BppError stmt_endtest_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
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
    lang_desc_register(&g_test_desc);
}
