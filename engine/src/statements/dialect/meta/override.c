// FILENAME: override.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (exec_control_internal.h, exec_internal.h)
// NEEDS: libcore (memops.h, memops.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c)
// NEEDS: libcore (strops.h, strops.c)
// NEEDS: libengine (lexer.h, lexer.c, override.h, vm.h)
// NEEDS: libkernel (types.h, vdev.h, vdev.c)
// Provides runtime implementation for the OVERRIDE statement in BASIC++.
//
// ---- Includes ----

#include "statements/dialect/meta/override.h"
#include "runtime/override.h"
#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "device/vdev.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"

static bool is_protected_override_keyword(const char *name) {
    if (!name) return false;
    static const char *protected_kws[] = {
        "SCOPE", "ALIAS", "KEYWORD", "OVERRIDE",
        "REM", "END", "STOP", "NEW", "RUN", "CLEAR", NULL
    };
    for (int i = 0; protected_kws[i] != NULL; ++i) {
        if (runtime_strcasecmp(name, protected_kws[i]) == 0) return true;
    }
    return false;
}

BppError stmt_override_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    if (!vm || !lex) {
        err.code = 1;
        err.message = "Null pointer context in OVERRIDE handler";
        return err;
    }

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_EOF || tok.type == TOK_EOL) {
        err.code = 2;
        err.message = "Expected OVERRIDE target keyword or CLEAR";
        return err;
    }

    char first_word[64];
    size_t len = tok.length < 63 ? tok.length : 63;
    if (tok.start) {
        runtime_memcpy(first_word, tok.start, len);
    }
    first_word[len] = '\0';

    if (runtime_strcasecmp(first_word, "CLEAR") == 0) {
        lex_next(lex);
        override_clear(vm);
        vdev_printf(vm_get_vdev(vm), "All statement overrides cleared.\n");
        return err;
    }

    BppToken target_tok = lex_next(lex);
    char target_name[64];
    size_t tlen = target_tok.length < 63 ? target_tok.length : 63;
    if (target_tok.start) {
        runtime_memcpy(target_name, target_tok.start, tlen);
    }
    target_name[tlen] = '\0';

    if (is_protected_override_keyword(target_name)) {
        err.code = 13;
        err.message = "Cannot override protected system keyword";
        return err;
    }

    BppToken with_tok = lex_next(lex);
    char with_word[64];
    size_t wlen = with_tok.length < 63 ? with_tok.length : 63;
    if (with_tok.start) {
        runtime_memcpy(with_word, with_tok.start, wlen);
    }
    with_word[wlen] = '\0';

    if (runtime_strcasecmp(with_word, "WITH") != 0) {
        err.code = 2;
        err.message = "Expected WITH after OVERRIDE <target>";
        return err;
    }

    BppToken next_tok = lex_peek(lex);
    char next_word[64];
    size_t nlen = next_tok.length < 63 ? next_tok.length : 63;
    if (next_tok.start) {
        runtime_memcpy(next_word, next_tok.start, nlen);
    }
    next_word[nlen] = '\0';

    if (runtime_strcasecmp(next_word, "GOSUB") == 0) {
        lex_next(lex);
        BppToken line_tok = lex_next(lex);
        if (line_tok.type != TOK_NUMBER) {
            err.code = 2;
            err.message = "Expected line number after GOSUB in OVERRIDE";
            return err;
        }
        long line_num = (long)line_tok.as.number;
        override_register(vm, target_name, line_num, NULL);
        vdev_printf(vm_get_vdev(vm), "Overrode %s WITH GOSUB %ld\n", target_name, line_num);
        return err;
    }

    BppToken sub_tok = lex_next(lex);
    char sub_name[64];
    size_t slen = sub_tok.length < 63 ? sub_tok.length : 63;
    if (sub_tok.start) {
        runtime_memcpy(sub_name, sub_tok.start, slen);
    }
    sub_name[slen] = '\0';

    override_register(vm, target_name, 0, sub_name);
    vdev_printf(vm_get_vdev(vm), "Overrode %s WITH SUB %s\n", target_name, sub_name);
    return err;
}

void stmt_override_register(void) {
    static const MicroLibMetadata meta = {
        .name = "OVERRIDE",
        .category = "Introspection",
        .syntax = "OVERRIDE target WITH GOSUB line_num | OVERRIDE target WITH sub_name | OVERRIDE CLEAR",
        .help_text = "Replaces or hooks built-in statement execution with user-defined SUB or GOSUB routines.",
        .error_codes = "Error 2: Syntax Error, Error 13: Permission Denied (Protected Keyword)"
    };
    microlib_register(&meta);
}
