// FILENAME: button.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_stmts.c)
// NEEDS: libcore (language_descriptor.h, strings.h)
// NEEDS: libengine (button.h, eval.h)
// NEEDS: libkernel (vdev.h)
// Provides runtime implementation for the BUTTON statement in BASIC++.

#include "statements/ui/widgets/button.h"
#include "runtime/language_descriptor.h"
#include "eval/eval.h"
#include "device/vdev.h"
#include "runtime/strings.h"
#include "runtime/format/snprintf.h"
#include "runtime/string/strops.h"

static const LangDesc g_button_desc = {
    .name = "BUTTON",
    .category = "User Interface",
    .syntax = "BUTTON caption$ [, col%, row%, width%, height%]",
    .description = "Renders a command button UI widget with caption.",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_STATEMENT
};

void stmt_button_register(void) {
    lang_desc_register(&g_button_desc);
}

static int parse_widget_args(VMContext *vm, LexerContext *lex, BValue *args, int max_args, BppError *err) {
    int count = 0;
    while (count < max_args) {
        BppToken tok = lex_peek(lex);
        if (tok.type == TOK_EOL || tok.type == TOK_EOF) break;
        args[count] = eval_expression(vm, lex, err);
        if (err->code != 0) break;
        count++;
        tok = lex_peek(lex);
        if (tok.type == TOK_COMMA) {
            lex_next(lex);
        } else {
            break;
        }
    }
    return count;
}

BppError stmt_button_handler(VMContext *vm, LexerContext *lex) {
    BppError err = {0};
    BValue args[6];
    int count = parse_widget_args(vm, lex, args, 6, &err);
    if (err.code != 0 || count == 0) return err;

    char caption[128] = "OK";
    int cap_idx = count - 1;
    for (int i = 0; i < count; i++) {
        if (args[i].type == VAL_STRING) { cap_idx = i; break; }
    }
    if (args[cap_idx].type == VAL_STRING && args[cap_idx].as.string) {
        const char *s = str_data(args[cap_idx].as.string);
        if (s) runtime_strncpy(caption, s, sizeof(caption) - 1);
    } else {
        runtime_snprintf(caption, sizeof(caption), "%g", args[cap_idx].as.number);
    }

    for (int i = 0; i < count; i++) {
        if (args[i].type == VAL_STRING && args[i].as.string) {
            str_release(vm_get_str(vm), args[i].as.string);
        }
    }

    VDevContext *vdev = vm_get_vdev(vm);
    if (vdev) {
        char buf[256];
        runtime_snprintf(buf, sizeof(buf), "[ < %s > ] ", caption);
        vdev_puts(vdev, buf);
    }
    return err;
}
