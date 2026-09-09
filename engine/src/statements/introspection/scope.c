// FILENAME: scope.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (exec_control_internal.h, exec_internal.h, module.c, sub_internal.h)
// NEEDS: libcore (language_descriptor.h, memops.h, strops.h)
// NEEDS: libengine (alias.h, lexer.h, scope.h, vm.h)
// NEEDS: libkernel (types.h, vdev.h)
// Provides runtime implementation for the SCOPE statement in BASIC++.

#include "statements/introspection/scope.h"
#include "scope/scope.h"
#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "device/vdev.h"
#include "statements/introspection/alias.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"

static const LangDesc g_scope_desc = {
    .name = "SCOPE",
    .category = "Introspection",
    .syntax = "SCOPE [BEGIN | END | DISABLE kw | ENABLE kw | HOOK ... | MODULE name | PRIVATE sym]",
    .description = "Manages lexical scope blocks, keyword enablement, execution hooks, and symbol protection.",
    .error_summary = "Error 2: Syntax Error, Error 13: Permission Denied",
    .subsystem = SUBSYSTEM_FLEX,
    .safety = SAFETY_SAFE,
    .type = FEATURE_STATEMENT
};

void stmt_scope_register(void) {
    lang_desc_register(&g_scope_desc);
}

static BppError handle_scope_hook(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    BppToken type_tok = lex_next(lex);
    char hook_type_str[64];
    size_t len = type_tok.length < 63 ? type_tok.length : 63;
    if (type_tok.start) {
        runtime_memcpy(hook_type_str, type_tok.start, len);
    }
    hook_type_str[len] = '\0';

    BppHookType hook_type = HOOK_BEFORE;
    if (runtime_strcasecmp(hook_type_str, "BEFORE") == 0) {
        hook_type = HOOK_BEFORE;
    } else if (runtime_strcasecmp(hook_type_str, "AFTER") == 0) {
        hook_type = HOOK_AFTER;
    } else if (runtime_strcasecmp(hook_type_str, "OVERRIDE") == 0) {
        hook_type = HOOK_OVERRIDE;
    } else if (runtime_strcasecmp(hook_type_str, "CLEAR") == 0) {
        scope_clear_hooks(vm);
        vdev_printf(vm_get_vdev(vm), "All execution hooks cleared.\n");
        return err;
    } else {
        err.code = 2;
        err.message = "Expected BEFORE, AFTER, OVERRIDE, or CLEAR after SCOPE HOOK";
        return err;
    }

    BppToken cmd_tok = lex_next(lex);
    if (cmd_tok.type != TOK_IDENT && cmd_tok.type != TOK_KEYWORD) {
        err.code = 2;
        err.message = "Expected command name after SCOPE HOOK type";
        return err;
    }
    char cmd_name[64];
    size_t clen = cmd_tok.length < 63 ? cmd_tok.length : 63;
    if (cmd_tok.start) {
        runtime_memcpy(cmd_name, cmd_tok.start, clen);
    }
    cmd_name[clen] = '\0';

    BppToken gosub_tok = lex_next(lex);
    char gosub_str[64];
    size_t glen = gosub_tok.length < 63 ? gosub_tok.length : 63;
    if (gosub_tok.start) {
        runtime_memcpy(gosub_str, gosub_tok.start, glen);
    }
    gosub_str[glen] = '\0';

    if (runtime_strcasecmp(gosub_str, "GOSUB") != 0) {
        err.code = 2;
        err.message = "Expected GOSUB after target command name in SCOPE HOOK";
        return err;
    }

    BppToken target_tok = lex_next(lex);
    if (target_tok.type == TOK_NUMBER) {
        scope_register_hook(vm, cmd_name, hook_type, NULL, (BppLineNumber)target_tok.as.number);
    } else if (target_tok.type == TOK_IDENT) {
        char label_buf[64];
        size_t l_len = target_tok.length < 63 ? target_tok.length : 63;
        if (target_tok.start) {
            runtime_memcpy(label_buf, target_tok.start, l_len);
        }
        label_buf[l_len] = '\0';
        scope_register_hook(vm, cmd_name, hook_type, label_buf, 0);
    } else {
        err.code = 2;
        err.message = "Expected line number or label after GOSUB in SCOPE HOOK";
        return err;
    }

    vdev_printf(vm_get_vdev(vm), "Hook registered for '%s'.\n", cmd_name);
    return err;
}

BppError stmt_scope_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    if (!vm || !lex) {
        err.code = 1;
        err.message = "Null pointer context in SCOPE handler";
        return err;
    }

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_EOF || tok.type == TOK_EOL) {
        err.code = 2;
        err.message = "Expected SCOPE sub-command";
        return err;
    }

    char subcmd[64];
    size_t len = tok.length < 63 ? tok.length : 63;
    if (tok.start) {
        runtime_memcpy(subcmd, tok.start, len);
    }
    subcmd[len] = '\0';

    if (runtime_strcasecmp(subcmd, "BEGIN") == 0) {
        lex_next(lex);
        scope_block_push(vm);
        return err;
    }

    if (runtime_strcasecmp(subcmd, "END") == 0) {
        lex_next(lex);
        if (!scope_block_pop(vm)) {
            err.code = 2;
            err.message = "SCOPE END without matching SCOPE BEGIN";
            return err;
        }
        return err;
    }

    if (runtime_strcasecmp(subcmd, "MODULE") == 0) {
        lex_next(lex);
        BppToken name_tok = lex_next(lex);
        if (name_tok.type != TOK_IDENT && name_tok.type != TOK_STRING) {
            err.code = 2;
            err.message = "Expected module name after SCOPE MODULE";
            return err;
        }
        char mod_name[64];
        size_t mlen = name_tok.length < 63 ? name_tok.length : 63;
        if (name_tok.start) {
            runtime_memcpy(mod_name, name_tok.start, mlen);
        }
        mod_name[mlen] = '\0';

        scope_namespace_enter(vm, mod_name);
        return err;
    }

    if (runtime_strcasecmp(subcmd, "HOOK") == 0) {
        lex_next(lex);
        return handle_scope_hook(vm, lex);
    }

    if (runtime_strcasecmp(subcmd, "DISABLE") == 0) {
        lex_next(lex);
        BppToken kw_tok = lex_next(lex);
        if (kw_tok.type != TOK_IDENT && kw_tok.type != TOK_KEYWORD) {
            err.code = 2;
            err.message = "Expected keyword name after SCOPE DISABLE";
            return err;
        }
        char kw[64];
        size_t klen = kw_tok.length < 63 ? kw_tok.length : 63;
        if (kw_tok.start) {
            runtime_memcpy(kw, kw_tok.start, klen);
        }
        kw[klen] = '\0';

        scope_keyword_disable(vm, kw);
        vdev_printf(vm_get_vdev(vm), "Keyword '%s' disabled in current scope.\n", kw);
        return err;
    }

    if (runtime_strcasecmp(subcmd, "ENABLE") == 0) {
        lex_next(lex);
        BppToken kw_tok = lex_next(lex);
        if (kw_tok.type != TOK_IDENT && kw_tok.type != TOK_KEYWORD) {
            err.code = 2;
            err.message = "Expected keyword name after SCOPE ENABLE";
            return err;
        }
        char kw[64];
        size_t klen = kw_tok.length < 63 ? kw_tok.length : 63;
        if (kw_tok.start) {
            runtime_memcpy(kw, kw_tok.start, klen);
        }
        kw[klen] = '\0';

        scope_keyword_enable(vm, kw);
        vdev_printf(vm_get_vdev(vm), "Keyword '%s' enabled in current scope.\n", kw);
        return err;
    }

    if (runtime_strcasecmp(subcmd, "PRIVATE") == 0) {
        lex_next(lex);
        BppToken sym_tok = lex_next(lex);
        if (sym_tok.type != TOK_IDENT && sym_tok.type != TOK_KEYWORD) {
            err.code = 2;
            err.message = "Expected symbol name after SCOPE PRIVATE";
            return err;
        }
        char sym[64];
        size_t slen = sym_tok.length < 63 ? sym_tok.length : 63;
        if (sym_tok.start) {
            runtime_memcpy(sym, sym_tok.start, slen);
        }
        sym[slen] = '\0';

        scope_set_symbol_private(vm, sym);
        vdev_printf(vm_get_vdev(vm), "Symbol '%s' marked private.\n", sym);
        return err;
    }

    if (runtime_strcasecmp(subcmd, "PROTECT") == 0) {
        lex_next(lex);
        BppToken sym_tok = lex_next(lex);
        if (sym_tok.type != TOK_IDENT && sym_tok.type != TOK_KEYWORD) {
            err.code = 2;
            err.message = "Expected symbol name after SCOPE PROTECT";
            return err;
        }
        char sym[64];
        size_t slen = sym_tok.length < 63 ? sym_tok.length : 63;
        if (sym_tok.start) {
            runtime_memcpy(sym, sym_tok.start, slen);
        }
        sym[slen] = '\0';

        scope_protect_symbol(vm, sym);
        vdev_printf(vm_get_vdev(vm), "Symbol '%s' protected.\n", sym);
        return err;
    }

    err.code = 2;
    err.message = "Unknown SCOPE sub-command";
    return err;
}
