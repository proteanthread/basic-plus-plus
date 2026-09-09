// FILENAME: stmt_device.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, exec_dispatch.c
// NEEDS: libkernel (types.h, lexer.h, errors.h, vdev.h), libcore (eval.h, strings.h, language_descriptor.h)
// Implementation for DEVICE and DEVICES statements in BASIC++.
//
// ---- Includes ----

#include "statements/io/device/stmt_device.h"
#include "device/vdev.h"
#include "eval/eval.h"
#include "runtime/strings.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/ctype/ctype.h"
#include "platform/platform.h"

static const LangDesc g_device_desc = {
    .name = "DEVICE",
    .category = "System & Hardware",
    .syntax = "DEVICE \"dev_name:\", \"config_param=val\"",
    .description = "Manages virtual devices, aliases, User-Defined Devices (UDD), and dynamic driver bindings.",
    .error_summary = "Error 2: Syntax Error, Error 5: Illegal Function Call, Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SYSTEM,
    .type = FEATURE_STATEMENT
};

static const LangDesc g_devices_desc = {
    .name = "DEVICES",
    .category = "System & Hardware",
    .syntax = "DEVICE \"dev_name:\", \"config_param=val\"",
    .description = "Lists all registered virtual devices, drivers, classes, and active aliases.",
    .error_summary = "Error 2: Syntax Error",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SYSTEM,
    .type = FEATURE_STATEMENT
};

void stmt_device_register(void) {
    lang_desc_register(&g_device_desc);

    lang_desc_register(&g_devices_desc);
}

BppError stmt_devices_handler(VMContext *vm, LexerContext *lex) {
    (void)lex;
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    if (!vm) {
        err.code = 5; err.message = "Null context in DEVICES";
        return err;
    }
    VDevContext *ctx = vm_get_vdev(vm);
    if (ctx) {
        vdev_list_all(ctx);
        vdev_puts(ctx, "\n");
        vdev_alias_list(ctx);
    }
    return err;
}

BppError stmt_device_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    if (!vm || !lex) {
        err.code = 5; err.message = "Null context in DEVICE";
        return err;
    }

    VDevContext *ctx = vm_get_vdev(vm);
    BppToken tok = lex_peek(lex);

    // Form 1: DEVICE LIST or no args
    if (tok.type == TOK_EOF || tok.type == TOK_EOL || tok.type == TOK_BACKSLASH) {
        return stmt_devices_handler(vm, lex);
    }

    // Check for keyword sub-commands
    if (tok.type == TOK_IDENT || tok.type == TOK_KEYWORD) {
        if ((tok.type == TOK_KEYWORD && tok.as.keyword == KW_LIST) ||
            (tok.type == TOK_IDENT && tok.length == 4 && platform_strncasecmp(tok.start, "LIST", 4) == 0)) {
            lex_next(lex);
            return stmt_devices_handler(vm, lex);
        }

        if (tok.type == TOK_IDENT && tok.length == 5 && platform_strncasecmp(tok.start, "RESET", 5) == 0) {
            lex_next(lex);
            if (ctx) vdev_reset_defaults(ctx);
            return err;
        }

        // Form 2: DEVICE ALIAS "alias:" AS "target:"
        if ((tok.type == TOK_KEYWORD && tok.as.keyword == KW_ALIAS) ||
            (tok.type == TOK_IDENT && tok.length == 5 && platform_strncasecmp(tok.start, "ALIAS", 5) == 0)) {
            lex_next(lex); // Consume ALIAS

            BValue alias_val = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            if (alias_val.type != VAL_STRING || !alias_val.as.string) {
                if (alias_val.type == VAL_STRING && alias_val.as.string) str_release(vm_get_str(vm), alias_val.as.string);
                err.code = ERR_TYPE_MISMATCH;
                return err;
            }

            BppToken as_tok = lex_peek(lex);
            bool is_as = (as_tok.type == TOK_KEYWORD && (as_tok.as.keyword == KW_AS || as_tok.as.keyword == KW_TO)) ||
                         (as_tok.type == TOK_IDENT && as_tok.length == 2 && (platform_strncasecmp(as_tok.start, "AS", 2) == 0 || platform_strncasecmp(as_tok.start, "TO", 2) == 0));
            if (is_as) {
                lex_next(lex); // Consume AS / TO
            } else {
                str_release(vm_get_str(vm), alias_val.as.string);
                err.code = ERR_SYNTAX;
                err.message = "Expected 'AS' in DEVICE ALIAS";
                return err;
            }

            BValue target_val = eval_expression(vm, lex, &err);
            if (err.code != 0) {
                str_release(vm_get_str(vm), alias_val.as.string);
                return err;
            }
            if (target_val.type != VAL_STRING || !target_val.as.string) {
                str_release(vm_get_str(vm), alias_val.as.string);
                if (target_val.type == VAL_STRING && target_val.as.string) str_release(vm_get_str(vm), target_val.as.string);
                err.code = ERR_TYPE_MISMATCH;
                return err;
            }

            if (ctx) {
                vdev_alias_set(ctx, str_data(alias_val.as.string), str_data(target_val.as.string));
            }
            str_release(vm_get_str(vm), alias_val.as.string);
            str_release(vm_get_str(vm), target_val.as.string);
            return err;
        }

        // Form 3: DEVICE UNALIAS "alias:"
        if (tok.type == TOK_IDENT && tok.length == 7 && platform_strncasecmp(tok.start, "UNALIAS", 7) == 0) {
            lex_next(lex); // Consume UNALIAS

            BValue alias_val = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            if (alias_val.type != VAL_STRING || !alias_val.as.string) {
                if (alias_val.type == VAL_STRING && alias_val.as.string) str_release(vm_get_str(vm), alias_val.as.string);
                err.code = ERR_TYPE_MISMATCH;
                return err;
            }

            if (ctx) {
                vdev_alias_remove(ctx, str_data(alias_val.as.string));
            }
            str_release(vm_get_str(vm), alias_val.as.string);
            return err;
        }

        // Form 4: DEVICE MOUNT "dev:" TYPE "driver" [OPTIONS "opts"]
        if ((tok.type == TOK_KEYWORD && tok.as.keyword == KW_MOUNT) ||
            (tok.type == TOK_IDENT && tok.length == 5 && platform_strncasecmp(tok.start, "MOUNT", 5) == 0)) {
            lex_next(lex); // Consume MOUNT

            BValue dev_val = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            if (dev_val.type != VAL_STRING || !dev_val.as.string) {
                if (dev_val.type == VAL_STRING && dev_val.as.string) str_release(vm_get_str(vm), dev_val.as.string);
                err.code = ERR_TYPE_MISMATCH;
                return err;
            }

            BppToken type_tok = lex_peek(lex);
            bool is_type = (type_tok.type == TOK_KEYWORD && type_tok.as.keyword == KW_TYPE) ||
                           (type_tok.type == TOK_IDENT && type_tok.length == 4 && platform_strncasecmp(type_tok.start, "TYPE", 4) == 0);
            if (is_type) {
                lex_next(lex); // Consume TYPE
            } else {
                str_release(vm_get_str(vm), dev_val.as.string);
                err.code = ERR_SYNTAX;
                err.message = "Expected 'TYPE' in DEVICE MOUNT";
                return err;
            }

            BValue type_val = eval_expression(vm, lex, &err);
            if (err.code != 0) {
                str_release(vm_get_str(vm), dev_val.as.string);
                return err;
            }
            if (type_val.type != VAL_STRING || !type_val.as.string) {
                str_release(vm_get_str(vm), dev_val.as.string);
                if (type_val.type == VAL_STRING && type_val.as.string) str_release(vm_get_str(vm), type_val.as.string);
                err.code = ERR_TYPE_MISMATCH;
                return err;
            }

            const char *opts = "";
            BppToken opt_tok = lex_peek(lex);
            if (opt_tok.type == TOK_IDENT && opt_tok.length == 7 && platform_strncasecmp(opt_tok.start, "OPTIONS", 7) == 0) {
                lex_next(lex);
                BValue opt_val = eval_expression(vm, lex, &err);
                if (err.code == 0 && opt_val.type == VAL_STRING && opt_val.as.string) {
                    opts = str_data(opt_val.as.string);
                    str_release(vm_get_str(vm), opt_val.as.string);
                }
            }

            if (ctx) {
                vdev_mount(ctx, str_data(dev_val.as.string), str_data(type_val.as.string), opts);
            }
            str_release(vm_get_str(vm), dev_val.as.string);
            str_release(vm_get_str(vm), type_val.as.string);
            return err;
        }

        // Form 5: DEVICE UNMOUNT "dev:"
        if ((tok.type == TOK_KEYWORD && tok.as.keyword == KW_UMOUNT) ||
            (tok.type == TOK_IDENT && tok.length == 7 && platform_strncasecmp(tok.start, "UNMOUNT", 7) == 0) ||
            (tok.type == TOK_IDENT && tok.length == 6 && platform_strncasecmp(tok.start, "UMOUNT", 6) == 0)) {
            lex_next(lex); // Consume UNMOUNT

            BValue dev_val = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            if (dev_val.type != VAL_STRING || !dev_val.as.string) {
                if (dev_val.type == VAL_STRING && dev_val.as.string) str_release(vm_get_str(vm), dev_val.as.string);
                err.code = ERR_TYPE_MISMATCH;
                return err;
            }

            if (ctx) {
                vdev_unmount(ctx, str_data(dev_val.as.string));
            }
            str_release(vm_get_str(vm), dev_val.as.string);
            return err;
        }

        // Form 6: DEVICE StructName ... END DEVICE (UDD definition)
        // Skip until END DEVICE
        lex_next(lex); // Consume UDD name
        while (true) {
            BppToken cur = lex_peek(lex);
            if (cur.type == TOK_EOF) break;
            if ((cur.type == TOK_KEYWORD && cur.as.keyword == KW_END) ||
                (cur.type == TOK_IDENT && cur.length == 3 && platform_strncasecmp(cur.start, "END", 3) == 0)) {
                lex_next(lex);
                BppToken next_tok = lex_peek(lex);
                if (next_tok.type == TOK_IDENT && next_tok.length == 6 && platform_strncasecmp(next_tok.start, "DEVICE", 6) == 0) {
                    lex_next(lex);
                    break;
                }
            }
            lex_next(lex);
        }
        return err;
    }

    err.code = ERR_SYNTAX;
    err.message = "Unrecognized DEVICE sub-command";
    return err;
}
