// FILENAME: try.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (ctype.h, ctype.c, dialect.h, dialect.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (lexer.h, lexer.c, string.c, vm.h)
// NEEDS: libkernel (config.h)
// Provides runtime implementation for the TRY statement in BASIC++.
//
// ---- Includes ----

#include "types/config.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "runtime/strings.h"
#include "core/dialect.h"
#include "runtime/micro_lib_metadata.h"

void stmt_try_register(void) {
    MicroLibMetadata meta = {
        .name = "TRY",
        .category = "Event Trapping",
        .syntax = "TRY ... CATCH err_var ... FINALLY ... END TRY",
        .help_text = "Structured exception-handling block supporting TRY, CATCH, FINALLY, and END TRY.",
        .error_codes = "Error 2: Syntax Error, Error 35: TRY Without CATCH/FINALLY"
    };
    microlib_register(&meta);
}
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

extern BValue eval_expression(VMContext *vm, LexerContext *lex, BppError *out_err);
extern void vm_register_alias(VMContext *vm, const char *name, const char *expansion);

// TRY statement handler
BppError stmt_try_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    (void)lex;

    MemoryContext *mem = vm_get_mem(vm);
    size_t count = 0;
    BppProgramLine *lines = mem_program_get_all(mem, &count);
    BppLineNumber current_ln = vm_get_current_line(vm);

    size_t start_idx = 0;
    bool found = false;
    for (size_t i = 0; i < count; ++i) {
        if (lines[i].line_number == current_ln) {
            start_idx = i;
            found = true;
            break;
        }
    }

    if (!found) {
        err.code = 11;
        err.message = "Execution state corruption during TRY scan";
        return err;
    }

    // Scan forward to locate matching CATCH and END TRY statements
    int try_nesting = 0;
    BppLineNumber catch_ln = 0.0;
    const char *catch_pos = NULL;
    BppLineNumber end_try_ln = 0.0;
    const char *end_try_pos = NULL;

    for (size_t i = start_idx + 1; i < count; ++i) {
        LexerContext *scan_lex = lex_init(mem, lines[i].text);
        if (!scan_lex) {
            err.code = 14;
            err.message = "Out of memory during TRY block scan";
            return err;
        }
        BppToken tok = lex_next(scan_lex);
        if (tok.type == TOK_KEYWORD) {
            if (tok.as.keyword == KW_TRY) {
                try_nesting++;
            } else if (tok.as.keyword == KW_CATCH) {
                if (try_nesting == 0) {
                    catch_ln = lines[i].line_number;
                    catch_pos = lines[i].text;
                }
            } else if (tok.as.keyword == KW_END) {
                BppToken next_tok = lex_next(scan_lex);
                if (next_tok.type == TOK_KEYWORD && next_tok.as.keyword == KW_TRY) {
                    if (try_nesting > 0) {
                        try_nesting--;
                    } else {
                        end_try_ln = lines[i].line_number;
                        end_try_pos = lines[i].text;
                        lex_shutdown(scan_lex);
                        break;
                    }
                }
            }
        }
        lex_shutdown(scan_lex);
    }

    if (end_try_ln == 0.0) {
        err.code = 2;
        err.message = "TRY without matching END TRY";
        return err;
    }

    // Push exception frame on TryStack
    BppTryFrame frame;
    frame.catch_line = catch_ln;
    frame.catch_pos = catch_pos;
    frame.end_try_line = end_try_ln;
    frame.end_try_pos = end_try_pos;

    frame.gosub_stack_depth = vm_get_gosub_stack(vm) ? gosub_stack_depth(vm_get_gosub_stack(vm)) : 0;
    frame.for_stack_depth = vm_get_for_stack(vm) ? for_stack_depth(vm_get_for_stack(vm)) : 0;
    frame.while_stack_depth = vm_get_while_stack(vm) ? while_stack_depth(vm_get_while_stack(vm)) : 0;
    frame.do_stack_depth = vm_get_do_stack(vm) ? do_stack_depth(vm_get_do_stack(vm)) : 0;
    frame.select_stack_depth = vm_get_select_stack(vm) ? select_stack_depth(vm_get_select_stack(vm)) : 0;
    frame.sub_stack_depth = vm_get_sub_stack(vm) ? sub_stack_depth(vm_get_sub_stack(vm)) : 0;

    if (!try_stack_push(vm_get_try_stack(vm), frame)) {
        err.code = 24;
        err.message = "TryStack overflow (maximum nesting exceeded)";
        return err;
    }

    return err;
}

// CATCH statement handler
BppError stmt_catch_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppTryFrame frame;
    TryStack *tstack = vm_get_try_stack(vm);
    if (try_stack_peek(tstack, &frame) && frame.catch_line == vm_get_current_line(vm)) {
        // Reached normally: skip the CATCH block entirely and jump to END TRY
        try_stack_pop(tstack, NULL);
        vm_jump(vm, frame.end_try_line, frame.end_try_pos);
        return err;
    }

    if (lex) {
        BppToken tok = lex_peek(lex);
        if (tok.type == TOK_IDENT) {
            char var_name[64] = {0};
            size_t len = tok.length < sizeof(var_name) - 1 ? tok.length : sizeof(var_name) - 1;
            memcpy(var_name, tok.start, len);
            var_name[len] = '\0';
            lex_next(lex);

            BValue err_val;
            err_val.type = VAL_NUMBER;
            err_val.as.number = (double)vm_get_err_code(vm);
            var_assign(vm_get_var(vm), var_name, err_val);
        }
    }

    return err;
}

// END TRY statement handler
BppError stmt_end_try_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    (void)vm;
    (void)lex;
    return err;
}

// THROW statement handler
BppError stmt_throw_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BValue code_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    if (code_val.type != VAL_NUMBER) {
        err.code = 2;
        err.message = "Expected numeric error code for THROW";
        return err;
    }
    int code = (int)code_val.as.number;

    BppToken next = lex_peek(lex);
    char msg_buf[256] = "User Exception";
    if (next.type == TOK_COMMA) {
        lex_next(lex); // Consume comma
        BValue msg_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (msg_val.type != VAL_STRING) {
            err.code = 2;
            err.message = "Expected string error message for THROW";
            return err;
        }
        if (msg_val.as.string) {
            const char *raw_str = str_data(msg_val.as.string);
            strncpy(msg_buf, raw_str ? raw_str : "", sizeof(msg_buf) - 1);
            msg_buf[sizeof(msg_buf) - 1] = '\0';
        }
    }

    err.code = code;
    char *err_msg = (char *)mem_scratch_alloc(vm_get_mem(vm), strlen(msg_buf) + 1);
    if (err_msg) {
        memcpy(err_msg, msg_buf, strlen(msg_buf) + 1);
        err.message = err_msg;
    } else {
        err.message = "User Exception";
    }
    return err;
}

