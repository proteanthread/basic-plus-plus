// FILENAME: keyword.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (exec_internal.h)
// NEEDS: libcore (keyword_props.h, keyword_props.c, memops.h, memops.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c)
// NEEDS: libcore (strops.h, strops.c)
// NEEDS: libengine (keyword.h, lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h, vdev.h, vdev.c)
// Provides runtime implementation for the KEYWORD statement in BASIC++.
//
// ---- Includes ----

#include "statements/dialect/meta/keyword.h"
#include "runtime/keyword_props.h"
#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "device/vdev.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"

BppError stmt_keyword_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    if (!vm || !lex) {
        err.code = 1;
        err.message = "Null pointer context in KEYWORD handler";
        return err;
    }

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_EOF || tok.type == TOK_EOL) {
        err.code = 2;
        err.message = "Expected KEYWORD sub-command or target keyword name";
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
        kw_props_clear(vm);
        vdev_printf(vm_get_vdev(vm), "All keyword properties cleared.\n");
        return err;
    }

    if (runtime_strcasecmp(first_word, "LIST") == 0) {
        lex_next(lex);
        kw_props_list(vm, NULL);
        return err;
    }

    // KEYWORD <target_kw> SET <prop> = <val> | GET <prop> | LIST
    BppToken target_tok = lex_next(lex);
    if (target_tok.type != TOK_IDENT && target_tok.type != TOK_KEYWORD && target_tok.type != TOK_STRING) {
        err.code = 2;
        err.message = "Expected target keyword name after KEYWORD";
        return err;
    }
    char target_kw[64];
    size_t tlen = target_tok.length < 63 ? target_tok.length : 63;
    if (target_tok.start) {
        runtime_memcpy(target_kw, target_tok.start, tlen);
    }
    target_kw[tlen] = '\0';

    BppToken action_tok = lex_next(lex);
    if (action_tok.type == TOK_EOF || action_tok.type == TOK_EOL) {
        // Default to list for this keyword
        kw_props_list(vm, target_kw);
        return err;
    }

    char action[64];
    size_t alen = action_tok.length < 63 ? action_tok.length : 63;
    if (action_tok.start) {
        runtime_memcpy(action, action_tok.start, alen);
    }
    action[alen] = '\0';

    if (runtime_strcasecmp(action, "LIST") == 0) {
        kw_props_list(vm, target_kw);
        return err;
    }

    if (runtime_strcasecmp(action, "GET") == 0) {
        BppToken prop_tok = lex_next(lex);
        if (prop_tok.type != TOK_IDENT && prop_tok.type != TOK_STRING) {
            err.code = 2;
            err.message = "Expected property name after KEYWORD <kw> GET";
            return err;
        }
        char prop_name[64];
        size_t plen = prop_tok.length < 63 ? prop_tok.length : 63;
        if (prop_tok.start) {
            runtime_memcpy(prop_name, prop_tok.start, plen);
        }
        prop_name[plen] = '\0';

        const char *val = kw_props_get_property(vm, target_kw, prop_name);
        if (val) {
            vdev_printf(vm_get_vdev(vm), "KEYWORD %s %s = %s\n", target_kw, prop_name, val);
        } else {
            vdev_printf(vm_get_vdev(vm), "Property '%s' not set on KEYWORD %s\n", prop_name, target_kw);
        }
        return err;
    }

    if (runtime_strcasecmp(action, "SET") == 0) {
        BppToken prop_tok = lex_next(lex);
        if (prop_tok.type != TOK_IDENT && prop_tok.type != TOK_STRING) {
            err.code = 2;
            err.message = "Expected property name after KEYWORD <kw> SET";
            return err;
        }
        char prop_name[64];
        size_t plen = prop_tok.length < 63 ? prop_tok.length : 63;
        if (prop_tok.start) {
            runtime_memcpy(prop_name, prop_tok.start, plen);
        }
        prop_name[plen] = '\0';

        BppToken eq_tok = lex_peek(lex);
        if (eq_tok.type == TOK_EQ) {
            lex_next(lex);
        }

        BppToken val_tok = lex_next(lex);
        if (val_tok.type == TOK_EOF || val_tok.type == TOK_EOL) {
            err.code = 2;
            err.message = "Expected property value after '='";
            return err;
        }
        char prop_val[256];
        size_t vlen = val_tok.length < 255 ? val_tok.length : 255;
        if (val_tok.start) {
            runtime_memcpy(prop_val, val_tok.start, vlen);
        }
        prop_val[vlen] = '\0';

        kw_props_set_property(vm, target_kw, prop_name, prop_val);
        vdev_printf(vm_get_vdev(vm), "Set property '%s' = '%s' on KEYWORD %s\n", prop_name, prop_val, target_kw);
        return err;
    }

    err.code = 2;
    err.message = "Expected SET, GET, LIST, or CLEAR after KEYWORD <kw>";
    return err;
}

void stmt_keyword_register(void) {
    static const MicroLibMetadata meta = {
        .name = "KEYWORD",
        .category = "Introspection",
        .syntax = "KEYWORD target SET prop = val | KEYWORD target GET prop | KEYWORD target LIST | KEYWORD CLEAR",
        .help_text = "Configures dynamic syntactic and semantic properties for engine keywords.",
        .error_codes = "Error 2: Syntax Error"
    };
    microlib_register(&meta);
}
