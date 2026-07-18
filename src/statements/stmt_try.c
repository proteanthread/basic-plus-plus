/**
 * @file stmt_try.c
 * @brief TRY, CATCH, END TRY, THROW, ALIAS, and METADATA statement handlers.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Implements structured exception handling blocks (TRY/CATCH/END TRY),
 *   explicit exceptions throwing (THROW), dynamic keyword aliases (ALIAS), and
 *   runtime metadata block registration (METADATA REGISTER).
 * - Why it exists: Provides modern programming features to BASIC++ while maintaining compatibility with legacy ON ERROR GOTO.
 * - Why it works this way: TRY pushes active execution catch contexts on a TryStack. THROW triggers VM errors.
 *   ALIAS registers text macros in a dynamic registry.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Nesting limit parameters, default error message strings.
 * - What cannot be changed: TryFrame stack recovery logic without updating vm_stack.c.
 * - What to expect: Errors occurring within a TRY block will bubble up to the nearest CATCH block.
 * - What to do if something breaks: Trace TryStack pushes and pops, verify restored stack depths.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: TryStack is initialized at VM boot.
 * - Portability concerns: ANSI/ISO C17 compliant.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - How future expansion can occur safely: Add user-defined exception class names.
 * - How to write external extensions: Custom statement handlers can throw errors to trigger active CATCH frames.
 */

#include "bpp_config.h"
#include "bpp_vm.h"
#include "bpp_lexer.h"
#include "bpp_strings.h"
#include "bpp_dialect.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

extern BValue eval_expression(VMContext *vm, LexerContext *lex, BppError *out_err);
extern void vm_register_alias(VMContext *vm, const char *name, const char *expansion);

/* TRY statement handler */
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

    /* Scan forward to locate matching CATCH and END TRY statements */
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

    /* Push exception frame on TryStack */
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

/* CATCH statement handler */
BppError stmt_catch_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    (void)lex;

    BppTryFrame frame;
    TryStack *tstack = vm_get_try_stack(vm);
    if (try_stack_peek(tstack, &frame) && frame.catch_line == vm_get_current_line(vm)) {
        /* Reached normally: skip the CATCH block entirely and jump to END TRY */
        try_stack_pop(tstack, NULL);
        vm_jump(vm, frame.end_try_line, frame.end_try_pos);
    }

    return err;
}

/* END TRY statement handler */
BppError stmt_end_try_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    (void)vm;
    (void)lex;
    return err;
}

/* THROW statement handler */
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
        lex_next(lex); /* Consume comma */
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

/* ALIAS statement handler */
BppError stmt_alias_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BValue name_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (name_val.type != VAL_STRING) {
        err.code = 2;
        err.message = "Expected string expression for ALIAS name";
        return err;
    }

    BppToken comma = lex_next(lex);
    if (comma.type != TOK_COMMA) {
        err.code = 2;
        err.message = "Expected ',' after ALIAS name";
        return err;
    }

    BValue exp_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (exp_val.type != VAL_STRING) {
        err.code = 2;
        err.message = "Expected string expression for ALIAS expansion";
        return err;
    }

    const char *name_str = name_val.as.string ? str_data(name_val.as.string) : "";
    const char *exp_str = exp_val.as.string ? str_data(exp_val.as.string) : "";

    char name_buf[64];
    strncpy(name_buf, name_str, sizeof(name_buf) - 1);
    name_buf[sizeof(name_buf) - 1] = '\0';

    char exp_buf[256];
    strncpy(exp_buf, exp_str, sizeof(exp_buf) - 1);
    exp_buf[sizeof(exp_buf) - 1] = '\0';

    BppDialect *d = vm_get_defining_dialect(vm);
    if (d) {
        if (d->keyword_count < 256) {
            snprintf(d->keywords[d->keyword_count].name, sizeof(d->keywords[d->keyword_count].name), "%.63s", name_buf);
            snprintf(d->keywords[d->keyword_count].mapped_to, sizeof(d->keywords[d->keyword_count].mapped_to), "%.63s", exp_buf);
            d->keywords[d->keyword_count].id = KW_NONE;
            d->keyword_count++;
        }
    } else {
        vm_register_alias(vm, name_buf, exp_buf);
        /* Dynamically register custom keyword in lexer */
        keyword_register_custom(name_buf);
    }

    return err;
}

/* METADATA statement handler */
BppError stmt_metadata_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken reg_tok = lex_peek(lex);
    if (reg_tok.type != TOK_IDENT || reg_tok.length != 8 || strncasecmp(reg_tok.start, "REGISTER", 8) != 0) {
        /* Metaprogramming block, skipped by execution */
        return err;
    }
    lex_next(lex); /* Consume 'REGISTER' */

    BValue type_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (type_val.type != VAL_STRING) {
        err.code = 2;
        err.message = "Expected string for METADATA REGISTER type";
        return err;
    }

    BppToken comma = lex_next(lex);
    if (comma.type != TOK_COMMA) { err.code = 2; err.message = "Expected ','"; return err; }

    BValue target_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (target_val.type != VAL_STRING) {
        err.code = 2;
        err.message = "Expected string for METADATA REGISTER target";
        return err;
    }

    comma = lex_next(lex);
    if (comma.type != TOK_COMMA) { err.code = 2; err.message = "Expected ','"; return err; }

    BValue doc_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (doc_val.type != VAL_STRING) {
        err.code = 2;
        err.message = "Expected string for METADATA REGISTER doc";
        return err;
    }

    comma = lex_next(lex);
    if (comma.type != TOK_COMMA) { err.code = 2; err.message = "Expected ','"; return err; }

    BValue body_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (body_val.type != VAL_STRING) {
        err.code = 2;
        err.message = "Expected string for METADATA REGISTER body";
        return err;
    }

    const char *type_str = type_val.as.string ? str_data(type_val.as.string) : "";
    const char *target_str = target_val.as.string ? str_data(target_val.as.string) : "";
    const char *doc_str = doc_val.as.string ? str_data(doc_val.as.string) : "";
    const char *body_str = body_val.as.string ? str_data(body_val.as.string) : "";

    BppMetadataRegistry *reg = vm_get_metadata(vm);
    if (!metadata_register_block(reg, type_str, target_str, doc_str, body_str)) {
        err.code = 14;
        err.message = "Failed to register metadata block";
        return err;
    }

    return err;
}
