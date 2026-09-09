// FILENAME: cont.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (cont.h, eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h)
// NEEDS: libkernel (security.h, security.c, vdev.h, vdev.c)
// Provides runtime implementation for the CONT statement in BASIC++.
//
// ---- Includes ----

#include "statements/program/cont.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "device/vdev.h"
#include "security/security.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

static const LangDesc g_cont_desc = {
    .name = "CONT",
    .category = "Program Mgmt & Editing",
    .syntax = "CONT",
    .description = "Resumes program execution after a break or STOP statement.",
    .error_summary = "Error 2: Syntax Error, Error 17: Can't Continue",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_STATEMENT
};

BppError stmt_cont_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    (void)lex;
    vm_set_single_step(vm, false);
    return err;
}

void stmt_cont_register(void) {
    lang_desc_register(&g_cont_desc);
}

