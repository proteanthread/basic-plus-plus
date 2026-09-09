// FILENAME: stmt_slot_io.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, exec_dispatch.c
// NEEDS: libkernel (types.h, lexer.h, errors.h, vdev.h), libcore (eval.h, strings.h, language_descriptor.h)
// Implementation for Apple II PR# and IN# statement micro-library.
//
// ---- Includes ----

#include "statements/io/device/stmt_slot_io.h"
#include "device/vdev.h"
#include "eval/eval.h"
#include "runtime/strings.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

static const LangDesc g_pr_desc = {
    .name = "PR#",
    .category = "System & Hardware",
    .syntax = "PR# slot_number",
    .description = "Redirects text and character output to Apple II peripheral expansion slot 0..7.",
    .error_summary = "Error 2: Syntax Error, Error 5: Illegal Function Call",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SYSTEM,
    .type = FEATURE_STATEMENT
};

static const LangDesc g_in_desc = {
    .name = "IN#",
    .category = "System & Hardware",
    .syntax = "IN# slot_number",
    .description = "Redirects keyboard and character input to Apple II peripheral expansion slot 0..7.",
    .error_summary = "Error 2: Syntax Error, Error 5: Illegal Function Call",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SYSTEM,
    .type = FEATURE_STATEMENT
};

void stmt_slot_io_register(void) {
    lang_desc_register(&g_pr_desc);

    lang_desc_register(&g_in_desc);
}

BppError stmt_pr_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    if (!vm || !lex) {
        err.code = 5; err.message = "Null context in PR#";
        return err;
    }

    // Optional #
    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_HASH) {
        lex_next(lex);
    }

    BValue slot_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (slot_val.type != VAL_NUMBER && slot_val.type != VAL_INTEGER) {
        if (slot_val.type == VAL_STRING && slot_val.as.string) str_release(vm_get_str(vm), slot_val.as.string);
        err.code = ERR_TYPE_MISMATCH;
        return err;
    }
    int slot = (int)slot_val.as.number;
    if (slot < 0 || slot > 7) {
        err.code = ERR_ILLEGAL_FUNCTION_CALL;
        err.message = "Slot number out of range (0..7)";
        return err;
    }

    VDevContext *ctx = vm_get_vdev(vm);
    if (ctx) {
        switch (slot) {
            case 0: vdev_alias_set(ctx, "PRN:", "CON:"); break;
            case 1: vdev_alias_set(ctx, "PRN:", "LPT1:"); break;
            case 2: vdev_alias_set(ctx, "PRN:", "COM1:"); break;
            case 3: vdev_alias_set(ctx, "PRN:", "SCN1:"); break;
            case 6: vdev_alias_set(ctx, "PRN:", "DSK1:"); break;
            default: break;
        }
    }
    return err;
}

BppError stmt_in_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    if (!vm || !lex) {
        err.code = 5; err.message = "Null context in IN#";
        return err;
    }

    // Optional #
    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_HASH) {
        lex_next(lex);
    }

    BValue slot_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (slot_val.type != VAL_NUMBER && slot_val.type != VAL_INTEGER) {
        if (slot_val.type == VAL_STRING && slot_val.as.string) str_release(vm_get_str(vm), slot_val.as.string);
        err.code = ERR_TYPE_MISMATCH;
        return err;
    }
    int slot = (int)slot_val.as.number;
    if (slot < 0 || slot > 7) {
        err.code = ERR_ILLEGAL_FUNCTION_CALL;
        err.message = "Slot number out of range (0..7)";
        return err;
    }

    VDevContext *ctx = vm_get_vdev(vm);
    if (ctx) {
        switch (slot) {
            case 0: vdev_alias_set(ctx, "KYBD:", "CON:"); break;
            case 2: vdev_alias_set(ctx, "KYBD:", "COM1:"); break;
            default: break;
        }
    }
    return err;
}
