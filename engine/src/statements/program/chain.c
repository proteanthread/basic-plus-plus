// FILENAME: chain.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (ctype.h, ctype.c, memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c, variables.h, variables.c)
// NEEDS: libengine (chain.h, eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h)
// NEEDS: libkernel (errors.h, security.h, security.c, vdev.h, vdev.c)
// Provides runtime implementation for the CHAIN statement in BASIC++.
//
// ---- Includes ----

#include "statements/program/chain.h"
#include "types/errors.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "memory/memory.h"
#include "runtime/variables.h"
#include "runtime/strings.h"
#include "device/vdev.h"
#include "security/security.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/ctype/ctype.h"

static const LangDesc g_chain_desc = {
    .name = "CHAIN",
    .category = "Program Mgmt & Editing",
    .syntax = "CHAIN [MERGE] filename_expr [, [line_number] [, ALL] [, MERGE]]",
    .description = "Passes control to another program file with optional variable preservation and line merging.",
    .error_summary = "Error 2: Syntax Error, Error 53: File Not Found, Error 8: Undefined Line Number",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_STATEMENT
};

#if defined(_MSC_VER)
#define runtime_strcasecmp runtime_strcasecmp
#define runtime_strncasecmp runtime_strncasecmp
#endif

extern BppError vm_load_program_file(VMContext *vm, const char *filename);

BppError stmt_chain_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    if (!vm || !lex) {
        err.code = ERR_ILLEGAL_FUNCTION_CALL;
        return err;
    }

    bool merge_flag = false;
    bool all_flag = false;
    double target_line = 0.0;

    // Check for leading MERGE option e.g. CHAIN MERGE "filename"
    BppToken peek = lex_peek(lex);
    if ((peek.type == TOK_KEYWORD && peek.as.keyword == KW_MERGE) ||
        (peek.type == TOK_IDENT && peek.length == 5 && runtime_strncasecmp(peek.start, "MERGE", 5) == 0)) {
        merge_flag = true;
        lex_next(lex);
    }

    // Evaluate filename expression
    BValue fn_val = eval_expression(vm, lex, &err);
    if (err.code != 0) {
        return err;
    }
    if (fn_val.type != VAL_STRING || !fn_val.as.string) {
        if (fn_val.type == VAL_STRING && fn_val.as.string) {
            str_release(vm_get_str(vm), fn_val.as.string);
        }
        err.code = ERR_TYPE_MISMATCH;
        return err;
    }

    // Parse optional parameters: [, [line_number] [, ALL] [, MERGE]]
    peek = lex_peek(lex);
    if (peek.type == TOK_COMMA) {
        lex_next(lex); // Consume ','
        peek = lex_peek(lex);

        // Optional starting line number
        if (peek.type == TOK_NUMBER) {
            peek = lex_next(lex);
            target_line = peek.as.number;
        } else if (peek.type != TOK_COMMA && peek.type != TOK_EOL && peek.type != TOK_EOF) {
            BppError line_err;
            runtime_memset(&line_err, 0, sizeof(line_err));
            BValue line_val = eval_expression(vm, lex, &line_err);
            if (line_err.code == 0) {
                if (line_val.type == VAL_NUMBER) {
                    target_line = line_val.as.number;
                } else if (line_val.type == VAL_STRING && line_val.as.string) {
                    str_release(vm_get_str(vm), line_val.as.string);
                }
            }
        }

        // Check for option flags after comma
        peek = lex_peek(lex);
        if (peek.type == TOK_COMMA) {
            lex_next(lex); // Consume ','
            peek = lex_peek(lex);
            if (peek.type == TOK_IDENT && peek.length == 3 && runtime_strncasecmp(peek.start, "ALL", 3) == 0) {
                all_flag = true;
                lex_next(lex);
            } else if ((peek.type == TOK_KEYWORD && peek.as.keyword == KW_MERGE) ||
                       (peek.type == TOK_IDENT && peek.length == 5 && runtime_strncasecmp(peek.start, "MERGE", 5) == 0)) {
                merge_flag = true;
                lex_next(lex);
            }

            // Check for additional option flag after third comma
            peek = lex_peek(lex);
            if (peek.type == TOK_COMMA) {
                lex_next(lex); // Consume ','
                peek = lex_peek(lex);
                if (peek.type == TOK_IDENT && peek.length == 3 && runtime_strncasecmp(peek.start, "ALL", 3) == 0) {
                    all_flag = true;
                    lex_next(lex);
                } else if ((peek.type == TOK_KEYWORD && peek.as.keyword == KW_MERGE) ||
                           (peek.type == TOK_IDENT && peek.length == 5 && runtime_strncasecmp(peek.start, "MERGE", 5) == 0)) {
                    merge_flag = true;
                    lex_next(lex);
                }
            }
        }
    }

    const char *filename = str_data(fn_val.as.string);

    // 1. Program Memory Overlay vs Clear
    if (!merge_flag) {
        mem_program_clear(vm_get_mem(vm));
    }

    // 2. Variable Preservation: ALL or MERGE preserves all variables; default keeps COMMON variables
    if (!all_flag && !merge_flag) {
        var_clear_for_chain(vm_get_var(vm));
    }

    // 3. Reset Control Flow Stack
    vm_reset_for_run(vm);

    // 4. Load Program Lines
    BppError load_err = vm_load_program_file(vm, filename);
    str_release(vm_get_str(vm), fn_val.as.string);
    if (load_err.code != 0) {
        return load_err;
    }

    // 5. Target Line Jump
    if (target_line > 0.0) {
        vm_set_start_line(vm, target_line);
    }

    // 6. Resume Program Execution
    vm_run_program(vm);
    return err;
}

void stmt_chain_register(void) {
    lang_desc_register(&g_chain_desc);
}


