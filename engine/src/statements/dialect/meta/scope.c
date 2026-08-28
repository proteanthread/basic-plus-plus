// FILENAME: scope.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (exec_control_internal.h, exec_internal.h, module.c)
// NEEDED BY: libengine (sub_internal.h)
// NEEDS: libcore (memops.h, memops.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c)
// NEEDS: libcore (strops.h, strops.c)
// NEEDS: libengine (alias.h, alias.c, lexer.h, lexer.c, scope.h, vm.h)
// NEEDS: libkernel (types.h, vdev.h, vdev.c)
// Provides runtime implementation for the SCOPE statement in BASIC++.
//
// ---- Includes ----

#include "statements/dialect/meta/scope.h"
#include "scope/scope.h"
#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "device/vdev.h"
#include "statements/dialect/meta/alias.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"

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
        scope_register_hook(vm, cmd_name, hook_type, NULL, target_tok.as.number);
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
        err.message = "Expected SCOPE sub-command (DISABLE, ENABLE, HOOK, PRIVATE, MODULE, BEGIN, END, PROTECT)";
        return err;
    }

    char subcmd[64];
    size_t len = tok.length < 63 ? tok.length : 63;
    if (tok.start) {
        runtime_memcpy(subcmd, tok.start, len);
    }
    subcmd[len] = '\0';

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

        if (vm_is_protected_keyword(kw)) {
            err.code = 2;
            err.message = "Cannot disable protected system keyword";
            return err;
        }

        scope_keyword_disable(vm, kw);
        vdev_printf(vm_get_vdev(vm), "Keyword '%s' disabled.\n", kw);
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
        vdev_printf(vm_get_vdev(vm), "Keyword '%s' enabled.\n", kw);
        return err;
    }

    if (runtime_strcasecmp(subcmd, "HOOK") == 0) {
        lex_next(lex);
        return handle_scope_hook(vm, lex);
    }

    if (runtime_strcasecmp(subcmd, "PRIVATE") == 0) {
        lex_next(lex);
        BppToken sym_tok = lex_next(lex);
        if (sym_tok.type != TOK_IDENT) {
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
        vdev_printf(vm_get_vdev(vm), "Symbol '%s' set to PRIVATE.\n", sym);
        return err;
    }

    if (runtime_strcasecmp(subcmd, "MODULE") == 0) {
        lex_next(lex);
        BppToken ns_tok = lex_next(lex);
        if (ns_tok.type != TOK_IDENT && ns_tok.type != TOK_STRING) {
            err.code = 2;
            err.message = "Expected namespace name after SCOPE MODULE";
            return err;
        }
        char ns[64];
        size_t nslen = ns_tok.length < 63 ? ns_tok.length : 63;
        if (ns_tok.start) {
            runtime_memcpy(ns, ns_tok.start, nslen);
        }
        ns[nslen] = '\0';

        scope_namespace_enter(vm, ns);
        vdev_printf(vm_get_vdev(vm), "Entered scope namespace '%s'.\n", ns);
        return err;
    }

    if (runtime_strcasecmp(subcmd, "BEGIN") == 0) {
        lex_next(lex);
        scope_block_push(vm);
        vdev_printf(vm_get_vdev(vm), "Scope block pushed (depth %d).\n", scope_get_block_depth(vm));
        return err;
    }

    if (runtime_strcasecmp(subcmd, "END") == 0) {
        lex_next(lex);
        scope_block_pop(vm);
        vdev_printf(vm_get_vdev(vm), "Scope block popped (depth %d).\n", scope_get_block_depth(vm));
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

void stmt_scope_register(void) {
    static const MicroLibMetadata meta = {
        .name = "SCOPE",
        .category = "Introspection",
        .syntax = "SCOPE [BEGIN | END | DISABLE kw | ENABLE kw | HOOK ... | MODULE name | PRIVATE sym]",
        .help_text = "Manages lexical scope blocks, keyword enablement, execution hooks, and symbol protection.",
        .error_codes = "Error 2: Syntax Error, Error 13: Permission Denied"
    };
    microlib_register(&meta);
}
