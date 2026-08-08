/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file scope.c
 * @brief Implementation of SCOPE statement handler and sub-commands.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Implements the SCOPE statement handling all sub-commands:
 *   SCOPE DISABLE <kw>, SCOPE ENABLE <kw>, SCOPE HOOK BEFORE/AFTER/OVERRIDE <cmd> GOSUB <line/label>,
 *   SCOPE PRIVATE <symbol>, SCOPE MODULE <name>, SCOPE BEGIN, SCOPE END, SCOPE PROTECT <name>.
 * - Why it exists: Provides language governance, block scoping, sandboxing, and execution intercept hooks.
 * - Why it works this way: Parses sub-commands case-insensitively and updates SCOPE state.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Sub-command syntax extensions.
 * - What cannot be changed: Protected keyword constraints (protected keywords cannot be disabled/overridden).
 * - What to expect: Sub-command parsing outputs error 2 (ERR_SYNTAX) on invalid syntax.
 * - What to do if something breaks: Check token type checking and string copying bounds.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: C17 standard compliance across MSVC and GCC targets.
 * - Portability concerns: None.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - How future expansion can occur safely: Add new sub-commands to the sub-command dispatcher loop.
 *
 * SECTION 5: COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES
 * - Prerequisite C Source Files: `engine/src/scope/scope.c` (scope state manager), `engine/src/lexer/lexer.c` (protected keyword lookup), `engine/src/vm/exec.c` (execution HOOK_BEFORE/AFTER runner).
 * - Prerequisite Header Surfaces: `engine/include/statements/dialect/scope.h`, `engine/include/scope/scope.h`, `engine/include/lexer/lexer.h`, `engine/include/vm/vm.h`.
 */

#include "statements/dialect/scope.h"
#include "scope/scope.h"
#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "device/vdev.h"
#include "statements/dialect/alias.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(_MSC_VER)
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#endif

static BppError handle_scope_hook(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken type_tok = lex_next(lex);
    char hook_type_str[64];
    size_t len = type_tok.length < 63 ? type_tok.length : 63;
    memcpy(hook_type_str, type_tok.start, len);
    hook_type_str[len] = '\0';

    BppHookType hook_type = HOOK_BEFORE;
    if (strcasecmp(hook_type_str, "BEFORE") == 0) {
        hook_type = HOOK_BEFORE;
    } else if (strcasecmp(hook_type_str, "AFTER") == 0) {
        hook_type = HOOK_AFTER;
    } else if (strcasecmp(hook_type_str, "OVERRIDE") == 0) {
        hook_type = HOOK_OVERRIDE;
    } else if (strcasecmp(hook_type_str, "CLEAR") == 0) {
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
    memcpy(cmd_name, cmd_tok.start, clen);
    cmd_name[clen] = '\0';

    BppToken gosub_tok = lex_next(lex);
    char gosub_str[64];
    size_t glen = gosub_tok.length < 63 ? gosub_tok.length : 63;
    memcpy(gosub_str, gosub_tok.start, glen);
    gosub_str[glen] = '\0';

    if (strcasecmp(gosub_str, "GOSUB") != 0) {
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
        memcpy(label_buf, target_tok.start, l_len);
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
    memset(&err, 0, sizeof(err));

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
    memcpy(subcmd, tok.start, len);
    subcmd[len] = '\0';

    if (strcasecmp(subcmd, "DISABLE") == 0) {
        lex_next(lex);
        BppToken kw_tok = lex_next(lex);
        if (kw_tok.type != TOK_IDENT && kw_tok.type != TOK_KEYWORD) {
            err.code = 2;
            err.message = "Expected keyword name after SCOPE DISABLE";
            return err;
        }
        char kw[64];
        size_t klen = kw_tok.length < 63 ? kw_tok.length : 63;
        memcpy(kw, kw_tok.start, klen);
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

    if (strcasecmp(subcmd, "ENABLE") == 0) {
        lex_next(lex);
        BppToken kw_tok = lex_next(lex);
        if (kw_tok.type != TOK_IDENT && kw_tok.type != TOK_KEYWORD) {
            err.code = 2;
            err.message = "Expected keyword name after SCOPE ENABLE";
            return err;
        }
        char kw[64];
        size_t klen = kw_tok.length < 63 ? kw_tok.length : 63;
        memcpy(kw, kw_tok.start, klen);
        kw[klen] = '\0';

        scope_keyword_enable(vm, kw);
        vdev_printf(vm_get_vdev(vm), "Keyword '%s' enabled.\n", kw);
        return err;
    }

    if (strcasecmp(subcmd, "HOOK") == 0) {
        lex_next(lex);
        return handle_scope_hook(vm, lex);
    }

    if (strcasecmp(subcmd, "PRIVATE") == 0) {
        lex_next(lex);
        BppToken sym_tok = lex_next(lex);
        if (sym_tok.type != TOK_IDENT) {
            err.code = 2;
            err.message = "Expected symbol name after SCOPE PRIVATE";
            return err;
        }
        char sym[64];
        size_t slen = sym_tok.length < 63 ? sym_tok.length : 63;
        memcpy(sym, sym_tok.start, slen);
        sym[slen] = '\0';

        scope_set_symbol_private(vm, sym);
        vdev_printf(vm_get_vdev(vm), "Symbol '%s' set to PRIVATE.\n", sym);
        return err;
    }

    if (strcasecmp(subcmd, "MODULE") == 0) {
        lex_next(lex);
        BppToken ns_tok = lex_next(lex);
        if (ns_tok.type != TOK_IDENT && ns_tok.type != TOK_STRING) {
            err.code = 2;
            err.message = "Expected namespace name after SCOPE MODULE";
            return err;
        }
        char ns[64];
        size_t nslen = ns_tok.length < 63 ? ns_tok.length : 63;
        memcpy(ns, ns_tok.start, nslen);
        ns[nslen] = '\0';

        scope_namespace_enter(vm, ns);
        vdev_printf(vm_get_vdev(vm), "Entered scope namespace '%s'.\n", ns);
        return err;
    }

    if (strcasecmp(subcmd, "BEGIN") == 0) {
        lex_next(lex);
        scope_block_push(vm);
        vdev_printf(vm_get_vdev(vm), "Scope block pushed (depth %d).\n", scope_get_block_depth(vm));
        return err;
    }

    if (strcasecmp(subcmd, "END") == 0) {
        lex_next(lex);
        scope_block_pop(vm);
        vdev_printf(vm_get_vdev(vm), "Scope block popped (depth %d).\n", scope_get_block_depth(vm));
        return err;
    }

    if (strcasecmp(subcmd, "PROTECT") == 0) {
        lex_next(lex);
        BppToken sym_tok = lex_next(lex);
        if (sym_tok.type != TOK_IDENT && sym_tok.type != TOK_KEYWORD) {
            err.code = 2;
            err.message = "Expected symbol name after SCOPE PROTECT";
            return err;
        }
        char sym[64];
        size_t slen = sym_tok.length < 63 ? sym_tok.length : 63;
        memcpy(sym, sym_tok.start, slen);
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

