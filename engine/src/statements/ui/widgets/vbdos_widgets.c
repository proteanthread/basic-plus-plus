// FILENAME: vbdos_widgets.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (exec_internal.h)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (eval.h, eval.c, string.c, vbdos_widgets.h)
// NEEDS: libkernel (vdev.h, vdev.c)
// Provides runtime implementation for the VBDOS_WIDGETS statement in BASIC++.
//
// ---- Includes ----

#include "statements/ui/widgets/vbdos_widgets.h"
#include "eval/eval.h"
#include "device/vdev.h"
#include "runtime/strings.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>
#include <stdio.h>

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
    BppError err; memset(&err, 0, sizeof(err));
    BValue args[6];
    int count = parse_widget_args(vm, lex, args, 6, &err);
    if (err.code != 0 || count == 0) return err;

    char caption[128] = "OK";
    // Find string argument if any, or use last argument
    int cap_idx = count - 1;
    for (int i = 0; i < count; i++) {
        if (args[i].type == VAL_STRING) { cap_idx = i; break; }
    }
    if (args[cap_idx].type == VAL_STRING && args[cap_idx].as.string) {
        const char *s = str_data(args[cap_idx].as.string);
        if (s) strncpy(caption, s, sizeof(caption) - 1);
    } else {
        snprintf(caption, sizeof(caption), "%g", args[cap_idx].as.number);
    }

    for (int i = 0; i < count; i++) {
        if (args[i].type == VAL_STRING && args[i].as.string) {
            str_release(vm_get_str(vm), args[i].as.string);
        }
    }

    VDevContext *vdev = vm_get_vdev(vm);
    if (vdev) {
        char buf[256];
        snprintf(buf, sizeof(buf), "[ < %s > ] ", caption);
        vdev_puts(vdev, buf);
    }
    return err;
}

BppError stmt_menu_handler(VMContext *vm, LexerContext *lex) {
    BppError err; memset(&err, 0, sizeof(err));
    BValue args[4];
    int count = parse_widget_args(vm, lex, args, 4, &err);
    if (err.code != 0 || count == 0) return err;

    char title[64] = "File";
    char item[128] = "Item";

    if (count == 1) {
        if (args[0].type == VAL_STRING && args[0].as.string) {
            const char *s = str_data(args[0].as.string);
            if (s) strncpy(item, s, sizeof(item) - 1);
        }
    } else {
        if (args[0].type == VAL_STRING && args[0].as.string) {
            const char *s = str_data(args[0].as.string);
            if (s) strncpy(title, s, sizeof(title) - 1);
        }
        if (args[1].type == VAL_STRING && args[1].as.string) {
            const char *s = str_data(args[1].as.string);
            if (s) strncpy(item, s, sizeof(item) - 1);
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
        snprintf(buf, sizeof(buf), "| %s: %s | ", title, item);
        vdev_puts(vdev, buf);
    }
    return err;
}

BppError stmt_textbox_handler(VMContext *vm, LexerContext *lex) {
    BppError err; memset(&err, 0, sizeof(err));
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
        if (s) strncpy(text, s, sizeof(text) - 1);
    }

    for (int i = 0; i < count; i++) {
        if (args[i].type == VAL_STRING && args[i].as.string) {
            str_release(vm_get_str(vm), args[i].as.string);
        }
    }

    VDevContext *vdev = vm_get_vdev(vm);
    if (vdev) {
        char buf[256];
        snprintf(buf, sizeof(buf), "[ %-20s ] ", text);
        vdev_puts(vdev, buf);
    }
    return err;
}

BppError stmt_listbox_handler(VMContext *vm, LexerContext *lex) {
    BppError err; memset(&err, 0, sizeof(err));
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
        if (s) strncpy(text, s, sizeof(text) - 1);
    }

    for (int i = 0; i < count; i++) {
        if (args[i].type == VAL_STRING && args[i].as.string) {
            str_release(vm_get_str(vm), args[i].as.string);
        }
    }

    VDevContext *vdev = vm_get_vdev(vm);
    if (vdev) {
        char buf[256];
        snprintf(buf, sizeof(buf), "[* %s ] ", text);
        vdev_puts(vdev, buf);
    }
    return err;
}

void stmt_vbdos_widgets_register(void) {
    static const MicroLibMetadata meta_btn = { .name = "BUTTON", .category = "User Interface", .syntax = "BUTTON caption$" };
    static const MicroLibMetadata meta_menu = { .name = "MENU", .category = "User Interface", .syntax = "MENU item$" };
    static const MicroLibMetadata meta_tb = { .name = "TEXTBOX", .category = "User Interface", .syntax = "TEXTBOX text$" };
    static const MicroLibMetadata meta_lb = { .name = "LISTBOX", .category = "User Interface", .syntax = "LISTBOX text$" };
    microlib_register(&meta_btn);
    microlib_register(&meta_menu);
    microlib_register(&meta_tb);
    microlib_register(&meta_lb);
}
