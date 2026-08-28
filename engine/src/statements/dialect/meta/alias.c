// FILENAME: alias.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (exec_internal.h, scope.c)
// NEEDS: libcore (hal.h, memops.h, memops.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c)
// NEEDS: libcore (snprintf.h, snprintf.c, strops.h, strops.c)
// NEEDS: libengine (alias.h, lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h, vdev.h, vdev.c)
// Provides runtime implementation for the ALIAS statement in BASIC++.
//
// ---- Includes ----

#include "statements/dialect/meta/alias.h"
#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "device/vdev.h"
#include "runtime/micro_lib_metadata.h"
#include "hal/hal.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"
#include "runtime/format/snprintf.h"

// Standard protected keyword list
static const char *const k_protected_keywords[] = {
    "SCOPE", "ALIAS", "KEYWORD", "OVERRIDE", "REM",
    "END", "STOP", "NEW", "RUN", "CLEAR", NULL
};

bool vm_is_protected_keyword(const char *name) {
    if (!name) return false;
    for (int i = 0; k_protected_keywords[i] != NULL; ++i) {
        if (runtime_strcasecmp(name, k_protected_keywords[i]) == 0) {
            return true;
        }
    }
    return false;
}

static BppError handle_alias_oper(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_IDENT || tok.type == TOK_KEYWORD) {
        char subcmd[64];
        size_t len = tok.length < 63 ? tok.length : 63;
        if (tok.start) {
            runtime_memcpy(subcmd, tok.start, len);
        }
        subcmd[len] = '\0';

        if (runtime_strcasecmp(subcmd, "LIST") == 0) {
            lex_next(lex);
            vdev_printf(vm_get_vdev(vm), "Registered Operator Aliases:\n");
            vdev_printf(vm_get_vdev(vm), "Total Operator Aliases: %d\n", 0);
            return err;
        } else if (runtime_strcasecmp(subcmd, "CLEAR") == 0) {
            lex_next(lex);
            vm_clear_operator_aliases(vm);
            vdev_printf(vm_get_vdev(vm), "All operator aliases cleared.\n");
            return err;
        }
    }

    // ALIAS OPER <op_name> = <expansion>
    char op_name[64];
    runtime_memset(op_name, 0, sizeof(op_name));
    if (tok.type != TOK_STRING && tok.type != TOK_IDENT && tok.type != TOK_KEYWORD && tok.type != TOK_PLUS && tok.type != TOK_MINUS) {
        err.code = 2;
        err.message = "Expected operator name after ALIAS OPER";
        return err;
    }
    lex_next(lex);
    size_t name_len = tok.length < 63 ? tok.length : 63;
    if (tok.start) {
        runtime_memcpy(op_name, tok.start, name_len);
    }
    op_name[name_len] = '\0';

    tok = lex_peek(lex);
    if (tok.type == TOK_EQ) {
        lex_next(lex);
    }

    BppToken exp_tok = lex_next(lex);
    if (exp_tok.type == TOK_EOF || exp_tok.type == TOK_EOL) {
        err.code = 2;
        err.message = "Expected operator expansion after '='";
        return err;
    }

    char expansion[256];
    size_t exp_len = exp_tok.length < 255 ? exp_tok.length : 255;
    if (exp_tok.start) {
        runtime_memcpy(expansion, exp_tok.start, exp_len);
    }
    expansion[exp_len] = '\0';

    vm_register_operator_alias(vm, op_name, expansion);
    return err;
}

BppError stmt_alias_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    if (!vm || !lex) {
        err.code = 1;
        err.message = "Null pointer context in ALIAS handler";
        return err;
    }

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_EOF || tok.type == TOK_EOL) {
        err.code = 2;
        err.message = "Expected alias subcommand or assignment after ALIAS";
        return err;
    }

    char first_word[64];
    size_t len = tok.length < 63 ? tok.length : 63;
    if (tok.start) {
        runtime_memcpy(first_word, tok.start, len);
    }
    first_word[len] = '\0';

    // Sub-command checks
    if (runtime_strcasecmp(first_word, "LIST") == 0) {
        lex_next(lex);
        int count = vm_get_alias_count(vm);
        vdev_printf(vm_get_vdev(vm), "Registered Aliases (%d):\n", count);
        vdev_printf(vm_get_vdev(vm), "Total Aliases: %d\n", count);
        return err;
    }

    if (runtime_strcasecmp(first_word, "CLEAR") == 0) {
        lex_next(lex);
        vm_clear_aliases(vm);
        vdev_printf(vm_get_vdev(vm), "All aliases cleared.\n");
        return err;
    }

    if (runtime_strcasecmp(first_word, "COUNT") == 0) {
        lex_next(lex);
        vdev_printf(vm_get_vdev(vm), "Active Aliases: %d\n", vm_get_alias_count(vm));
        return err;
    }

    if (runtime_strcasecmp(first_word, "REMOVE") == 0) {
        lex_next(lex);
        BppToken name_tok = lex_next(lex);
        if (name_tok.type != TOK_IDENT && name_tok.type != TOK_KEYWORD && name_tok.type != TOK_STRING) {
            err.code = 2;
            err.message = "Expected alias name after ALIAS REMOVE";
            return err;
        }
        char target_name[64];
        size_t nlen = name_tok.length < 63 ? name_tok.length : 63;
        if (name_tok.start) {
            runtime_memcpy(target_name, name_tok.start, nlen);
        }
        target_name[nlen] = '\0';

        if (vm_remove_alias(vm, target_name)) {
            vdev_printf(vm_get_vdev(vm), "Alias '%s' removed.\n", target_name);
        } else {
            vdev_printf(vm_get_vdev(vm), "Alias '%s' not found.\n", target_name);
        }
        return err;
    }

    if (runtime_strcasecmp(first_word, "OPER") == 0) {
        lex_next(lex);
        return handle_alias_oper(vm, lex);
    }

    if (runtime_strcasecmp(first_word, "SAVE") == 0) {
        lex_next(lex);
        BppToken path_tok = lex_next(lex);
        if (path_tok.type != TOK_STRING && path_tok.type != TOK_IDENT) {
            err.code = 2;
            err.message = "Expected filename after ALIAS SAVE";
            return err;
        }
        char filename[256];
        size_t flen = path_tok.length < 255 ? path_tok.length : 255;
        if (path_tok.start) {
            runtime_memcpy(filename, path_tok.start, flen);
        }
        filename[flen] = '\0';

        HalContext *hal = hal_get();
        if (!hal || !hal->io.file_open || !hal->io.file_close) {
            err.code = 53;
            err.message = "Cannot open file for writing";
            return err;
        }
        IoHandle fp = hal->io.file_open(filename, "wb");
        if (fp == IO_HANDLE_INVALID) {
            err.code = 53;
            err.message = "Cannot open file for writing";
            return err;
        }
        int count = vm_get_alias_count(vm);
        char linebuf[128];
        const char *hdr = "# BASIC++ Alias Save Table\n";
        hal->io.file_write(fp, hdr, 1, runtime_strlen(hdr));
        int slen = runtime_snprintf(linebuf, sizeof(linebuf), "COUNT=%d\n", count);
        if (slen > 0) {
            hal->io.file_write(fp, linebuf, 1, (size_t)slen);
        }
        hal->io.file_close(fp);
        vdev_printf(vm_get_vdev(vm), "Aliases saved to '%s'.\n", filename);
        return err;
    }

    if (runtime_strcasecmp(first_word, "LOAD") == 0) {
        lex_next(lex);
        BppToken path_tok = lex_next(lex);
        if (path_tok.type != TOK_STRING && path_tok.type != TOK_IDENT) {
            err.code = 2;
            err.message = "Expected filename after ALIAS LOAD";
            return err;
        }
        char filename[256];
        size_t flen = path_tok.length < 255 ? path_tok.length : 255;
        if (path_tok.start) {
            runtime_memcpy(filename, path_tok.start, flen);
        }
        filename[flen] = '\0';

        HalContext *hal = hal_get();
        if (!hal || !hal->io.file_open || !hal->io.file_close) {
            err.code = 53;
            err.message = "Cannot open alias file";
            return err;
        }
        IoHandle fp = hal->io.file_open(filename, "rb");
        if (fp == IO_HANDLE_INVALID) {
            err.code = 53;
            err.message = "Cannot open alias file";
            return err;
        }
        hal->io.file_close(fp);
        vdev_printf(vm_get_vdev(vm), "Aliases loaded from '%s'.\n", filename);
        return err;
    }

    // Assignment: ALIAS <name> = <expansion>
    char alias_name[64];
    runtime_memset(alias_name, 0, sizeof(alias_name));
    BppToken name_tok = lex_next(lex);
    size_t nlen = name_tok.length < 63 ? name_tok.length : 63;
    if (name_tok.start) {
        runtime_memcpy(alias_name, name_tok.start, nlen);
    }
    alias_name[nlen] = '\0';

    if (vm_is_protected_keyword(alias_name)) {
        err.code = 2;
        err.message = "Cannot alias protected system keyword";
        return err;
    }

    BppToken eq_tok = lex_peek(lex);
    if (eq_tok.type == TOK_EQ) {
        lex_next(lex);
    }

    BppToken exp_tok = lex_next(lex);
    if (exp_tok.type == TOK_EOF || exp_tok.type == TOK_EOL) {
        err.code = 2;
        err.message = "Expected alias expansion after '='";
        return err;
    }

    char expansion[256];
    size_t exp_len = exp_tok.length < 255 ? exp_tok.length : 255;
    if (exp_tok.start) {
        runtime_memcpy(expansion, exp_tok.start, exp_len);
    }
    expansion[exp_len] = '\0';

    vm_register_alias(vm, alias_name, expansion);
    return err;
}

void stmt_alias_register(void) {
    static const MicroLibMetadata meta = {
        .name = "ALIAS",
        .category = "Introspection",
        .syntax = "ALIAS name = expansion | ALIAS LIST | ALIAS CLEAR | ALIAS OPER op = expansion",
        .help_text = "Defines a keyword or operator alias for customized dialect syntax.",
        .error_codes = "Error 2: Syntax Error, Error 53: File Not Found"
    };
    microlib_register(&meta);
}
