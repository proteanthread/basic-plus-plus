// FILENAME: menu.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_stmts.c)
// NEEDS: libcore (language_descriptor.h, strings.h)
// NEEDS: libengine (eval.h, menu.h)
// NEEDS: libkernel (vdev.h)
// Provides runtime implementation for the MENU statement in BASIC++.

#include "statements/ui/widgets/menu.h"
#include "runtime/language_descriptor.h"
#include "eval/eval.h"
#include "device/vdev.h"
#include "runtime/strings.h"
#include "runtime/format/snprintf.h"
#include "runtime/string/strops.h"

static const LangDesc g_menu_desc = {
    .name = "MENU",
    .category = "User Interface",
    .syntax = "MENU [menu_title$,] item_caption$",
    .description = "Renders a menu item UI widget or menu bar header.",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_STATEMENT
};

void stmt_menu_register(void) {
    lang_desc_register(&g_menu_desc);
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

BppError stmt_menu_handler(VMContext *vm, LexerContext *lex) {
    BppError err = {0};
    BValue args[4];
    int count = parse_widget_args(vm, lex, args, 4, &err);
    if (err.code != 0 || count == 0) return err;

    char title[64] = "File";
    char item[128] = "Item";

    if (count == 1) {
        if (args[0].type == VAL_STRING && args[0].as.string) {
            const char *s = str_data(args[0].as.string);
            if (s) runtime_strncpy(item, s, sizeof(item) - 1);
        }
    } else {
        if (args[0].type == VAL_STRING && args[0].as.string) {
            const char *s = str_data(args[0].as.string);
            if (s) runtime_strncpy(title, s, sizeof(title) - 1);
        }
        if (args[1].type == VAL_STRING && args[1].as.string) {
            const char *s = str_data(args[1].as.string);
            if (s) runtime_strncpy(item, s, sizeof(item) - 1);
        }
    }

    for (int i = 0; i < count; i++) {
        if (args[i].type == VAL_STRING && args[i].as.string) {
            str_release(vm_get_str(vm), args[i].as.string);
        }
    }

    VDevContext *vdev = vm_get_vdev(vm);
    if (vdev) {
        char buf[256];
        runtime_snprintf(buf, sizeof(buf), "| %s: %s | ", title, item);
        vdev_puts(vdev, buf);
    }
    return err;
}
