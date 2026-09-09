// FILENAME: stmt_revert.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (exec_dispatch.c, common_reg_stmts.c)
// NEEDS: libcore (language_descriptor.h)
// NEEDS: libengine (vm.h, lexer.h, semantic_harvester.h, vdev.h)
// Provides runtime implementation for the REVERT statement in BASIC++.

#include "statements/program/stmt_revert.h"
#include "statements/program/modernize/semantic_harvester.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "device/vdev.h"
#include "runtime/language_descriptor.h"
#include "runtime/funcreg.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/format/snprintf.h"
#include "runtime/strings.h"

static const LangDesc g_revert_desc = {
    .name = "REVERT",
    .category = "Program Mgmt & Editing",
    .syntax = "REVERT [tool_tag$] | REVERT [ tool_tag$ ] | REVERT { tool: \"...\" }",
    .description = "Rolls back previous program operations from in-memory RAMbank snapshot ring stack.",
    .error_summary = "Error 5: Illegal Function Call",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_STATEMENT
};

BppError stmt_revert_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    if (!vm) return err;

    VDevContext *vd = vm_get_vdev(vm);
    char tag_buf[64] = {0};
    const char *target_tag = NULL;

    if (lex) {
        BppToken tok = lex_peek(lex);
        if (tok.type == TOK_RPN_LITERAL) {
            lex_next(lex);
            char map_buf[256];
            runtime_snprintf(map_buf, sizeof(map_buf), "%.*s", (int)tok.length, tok.start);
            char *colon = runtime_strchr(map_buf, ':');
            if (colon) {
                char *v = colon + 1;
                while (*v && (*v == ' ' || *v == '"')) v++;
                char *vend = v + runtime_strlen(v) - 1;
                while (vend > v && (*vend == ' ' || *vend == '"' || *vend == '}')) *vend-- = '\0';
                if (*v) {
                    runtime_strncpy(tag_buf, v, sizeof(tag_buf) - 1);
                    target_tag = tag_buf;
                }
            }
        } else if (tok.type == TOK_LBRACE) {
            lex_next(lex);
            while (lex_peek(lex).type != TOK_RBRACE && lex_peek(lex).type != TOK_EOF) {
                BppToken k = lex_next(lex);
                (void)k;
                if (lex_peek(lex).start && *lex_peek(lex).start == ':') lex_next(lex);
                if (lex_peek(lex).type == TOK_EOL) lex_next(lex);
                BppToken v = lex_next(lex);
                if (v.type == TOK_STRING || v.type == TOK_IDENT || v.type == TOK_KEYWORD) {
                    runtime_snprintf(tag_buf, sizeof(tag_buf), "%.*s", (int)v.length, v.start);
                    target_tag = tag_buf;
                }
                if (lex_peek(lex).type == TOK_COMMA) lex_next(lex);
            }
            if (lex_peek(lex).type == TOK_RBRACE) lex_next(lex);
        } else if (tok.type == TOK_LBRACKET) {
            lex_next(lex);
            BppToken b = lex_next(lex);
            if (b.type == TOK_STRING || b.type == TOK_IDENT || b.type == TOK_KEYWORD) {
                runtime_snprintf(tag_buf, sizeof(tag_buf), "%.*s", (int)b.length, b.start);
                target_tag = tag_buf;
            }
            while (lex_peek(lex).type != TOK_RBRACKET && lex_peek(lex).type != TOK_EOF) lex_next(lex);
            if (lex_peek(lex).type == TOK_RBRACKET) lex_next(lex);
        } else if (tok.type == TOK_IDENT || tok.type == TOK_KEYWORD || tok.type == TOK_STRING) {
            lex_next(lex);
            runtime_snprintf(tag_buf, sizeof(tag_buf), "%.*s", (int)tok.length, tok.start);
            target_tag = tag_buf;
        }
    }

    if (rambank_snapshot_pop(vm, target_tag) || rambank_snapshot_pop(vm, NULL)) {
        if (target_tag) {
            vdev_printf(vd, "REVERT: Successfully restored program from RAMbank snapshot for '%s'.\n", target_tag);
        } else {
            vdev_puts(vd, "REVERT: Successfully restored program from previous RAMbank snapshot.\n");
        }
    } else {
        if (target_tag) {
            vdev_printf(vd, "REVERT: No previous program snapshot available in RAMbank for '%s'.\n", target_tag);
        } else {
            vdev_puts(vd, "REVERT: No previous program snapshot available in RAMbank.\n");
        }
        err.code = 5;
        err.message = "No program snapshot available to revert";
    }

    return err;
}

BValue func_revert_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname; (void)err;
    BValue res;
    runtime_memset(&res, 0, sizeof(res));
    res.type = VAL_NUMBER;
    res.as.number = 0.0;

    if (!vm) return res;
    const char *tag = NULL;
    if (arg_count > 0 && args[0].type == VAL_STRING && args[0].as.string) {
        tag = str_data(args[0].as.string);
    }

    if (rambank_snapshot_pop(vm, tag) || rambank_snapshot_pop(vm, NULL)) {
        res.as.number = 1.0;
    }
    return res;
}

void stmt_revert_register(void) {
    lang_desc_register(&g_revert_desc);

    FunctionEntry entry = {
        .name = "REVERT",
        .keyword = KW_NONE,
        .category = FCAT_UTIL,
        .ret_type = FRET_INT,
        .min_args = 0,
        .max_args = 1,
        .safety = FSAFE_STATE,
        .overridable = 0,
        .handler = NULL,
        .help_text = "Rollback program snapshot: REVERT([tool_tag$])",
        .module_name = "Program"
    };
    funcreg_register(&entry);
}
