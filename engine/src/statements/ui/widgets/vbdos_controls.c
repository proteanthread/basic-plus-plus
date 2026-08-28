// FILENAME: vbdos_controls.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (exec_internal.h)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (eval.h, eval.c, string.c, vbdos_controls.h)
// NEEDS: libkernel (vdev.h, vdev.c)
// Provides runtime implementation for the VBDOS_CONTROLS statement in BASIC++.
//
// ---- Includes ----

#include "statements/ui/widgets/vbdos_controls.h"
#include "eval/eval.h"
#include "device/vdev.h"
#include "runtime/strings.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>
#include <stdio.h>

static int parse_ctrl_args(VMContext *vm, LexerContext *lex, BValue *args, int max_args, BppError *err) {
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

BppError stmt_checkbox_handler(VMContext *vm, LexerContext *lex) {
    BppError err; memset(&err, 0, sizeof(err));
    BValue args[6];
    int count = parse_ctrl_args(vm, lex, args, 6, &err);
    if (err.code != 0 || count == 0) return err;

    char caption[128] = "Check";
    int checked = 0;

    int str_idx = -1;
    for (int i = 0; i < count; i++) {
        if (args[i].type == VAL_STRING) { str_idx = i; break; }
    }

    if (str_idx >= 0 && args[str_idx].as.string) {
        const char *s = str_data(args[str_idx].as.string);
        if (s) strncpy(caption, s, sizeof(caption) - 1);
        if (str_idx + 1 < count && args[str_idx + 1].type == VAL_NUMBER) {
            checked = (args[str_idx + 1].as.number != 0.0);
        }
    } else {
        snprintf(caption, sizeof(caption), "%g", args[count - 1].as.number);
    }

    for (int i = 0; i < count; i++) {
        if (args[i].type == VAL_STRING && args[i].as.string) {
            str_release(vm_get_str(vm), args[i].as.string);
        }
    }

    VDevContext *vdev = vm_get_vdev(vm);
    if (vdev) {
        char buf[256];
        snprintf(buf, sizeof(buf), "[%c] %s ", checked ? 'X' : ' ', caption);
        vdev_puts(vdev, buf);
    }
    return err;
}

BppError stmt_optionbutton_handler(VMContext *vm, LexerContext *lex) {
    BppError err; memset(&err, 0, sizeof(err));
    BValue args[6];
    int count = parse_ctrl_args(vm, lex, args, 6, &err);
    if (err.code != 0 || count == 0) return err;

    char caption[128] = "Option";
    int selected = 0;

    int str_idx = -1;
    for (int i = 0; i < count; i++) {
        if (args[i].type == VAL_STRING) { str_idx = i; break; }
    }

    if (str_idx >= 0 && args[str_idx].as.string) {
        const char *s = str_data(args[str_idx].as.string);
        if (s) strncpy(caption, s, sizeof(caption) - 1);
        if (str_idx + 1 < count && args[str_idx + 1].type == VAL_NUMBER) {
            selected = (args[str_idx + 1].as.number != 0.0);
        }
    } else {
        snprintf(caption, sizeof(caption), "%g", args[count - 1].as.number);
    }

    for (int i = 0; i < count; i++) {
        if (args[i].type == VAL_STRING && args[i].as.string) {
            str_release(vm_get_str(vm), args[i].as.string);
        }
    }

    VDevContext *vdev = vm_get_vdev(vm);
    if (vdev) {
        char buf[256];
        snprintf(buf, sizeof(buf), "(%c) %s ", selected ? 'o' : ' ', caption);
        vdev_puts(vdev, buf);
    }
    return err;
}

BppError stmt_label_handler(VMContext *vm, LexerContext *lex) {
    BppError err; memset(&err, 0, sizeof(err));
    BValue args[6];
    int count = parse_ctrl_args(vm, lex, args, 6, &err);
    if (err.code != 0 || count == 0) return err;

    char text[128] = "Label";
    int txt_idx = count - 1;
    for (int i = 0; i < count; i++) {
        if (args[i].type == VAL_STRING) { txt_idx = i; break; }
    }
    if (args[txt_idx].type == VAL_STRING && args[txt_idx].as.string) {
        const char *s = str_data(args[txt_idx].as.string);
        if (s) strncpy(text, s, sizeof(text) - 1);
    } else {
        snprintf(text, sizeof(text), "%g", args[txt_idx].as.number);
    }

    for (int i = 0; i < count; i++) {
        if (args[i].type == VAL_STRING && args[i].as.string) {
            str_release(vm_get_str(vm), args[i].as.string);
        }
    }

    VDevContext *vdev = vm_get_vdev(vm);
    if (vdev) {
        char buf[256];
        snprintf(buf, sizeof(buf), "%s ", text);
        vdev_puts(vdev, buf);
    }
    return err;
}

BppError stmt_frame_handler(VMContext *vm, LexerContext *lex) {
    BppError err; memset(&err, 0, sizeof(err));
    BValue args[6];
    int count = parse_ctrl_args(vm, lex, args, 6, &err);
    if (err.code != 0 || count == 0) return err;

    char title[128] = "Frame";
    int str_idx = count - 1;
    for (int i = 0; i < count; i++) {
        if (args[i].type == VAL_STRING) { str_idx = i; break; }
    }
    if (args[str_idx].type == VAL_STRING && args[str_idx].as.string) {
        const char *s = str_data(args[str_idx].as.string);
        if (s) strncpy(title, s, sizeof(title) - 1);
    }

    for (int i = 0; i < count; i++) {
        if (args[i].type == VAL_STRING && args[i].as.string) {
            str_release(vm_get_str(vm), args[i].as.string);
        }
    }

    VDevContext *vdev = vm_get_vdev(vm);
    if (vdev) {
        char buf[256];
        snprintf(buf, sizeof(buf), "+--- %s ---+ ", title);
        vdev_puts(vdev, buf);
    }
    return err;
}

BppError stmt_combobox_handler(VMContext *vm, LexerContext *lex) {
    BppError err; memset(&err, 0, sizeof(err));
    BValue args[6];
    int count = parse_ctrl_args(vm, lex, args, 6, &err);
    if (err.code != 0 || count == 0) return err;

    char items[128] = "Item 1,Item 2";
    int str_idx = count - 1;
    for (int i = 0; i < count; i++) {
        if (args[i].type == VAL_STRING) { str_idx = i; break; }
    }
    if (args[str_idx].type == VAL_STRING && args[str_idx].as.string) {
        const char *s = str_data(args[str_idx].as.string);
        if (s) strncpy(items, s, sizeof(items) - 1);
    }

    for (int i = 0; i < count; i++) {
        if (args[i].type == VAL_STRING && args[i].as.string) {
            str_release(vm_get_str(vm), args[i].as.string);
        }
    }

    VDevContext *vdev = vm_get_vdev(vm);
    if (vdev) {
        char buf[256];
        snprintf(buf, sizeof(buf), "[ %s | v ] ", items);
        vdev_puts(vdev, buf);
    }
    return err;
}

BppError stmt_hscrollbar_handler(VMContext *vm, LexerContext *lex) {
    BppError err; memset(&err, 0, sizeof(err));
    BValue args[8];
    int count = parse_ctrl_args(vm, lex, args, 8, &err);
    if (err.code != 0 || count == 0) return err;

    for (int i = 0; i < count; i++) {
        if (args[i].type == VAL_STRING && args[i].as.string) {
            str_release(vm_get_str(vm), args[i].as.string);
        }
    }

    VDevContext *vdev = vm_get_vdev(vm);
    if (vdev) {
        vdev_puts(vdev, "[<---#------->] ");
    }
    return err;
}

BppError stmt_vscrollbar_handler(VMContext *vm, LexerContext *lex) {
    BppError err; memset(&err, 0, sizeof(err));
    BValue args[8];
    int count = parse_ctrl_args(vm, lex, args, 8, &err);
    if (err.code != 0 || count == 0) return err;

    for (int i = 0; i < count; i++) {
        if (args[i].type == VAL_STRING && args[i].as.string) {
            str_release(vm_get_str(vm), args[i].as.string);
        }
    }

    VDevContext *vdev = vm_get_vdev(vm);
    if (vdev) {
        vdev_puts(vdev, "[^|#|v] ");
    }
    return err;
}

BppError stmt_drivelistbox_handler(VMContext *vm, LexerContext *lex) {
    BppError err; memset(&err, 0, sizeof(err));
    BValue args[6];
    int count = parse_ctrl_args(vm, lex, args, 6, &err);
    if (err.code != 0) return err;

    for (int i = 0; i < count; i++) {
        if (args[i].type == VAL_STRING && args[i].as.string) {
            str_release(vm_get_str(vm), args[i].as.string);
        }
    }

    VDevContext *vdev = vm_get_vdev(vm);
    if (vdev) {
        vdev_puts(vdev, "[-C-] [-D-] ");
    }
    return err;
}

BppError stmt_dirlistbox_handler(VMContext *vm, LexerContext *lex) {
    BppError err; memset(&err, 0, sizeof(err));
    BValue args[6];
    int count = parse_ctrl_args(vm, lex, args, 6, &err);
    if (err.code != 0) return err;

    char path[128] = "C:\\BASIC";
    for (int i = 0; i < count; i++) {
        if (args[i].type == VAL_STRING && args[i].as.string) {
            const char *s = str_data(args[i].as.string);
            if (s) strncpy(path, s, sizeof(path) - 1);
            break;
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
        snprintf(buf, sizeof(buf), "[DIR: %s] ", path);
        vdev_puts(vdev, buf);
    }
    return err;
}

BppError stmt_filelistbox_handler(VMContext *vm, LexerContext *lex) {
    BppError err; memset(&err, 0, sizeof(err));
    BValue args[6];
    int count = parse_ctrl_args(vm, lex, args, 6, &err);
    if (err.code != 0) return err;

    char filter[128] = "*.*";
    for (int i = 0; i < count; i++) {
        if (args[i].type == VAL_STRING && args[i].as.string) {
            const char *s = str_data(args[i].as.string);
            if (s) strncpy(filter, s, sizeof(filter) - 1);
            break;
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
        snprintf(buf, sizeof(buf), "[FILES: %s] ", filter);
        vdev_puts(vdev, buf);
    }
    return err;
}

BppError stmt_timercontrol_handler(VMContext *vm, LexerContext *lex) {
    BppError err; memset(&err, 0, sizeof(err));
    BValue args[6];
    int count = parse_ctrl_args(vm, lex, args, 6, &err);
    if (err.code != 0) return err;

    int interval = 1000;
    int enabled = 1;
    if (count >= 1 && args[0].type == VAL_NUMBER) interval = (int)args[0].as.number;
    if (count >= 2 && args[1].type == VAL_NUMBER) enabled = (args[1].as.number != 0.0);

    for (int i = 0; i < count; i++) {
        if (args[i].type == VAL_STRING && args[i].as.string) {
            str_release(vm_get_str(vm), args[i].as.string);
        }
    }

    VDevContext *vdev = vm_get_vdev(vm);
    if (vdev) {
        char buf[256];
        snprintf(buf, sizeof(buf), "[TIMER: %dms %s] ", interval, enabled ? "ON" : "OFF");
        vdev_puts(vdev, buf);
    }
    return err;
}

void stmt_vbdos_controls_register(void) {
    static const MicroLibMetadata meta_cb = { .name = "CHECKBOX", .category = "User Interface", .syntax = "CHECKBOX caption$ [, checked%]" };
    static const MicroLibMetadata meta_ob = { .name = "OPTIONBUTTON", .category = "User Interface", .syntax = "OPTIONBUTTON caption$ [, selected%]" };
    static const MicroLibMetadata meta_lbl = { .name = "LABEL", .category = "User Interface", .syntax = "LABEL text$" };
    static const MicroLibMetadata meta_frm = { .name = "FRAME", .category = "User Interface", .syntax = "FRAME title$" };
    static const MicroLibMetadata meta_cmb = { .name = "COMBOBOX", .category = "User Interface", .syntax = "COMBOBOX items$ [, sel%]" };
    static const MicroLibMetadata meta_hsb = { .name = "HSCROLLBAR", .category = "User Interface", .syntax = "HSCROLLBAR min%, max%, val%, col%, row%, width%" };
    static const MicroLibMetadata meta_vsb = { .name = "VSCROLLBAR", .category = "User Interface", .syntax = "VSCROLLBAR min%, max%, val%, col%, row%, height%" };
    static const MicroLibMetadata meta_dlb = { .name = "DRIVELISTBOX", .category = "User Interface", .syntax = "DRIVELISTBOX col%, row%, width%" };
    static const MicroLibMetadata meta_dir = { .name = "DIRLISTBOX", .category = "User Interface", .syntax = "DIRLISTBOX path$, col%, row%, width%, height%" };
    static const MicroLibMetadata meta_flb = { .name = "FILELISTBOX", .category = "User Interface", .syntax = "FILELISTBOX filter$, col%, row%, width%, height%" };
    static const MicroLibMetadata meta_tmr = { .name = "TIMERCONTROL", .category = "User Interface", .syntax = "TIMERCONTROL interval_ms%, enabled%" };
    microlib_register(&meta_cb);
    microlib_register(&meta_ob);
    microlib_register(&meta_lbl);
    microlib_register(&meta_frm);
    microlib_register(&meta_cmb);
    microlib_register(&meta_hsb);
    microlib_register(&meta_vsb);
    microlib_register(&meta_dlb);
    microlib_register(&meta_dir);
    microlib_register(&meta_flb);
    microlib_register(&meta_tmr);
}
