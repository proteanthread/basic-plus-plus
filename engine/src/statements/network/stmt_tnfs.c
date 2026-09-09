// FILENAME: stmt_tnfs.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (string.h, strings.h, strings.c, tnfs.h, tnfs.c)
// NEEDS: libengine (eval.h, eval.c, stmt_tnfs.h, string.c)
// Implements TNFS.MOUNT and TNFS.UNMOUNT statement handlers.
//
// ---- Includes ----

#include "statements/network/stmt_tnfs.h"
#include "runtime/tnfs.h"
#include "eval/eval.h"
#include "runtime/strings.h"
#include "runtime/language_descriptor.h"
#include "runtime/format/snprintf.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

BppError stmt_tnfs_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    bool is_mount = false;
    bool is_unmount = false;

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_PERIOD) {
        lex_next(lex);
        tok = lex_peek(lex);
    }

    if (tok.type == TOK_IDENT || tok.type == TOK_KEYWORD) {
        if (tok.length == 7 && runtime_strncasecmp(tok.start, "UNMOUNT", 7) == 0) {
            is_unmount = true;
            lex_next(lex);
        } else if (tok.length == 5 && runtime_strncasecmp(tok.start, "MOUNT", 5) == 0) {
            is_mount = true;
            lex_next(lex);
        } else {
            lex_next(lex);
            is_mount = true;
        }
    } else {
        is_mount = true;
    }

    if (is_unmount) {
        tnfs_unmount();
        return err;
    }

    if (is_mount) {
        BValue host_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;

        const char *host = (host_val.type == VAL_STRING) ? str_data(host_val.as.string) : "tnfs.fujinet.online";
        const char *path = "/";
        uint16_t port = TNFS_DEFAULT_PORT;

        tok = lex_peek(lex);
        if (tok.type == TOK_COMMA) {
            lex_next(lex);
            BValue path_val = eval_expression(vm, lex, &err);
            if (err.code == 0 && path_val.type == VAL_STRING) {
                path = str_data(path_val.as.string);
            }
            tok = lex_peek(lex);
            if (tok.type == TOK_COMMA) {
                lex_next(lex);
                BValue port_val = eval_expression(vm, lex, &err);
                if (err.code == 0 && port_val.type == VAL_NUMBER) {
                    port = (uint16_t)port_val.as.number;
                }
            }
            if (path_val.type == VAL_STRING) {
                str_release(vm_get_str(vm), path_val.as.string);
            }
        }

        tnfs_mount(host, port, path);

        if (host_val.type == VAL_STRING) {
            str_release(vm_get_str(vm), host_val.as.string);
        }
    }

    return err;
}

static const LangDesc g_tnfs_desc = {
    .name = "TNFS",
    .category = "Hardware & Network",
    .syntax = "TNFS.MOUNT host$ [, path$ [, port]] | TNFS.UNMOUNT",
    .description = "Mounts or unmounts a remote Trusted Network File System (TNFS) server repository.",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_HARDWARE,
    .safety = SAFETY_IO,
    .type = FEATURE_STATEMENT
};

void stmt_tnfs_register(void) {
    lang_desc_register(&g_tnfs_desc);
}
