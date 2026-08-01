/**
 * @file stmt_flow.c
 * @brief Control flow statement command handlers (IF, GOTO, GOSUB, RETURN, END, REM).
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Implements control flow statements:
 *   - GOTO: Jumps execution directly to a line.
 *   - GOSUB/RETURN: Subroutine branches using a heap-allocated call stack.
 *   - IF/THEN/ELSE: Conditional branches supporting implicit line jumps and nested statements.
 *   - END: Halts program execution.
 *   - REM: Comments (ignored NOP).
 * - Why it exists: Provides branching, decisions, and subroutines required for BASIC execution.
 * - Why it works this way: It uses VM jump and GOSUB stack accessors. Jumps simply modify
 *   the VM context's next_line and next_pos parameters, allowing non-recursive execution.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Additional logical branches, syntax validation rules.
 * - What cannot be changed: Obligation to check line existence before jumping (Undefined line number).
 * - What to expect: Execution will branch immediately after the handler returns, as the VM loop
 *   respects next_line overrides.
 * - What to do if something breaks: If subroutines return to the wrong place, trace the GOSUB stack
 *   pushes and verify next_pos offsets.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: Target line numbers exist in the program store. Stacks do not overflow.
 * - Portability concerns: None. C17 compliant.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - How future expansion can occur safely: Add multi-line IF...ELSEIF...END IF or SELECT CASE blocks.
 * - How to write external extensions: Plugins do not manipulate control flow directly; they yield control to the VM.
 */

#include "bpp_stmt.h"
#include "bpp_file.h"
#include "bpp_eval.h"
#include "bpp_vdev.h"
#include "bpp_metadata.h"
#include "bpp_variables.h"
#include "bpp_strings.h"
#include "bpp_security.h"
#include "bpp_module.h"
#include "bpp_platform.h"
#include "bpp_memory.h"
#include <string.h>

BppError stmt_end_sub_handler(VMContext *vm, LexerContext *lex);
BppError stmt_end_function_handler(VMContext *vm, LexerContext *lex);

/* GOTO handler */
BppError stmt_goto_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_next(lex);
    if (tok.type == TOK_GLOBAL_LABEL) {
        char label_name[64];
        int len = (int)(tok.length < sizeof(label_name) - 1 ? tok.length : sizeof(label_name) - 1);
        memcpy(label_name, tok.as.string, len);
        label_name[len] = '\0';

        char filename[256];
        BppLineNumber target_line = 0.0;
        if (!metadata_resolve_label(vm_get_metadata(vm), label_name, filename, sizeof(filename), &target_line)) {
            err.code = 8; err.message = "Undefined global label";
            return err;
        }

        const char *cur_file = vm_get_current_filename(vm);
        if (filename[0] != '\0' && cur_file[0] != '\0' && strcasecmp(filename, cur_file) != 0) {
            /* Load and switch program file */
            BppError load_err = vm_load_program_file(vm, filename);
            if (load_err.code != 0) return load_err;
        }

        vm_jump(vm, target_line, NULL);
        return err;
    }

    if (tok.type != TOK_NUMBER) {
        err.code = 2; /* Syntax error */
        err.message = "Expected line number after GOTO";
        return err;
    }

    BppLineNumber target = tok.as.number;
    if (!mem_program_get(vm_get_mem(vm), target)) {
        err.code = 8; /* Undefined line number */
        err.message = "Undefined line number in GOTO";
        return err;
    }

    vm_jump(vm, target, NULL);
    return err;
}

/* GOSUB handler */
BppError stmt_gosub_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_next(lex);
    if (tok.type == TOK_GLOBAL_LABEL) {
        char label_name[64];
        int len = (int)(tok.length < sizeof(label_name) - 1 ? tok.length : sizeof(label_name) - 1);
        memcpy(label_name, tok.as.string, len);
        label_name[len] = '\0';

        char filename[256];
        BppLineNumber target_line = 0.0;
        if (!metadata_resolve_label(vm_get_metadata(vm), label_name, filename, sizeof(filename), &target_line)) {
            err.code = 8; err.message = "Undefined global label";
            return err;
        }

        const char *cur_file = vm_get_current_filename(vm);
        if (filename[0] != '\0' && cur_file[0] != '\0' && strcasecmp(filename, cur_file) != 0) {
            err.code = 38; err.message = "Cross-file GOSUB not supported";
            return err;
        }

        /* Push current return address */
        if (!vm_gosub_push(vm, vm_get_current_line(vm), lex_get_pos(lex))) {
            err.code = 12; err.message = "Subroutine nesting limit exceeded";
            return err;
        }

        vm_jump(vm, target_line, NULL);
        return err;
    }

    if (tok.type != TOK_NUMBER) {
        err.code = 2; /* Syntax error */
        err.message = "Expected line number after GOSUB";
        return err;
    }

    BppLineNumber target = tok.as.number;
    if (!mem_program_get(vm_get_mem(vm), target)) {
        err.code = 8; /* Undefined line number */
        err.message = "Undefined line number in GOSUB";
        return err;
    }

    /* Push current program line and the position *after* the target line number onto GOSUB stack. */
    BppLineNumber current = vm_get_current_line(vm);
    const char *resume_pos = lex_get_pos(lex);

    if (!vm_gosub_push(vm, current, resume_pos)) {
        err.code = 14; /* Out of memory / Stack overflow */
        err.message = "GOSUB stack overflow";
        return err;
    }

    vm_jump(vm, target, NULL);
    return err;
}

/* RETURN handler */
BppError stmt_return_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    (void)lex;

    BppLineNumber resume_line = 0.0;
    const char *resume_pos = NULL;

    if (!vm_gosub_pop(vm, &resume_line, &resume_pos)) {
        err.code = 3; /* RETURN without GOSUB */
        err.message = "RETURN without GOSUB";
        return err;
    }

    vm_jump(vm, resume_line, resume_pos);
    vm_clear_event_handlers(vm);
    return err;
}

/* END handler */
BppError stmt_end_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_KEYWORD) {
        if (tok.as.keyword == KW_SELECT) {
            lex_next(lex); /* Consume SELECT */
            BppSelectFrame frame;
            if (!vm_select_pop(vm, &frame)) {
                err.code = 2; err.message = "END SELECT without SELECT CASE";
            }
            return err;
        }
        if (tok.as.keyword == KW_IF) {
            lex_next(lex); /* Consume IF */
            /* Block IF end, nothing to pop at runtime */
            return err;
        }
        if (tok.as.keyword == KW_TRY) {
            lex_next(lex); /* Consume TRY */
            BppTryFrame frame;
            if (try_stack_pop(vm_get_try_stack(vm), &frame)) {
                /* Popped TRY frame */
            }
            return err;
        }
        if (tok.as.keyword == KW_ATOMIC) {
            lex_next(lex); /* Consume ATOMIC */
            file_txn_commit(vm_get_file(vm));
            return err;
        }
        if (tok.as.keyword == KW_SUB || tok.as.keyword == KW_PROCEDURE) {
            lex_next(lex); /* Consume SUB/PROCEDURE */
            return stmt_end_sub_handler(vm, lex);
        }
        if (tok.as.keyword == KW_FUNCTION) {
            lex_next(lex); /* Consume FUNCTION */
            return stmt_end_function_handler(vm, lex);
        }
        if (tok.as.keyword == KW_TYPE) {
            lex_next(lex); /* Consume TYPE */
            return err;
        }
        if (tok.as.keyword == KW_ENUM) {
            lex_next(lex); /* Consume ENUM */
            return err;
        }
        if (tok.as.keyword == KW_WITH) {
            lex_next(lex); /* Consume WITH */
            vm_with_stack_pop(vm);
            return err;
        }
        if (tok.as.keyword == KW_DIALECT) {
            lex_next(lex); /* Consume DIALECT */
            BppDialect *d = vm_get_defining_dialect(vm);
            if (d) {
                vm_set_active_dialect(vm, d);
                vm_set_defining_dialect(vm, NULL);
            } else {
                err.code = 2; err.message = "END DIALECT without DIALECT DEFINE";
            }
            return err;
        }
        if (tok.as.keyword == KW_CLASS) {
            lex_next(lex); /* Consume CLASS */
            return err;
        }
    }

    vm_halt(vm);
    return err;
}

/**
 * BYE handler - Halt program execution and signal interpreter exit.
 */
BppError stmt_bye_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    (void)lex;
    vm_halt(vm);
    vm_request_exit(vm);
    return err;
}

extern void platform_execute_shell(void);
extern void platform_execute_command(const char *cmd);

/**
 * SYSTEM handler - Drop into a temporary OS shell.
 */
BppError stmt_system_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    VDevContext *vdev = vm_get_vdev(vm);
    if (!vdev) return err;

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_STRING || tok.type == TOK_IDENT || tok.type == TOK_KEYWORD) {
        lex_next(lex);
        char query[256];
        const char *src = (tok.type == TOK_STRING) ? tok.as.string : tok.start;
        int len = (int)(tok.length < sizeof(query) - 1 ? tok.length : sizeof(query) - 1);
        memcpy(query, src, len);
        query[len] = '\0';

        /* Case-insensitive compare */
        for (int i = 0; query[i]; i++) {
            if (query[i] >= 'a' && query[i] <= 'z') query[i] -= 32;
        }

        if (strcmp(query, "PLATFORM") == 0) {
            vdev_printf(vdev, "%s (%s)\n", platform_name(), BPP_PROFILE_NAME);
        } else if (strcmp(query, "VERSION") == 0) {
            vdev_printf(vdev, "%s v%s \"%s\"\n", BPP_NAME, BPP_VERSION_STRING, BPP_VERSION_CODENAME);
        } else if (strcmp(query, "MEMORY") == 0) {
            size_t free_ram = mem_get_free_ram(vm_get_mem(vm));
            size_t total_ram = free_ram + mem_get_used_ram(vm_get_mem(vm));
            char free_buf[64], total_buf[64];
            mem_format_size(free_ram, free_buf, sizeof(free_buf));
            mem_format_size(total_ram, total_buf, sizeof(total_buf));
            vdev_printf(vdev, "Free RAM: %s / %s total\n", free_buf, total_buf);
        } else if (strcmp(query, "COMPILER") == 0) {
#if defined(_MSC_VER)
            vdev_printf(vdev, "MSVC %d\n", _MSC_VER);
#elif defined(__GNUC__) && !defined(__clang__)
            vdev_printf(vdev, "GCC %d.%d.%d\n", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
#elif defined(__clang__)
            vdev_printf(vdev, "Clang %d.%d.%d\n", __clang_major__, __clang_minor__, __clang_patchlevel__);
#else
            vdev_puts(vdev, "Unknown C17 Compiler\n");
#endif
        } else if (strcmp(query, "WORDSIZE") == 0) {
            vdev_printf(vdev, "%d-bit\n", (int)(sizeof(void*) * 8));
        } else {
            vdev_printf(vdev, "Unknown query '%s'. Use PLATFORM, VERSION, MEMORY, COMPILER, or WORDSIZE.\n", query);
        }
        return err;
    }

    /* Print full summary */
    vdev_printf(vdev, "Platform: %s (%s)\n", platform_name(), BPP_PROFILE_NAME);
#if defined(_MSC_VER)
    vdev_printf(vdev, "Compiler: MSVC %d\n", _MSC_VER);
#elif defined(__GNUC__) && !defined(__clang__)
    vdev_printf(vdev, "Compiler: GCC %d.%d.%d\n", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
#elif defined(__clang__)
    vdev_printf(vdev, "Compiler: Clang %d.%d.%d\n", __clang_major__, __clang_minor__, __clang_patchlevel__);
#else
    vdev_puts(vdev, "Compiler: Unknown C17 Compiler\n");
#endif
    vdev_printf(vdev, "Word size: %d-bit (ptr=%d int=%d long=%d)\n",
                (int)(sizeof(void*) * 8),
                (int)sizeof(void*),
                (int)sizeof(int),
                (int)sizeof(long));
    vdev_printf(vdev, "%s v%s \"%s\"\n", BPP_NAME, BPP_VERSION_STRING, BPP_VERSION_CODENAME);
    vdev_printf(vdev, "Security: %s\n", security_level_name(security_get_level()));
    vdev_printf(vdev, "Modules: %d registered\n", module_count());

    return err;
}

/**
 * SHELL handler - Execute a targeted string command natively.
 */
BppError stmt_shell_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    BValue cmd_val;
    memset(&cmd_val, 0, sizeof(cmd_val));

    /* Check if there's a string argument */
    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_EOF || tok.type == TOK_EOL) {
        platform_execute_shell();
        return err;
    }

    cmd_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    if (cmd_val.type != VAL_STRING) {
        err.code = 13; err.message = "Type mismatch (expected string for SHELL)";
        return err;
    }

    platform_execute_command(str_data(cmd_val.as.string));
    return err;
}

/**
 * STOP handler - Halt program execution with a 'Break' message.
 *
 * What it does: Halts the running program like END, but reports
 *   'Break in line N' so the user knows where execution paused.
 * Why it exists: GW-BASIC, QBASIC, and most classic BASIC dialects
 *   support STOP as a debugging breakpoint.
 * Why it works this way: Sets vm->running = false via vm_halt()
 *   and prints a break message with the current line number.
 * Assumptions: vdev CON: is initialized.
 * Portability concerns: None. C17 compliant.
 * Future expansions: Could integrate with a CONT (continue) command.
 */
BppError stmt_stop_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    (void)lex;

    VDevContext *vdev = vm_get_vdev(vm);
    BppLineNumber ln = vm_get_current_line(vm);
    if (ln > 0.0) {
        vdev_printf(vdev, "Break in %g\n", ln);
    } else {
        vdev_puts(vdev, "Break\n");
    }
    vm_halt(vm);
    return err;
}

/* REM handler (NOP) */
BppError stmt_rem_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    (void)vm;
    (void)lex;
    /* REM comments skip the entire rest of the line, which the lexer already did */
    return err;
}

/* Helper: Dispatch statement inside IF THEN/ELSE branch */
static BppError dispatch_branch(VMContext *vm, LexerContext *lex) {
    extern BppError execute_single_statement(VMContext *vm, LexerContext *lex);
    return execute_single_statement(vm, lex);
}

/* IF/THEN/ELSE handler */
BppError stmt_if_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    /* Evaluate condition */
    BValue cond = eval_expression(vm, lex, &err);
    if (err.code != 0) {
        return err;
    }

    if (cond.type == VAL_STRING) {
        err.code = 13; /* Type mismatch */
        err.message = "IF condition cannot be a string";
        return err;
    }

    /* Expect THEN */
    BppToken tok = lex_next(lex);
    if (tok.type != TOK_KEYWORD || tok.as.keyword != KW_THEN) {
        err.code = 2; /* Syntax error */
        err.message = "Expected THEN after IF condition";
        return err;
    }

    bool is_true = (cond.as.number != 0.0);

    if (is_true) {
        /* Parse and execute THEN branch statement */
        BppToken next = lex_peek(lex);
        if (next.type == TOK_NUMBER) {
            /* Implicit GOTO */
            lex_next(lex);
            BppLineNumber target = next.as.number;
            if (!mem_program_get(vm_get_mem(vm), target)) {
                err.code = 8;
                err.message = "Undefined line number in IF branch";
                return err;
            }
            vm_jump(vm, target, NULL);
            return err;
        }

        /* Execute all statements in the THEN branch (separated by colons) until ELSE or EOL */
        while (true) {
            err = dispatch_branch(vm, lex);
            if (err.code != 0 || vm_is_jump_active(vm)) {
                return err;
            }
            BppToken next_tok = lex_peek(lex);
            if (next_tok.type == TOK_EOL && *next_tok.start == ':') {
                lex_next(lex); /* Consume colon */
                BppToken after_colon = lex_peek(lex);
                if (after_colon.type == TOK_KEYWORD && after_colon.as.keyword == KW_ELSE) {
                    break;
                }
            } else {
                break;
            }
        }

        /* Skip trailing ELSE branch if it exists */
        BppToken skip = lex_peek(lex);
        if (skip.type == TOK_KEYWORD && skip.as.keyword == KW_ELSE) {
            while (skip.type != TOK_EOF && skip.type != TOK_EOL) {
                lex_next(lex);
                skip = lex_peek(lex);
            }
        }
    } else {
        /* Condition is false: skip to ELSE or EOL */
        BppToken skip = lex_peek(lex);
        while (skip.type != TOK_EOF && (skip.type != TOK_EOL || *skip.start == ':')) {
            if (skip.type == TOK_KEYWORD && skip.as.keyword == KW_ELSE) {
                break;
            }
            lex_next(lex);
            skip = lex_peek(lex);
        }

        if (skip.type == TOK_KEYWORD && skip.as.keyword == KW_ELSE) {
            lex_next(lex); /* Consume ELSE */

            /* Check if statement following ELSE is a line number (implicit GOTO) */
            BppToken next = lex_peek(lex);
            if (next.type == TOK_NUMBER) {
                lex_next(lex);
                BppLineNumber target = next.as.number;
                if (!mem_program_get(vm_get_mem(vm), target)) {
                    err.code = 8;
                    err.message = "Undefined line number in ELSE branch";
                    return err;
                }
                vm_jump(vm, target, NULL);
                return err;
            }

            /* Execute all statements in the ELSE branch (separated by colons) */
            while (true) {
                err = dispatch_branch(vm, lex);
                if (err.code != 0 || vm_is_jump_active(vm)) {
                    return err;
                }
                BppToken next_tok = lex_peek(lex);
                if (next_tok.type == TOK_EOL && *next_tok.start == ':') {
                    lex_next(lex); /* Consume colon */
                } else {
                    break;
                }
            }
        }
    }

    return err;
}

BppError stmt_with_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    
    char path[256] = "";
    size_t path_len = 0;
    
    while (true) {
        BppToken tok = lex_peek(lex);
        if (tok.type == TOK_IDENT || tok.type == TOK_KEYWORD) {
            lex_next(lex);
            size_t len = tok.length;
            if (path_len + len + 1 < sizeof(path)) {
                memcpy(path + path_len, tok.start, len);
                path_len += len;
                path[path_len] = '\0';
            }
        } else if (tok.type == TOK_PERIOD) {
            lex_next(lex);
            if (path_len + 1 < sizeof(path)) {
                path[path_len++] = '.';
                path[path_len] = '\0';
            }
        } else {
            break;
        }
    }
    
    if (path_len == 0) {
        err.code = 2; err.message = "Expected variable or object path in WITH statement";
        return err;
    }
    
    char fq_path[512];
    if (path[0] == '.') {
        const char *with_prefix = vm_with_stack_peek(vm);
        if (!with_prefix) {
            err.code = 2; err.message = "Leading '.' in WITH path outside of active WITH block";
            return err;
        }
        snprintf(fq_path, sizeof(fq_path), "%s%s", with_prefix, path);
    } else {
        snprintf(fq_path, sizeof(fq_path), "%s", path);
    }
    
    vm_with_stack_push(vm, fq_path);
    return err;
}
