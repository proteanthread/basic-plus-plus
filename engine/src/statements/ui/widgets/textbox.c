// FILENAME: textbox.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_stmts.c)
// NEEDS: libcore (language_descriptor.h, strings.h)
// NEEDS: libengine (eval.h, textbox.h)
// NEEDS: libkernel (vdev.h)
// Provides runtime implementation for the TEXTBOX statement in BASIC++.

#include "statements/ui/widgets/textbox.h"
#include "runtime/language_descriptor.h"
#include "eval/eval.h"
#include "device/vdev.h"
#include "runtime/strings.h"
#include "runtime/format/snprintf.h"
#include "runtime/string/strops.h"

static const LangDesc g_textbox_desc = {
    .name = "TEXTBOX",
    .category = "User Interface",
    .syntax = "TEXTBOX text$ [, col%, row%, width%, height%]",
    .description = "Renders an editable text box UI widget.",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_STATEMENT
};

void stmt_textbox_register(void) {
    lang_desc_register(&g_textbox_desc);
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

BppError stmt_textbox_handler(VMContext *vm, LexerContext *lex) {
    BppError err = {0};
    BValue args[6];
    int count = parse_widget_args(vm, lex, args, 6, &err);
    if (err.code != 0 || count == 0) return err;

    char text[128] = "";
    int txt_idx = count - 1;
    for (int i = 0; i < count; i++) {
        if (args[i].type == VAL_STRING) { txt_idx = i; break; }
    }
    if (args[txt_idx].type == VAL_STRING && args[txt_idx].as.string) {
        const char *s = str_data(args[txt_idx].as.string);
        if (s) runtime_strncpy(text, s, sizeof(text) - 1);
    }

    for (int i = 0; i < count; i++) {
        if (args[i].type == VAL_STRING && args[i].as.string) {
            str_release(vm_get_str(vm), args[i].as.string);
        }
    }

    VDevContext *vdev = vm_get_vdev(vm);
    if (vdev) {
        char buf[256];
        runtime_snprintf(buf, sizeof(buf), "[ %-20s ] ", text);
        vdev_puts(vdev, buf);
    }
    return err;
}
