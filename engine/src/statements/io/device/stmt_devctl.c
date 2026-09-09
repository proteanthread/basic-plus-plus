// FILENAME: stmt_devctl.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, exec_dispatch.c
// NEEDS: libkernel (types.h, lexer.h, errors.h, vdev.h), libcore (eval.h, strings.h, language_descriptor.h)
// Implementation for DEVCTL statement micro-library.
//
// ---- Includes ----

#include "statements/io/device/stmt_devctl.h"
#include "device/vdev.h"
#include "eval/eval.h"
#include "runtime/strings.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

static const LangDesc g_devctl_desc = {
    .name = "DEVCTL",
    .category = "System & Hardware",
    .syntax = "DEVCTL dev_spec$, cmd [, aux1 [, aux2 [, data$]]]",
    .description = "Directly executes a low-level IOCTL or control command on a device without requiring OPEN.",
    .error_summary = "Error 2: Syntax Error, Error 5: Illegal Function Call, Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SYSTEM,
    .type = FEATURE_STATEMENT
};

void stmt_devctl_register(void) {
    lang_desc_register(&g_devctl_desc);
}

static int parse_cmd_code(const BValue *val) {
    if (!val) return 0;
    if (val->type == VAL_NUMBER || val->type == VAL_INTEGER) {
        return (int)val->as.number;
    }
    if (val->type == VAL_STRING && val->as.string) {
        const char *s = str_data(val->as.string);
        if (!s) return 0;
        if (runtime_strcasecmp(s, "RESET") == 0) return 0;
        if (runtime_strcasecmp(s, "STATUS") == 0) return 13;
        if (runtime_strcasecmp(s, "SET_BAUD") == 0) return 2;
        if (runtime_strcasecmp(s, "MOTOR_ON") == 0) return 43;
        if (runtime_strcasecmp(s, "MOTOR_OFF") == 0) return 44;
        if (runtime_strcasecmp(s, "REWIND") == 0) return 45;
        if (runtime_strcasecmp(s, "DRAWTO") == 0) return 40;
        if (runtime_strcasecmp(s, "FILL") == 0) return 41;
        if (runtime_strcasecmp(s, "ATTACH_HARDWARE") == 0) return 200;
        if (runtime_strcasecmp(s, "PIN_MODE") == 0) return 200;
    }
    return 0;
}

BppError stmt_devctl_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    if (!vm || !lex) {
        err.code = 5; err.message = "Null context in DEVCTL";
        return err;
    }

    // 1. Device Spec String
    BValue dev_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (dev_val.type != VAL_STRING) {
        err.code = ERR_TYPE_MISMATCH;
        return err;
    }

    BppToken tok = lex_peek(lex);
    if (tok.type != TOK_COMMA) {
        if (dev_val.as.string) str_release(vm_get_str(vm), dev_val.as.string);
        err.code = ERR_SYNTAX;
        err.message = "Expected comma after device specification in DEVCTL";
        return err;
    }
    lex_next(lex);

    // 2. Command (numeric or string)
    BValue cmd_val = eval_expression(vm, lex, &err);
    if (err.code != 0) {
        if (dev_val.as.string) str_release(vm_get_str(vm), dev_val.as.string);
        return err;
    }

    int cmd = parse_cmd_code(&cmd_val);
    int aux1 = 0;
    int aux2 = 0;

    tok = lex_peek(lex);
    if (tok.type == TOK_COMMA) {
        lex_next(lex);
        BValue a1_val = eval_expression(vm, lex, &err);
        if (err.code != 0) goto cleanup;
        if (a1_val.type == VAL_NUMBER || a1_val.type == VAL_INTEGER) aux1 = (int)a1_val.as.number;
        if (a1_val.type == VAL_STRING && a1_val.as.string) str_release(vm_get_str(vm), a1_val.as.string);

        tok = lex_peek(lex);
        if (tok.type == TOK_COMMA) {
            lex_next(lex);
            BValue a2_val = eval_expression(vm, lex, &err);
            if (err.code != 0) goto cleanup;
            if (a2_val.type == VAL_NUMBER || a2_val.type == VAL_INTEGER) aux2 = (int)a2_val.as.number;
            if (a2_val.type == VAL_STRING && a2_val.as.string) str_release(vm_get_str(vm), a2_val.as.string);
        }
    }

    const char *dev_name = dev_val.as.string ? str_data(dev_val.as.string) : "";
    VDevContext *vctx = vm_get_vdev(vm);
    VDev *dev = vctx ? vdev_get(vctx, dev_name) : NULL;
    if (!dev) {
        err.code = ERR_ILLEGAL_FUNCTION_CALL;
        err.message = "Device not found or reserved";
        goto cleanup;
    }

    int args[4] = {aux1, aux2, 0, 0};
    vdev_ioctl(dev, cmd, args);

cleanup:
    if (dev_val.as.string) str_release(vm_get_str(vm), dev_val.as.string);
    if (cmd_val.type == VAL_STRING && cmd_val.as.string) str_release(vm_get_str(vm), cmd_val.as.string);
    return err;
}
