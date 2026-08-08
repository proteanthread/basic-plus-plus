/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file keyword.c
 * @brief Implementation of KEYWORD statement handler and property management sub-commands.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Implements the KEYWORD statement allowing users to query, set, list, and clear keyword properties:
 *   KEYWORD <kw> SET <prop> = <val>, KEYWORD <kw> GET <prop>, KEYWORD <kw> LIST, KEYWORD CLEAR.
 * - Why it exists: Provides dynamic keyword introspection and attribute management at runtime.
 * - Why it works this way: Parses sub-commands case-insensitively and updates keyword property state.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Additional KEYWORD sub-commands (e.g. DELETE, IMPORT).
 * - What cannot be changed: Token sequence validation and error code returns.
 * - What to expect: Sub-command parsing outputs error 2 (ERR_SYNTAX) on invalid syntax.
 * - What to do if something breaks: Check token type checking and string buffer bounds.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: C17 standard compliance across MSVC and GCC targets.
 * - Portability concerns: None.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - How future expansion can occur safely: Register property change listeners for IDE autocomplete plugins.
 *
 * SECTION 5: COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES
 * - Prerequisite C Source Files: `engine/src/runtime/keyword_props.c` (KEYWORD property metadata table), `engine/src/lexer/lexer.c` (keyword parsing), `engine/src/device/vdev.c` (LIST output).
 * - Prerequisite Header Surfaces: `engine/include/statements/dialect/keyword.h`, `engine/include/runtime/keyword_props.h`, `engine/include/lexer/lexer.h`, `engine/include/device/vdev.h`.
 */

#include "statements/dialect/keyword.h"
#include "runtime/keyword_props.h"
#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "device/vdev.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(_MSC_VER)
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#endif

BppError stmt_keyword_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

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
    memcpy(first_word, tok.start, len);
    first_word[len] = '\0';

    if (strcasecmp(first_word, "CLEAR") == 0) {
        lex_next(lex);
        kw_props_clear(vm);
        vdev_printf(vm_get_vdev(vm), "All keyword properties cleared.\n");
        return err;
    }

    if (strcasecmp(first_word, "LIST") == 0) {
        lex_next(lex);
        kw_props_list(vm, NULL);
        return err;
    }

    /* KEYWORD <target_kw> SET <prop> = <val> | GET <prop> | LIST */
    BppToken target_tok = lex_next(lex);
    if (target_tok.type != TOK_IDENT && target_tok.type != TOK_KEYWORD && target_tok.type != TOK_STRING) {
        err.code = 2;
        err.message = "Expected target keyword name after KEYWORD";
        return err;
    }
    char target_kw[64];
    size_t tlen = target_tok.length < 63 ? target_tok.length : 63;
    memcpy(target_kw, target_tok.start, tlen);
    target_kw[tlen] = '\0';

    BppToken action_tok = lex_next(lex);
    if (action_tok.type == TOK_EOF || action_tok.type == TOK_EOL) {
        /* Default to list for this keyword */
        kw_props_list(vm, target_kw);
        return err;
    }

    char action[64];
    size_t alen = action_tok.length < 63 ? action_tok.length : 63;
    memcpy(action, action_tok.start, alen);
    action[alen] = '\0';

    if (strcasecmp(action, "LIST") == 0) {
        kw_props_list(vm, target_kw);
        return err;
    }

    if (strcasecmp(action, "GET") == 0) {
        BppToken prop_tok = lex_next(lex);
        if (prop_tok.type != TOK_IDENT && prop_tok.type != TOK_STRING) {
            err.code = 2;
            err.message = "Expected property name after KEYWORD <kw> GET";
            return err;
        }
        char prop_name[64];
        size_t plen = prop_tok.length < 63 ? prop_tok.length : 63;
        memcpy(prop_name, prop_tok.start, plen);
        prop_name[plen] = '\0';

        const char *val = kw_props_get_property(vm, target_kw, prop_name);
        if (val) {
            vdev_printf(vm_get_vdev(vm), "KEYWORD %s %s = %s\n", target_kw, prop_name, val);
        } else {
            vdev_printf(vm_get_vdev(vm), "Property '%s' not set on KEYWORD %s\n", prop_name, target_kw);
        }
        return err;
    }

    if (strcasecmp(action, "SET") == 0) {
        BppToken prop_tok = lex_next(lex);
        if (prop_tok.type != TOK_IDENT && prop_tok.type != TOK_STRING) {
            err.code = 2;
            err.message = "Expected property name after KEYWORD <kw> SET";
            return err;
        }
        char prop_name[64];
        size_t plen = prop_tok.length < 63 ? prop_tok.length : 63;
        memcpy(prop_name, prop_tok.start, plen);
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
        memcpy(prop_val, val_tok.start, vlen);
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
        .syntax = "KEYWORD kw_name [SET prop = val | GET prop | LIST] | KEYWORD CLEAR",
        .help_text = "Configures dynamic properties and metadata attributes for language keywords.",
        .error_codes = "Error 2: Syntax Error"
    };
    microlib_register(&meta);
}

