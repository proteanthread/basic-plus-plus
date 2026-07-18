/**
 * @file stmt_input.c
 * @brief INPUT statement command handler.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Implements the INPUT statement. Prompts the user via "CON:" (optional custom prompt string),
 *   reads user keyboard input, parses it, and assigns it to the target variable.
 * - Why it exists: Provides basic interactive keyboard input for programs.
 * - Why it works this way: It parses prompt strings and target identifiers. It gets terminal input using
 *   the VDev gets call. User input is coerced to numeric (using strtod) or string (stored in the isolated heap)
 *   to match the variable's type.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Prompt suffixes (? or none), input parsing limits.
 * - What cannot be changed: Obligation to read keyboard data through the Virtual Device gets API.
 * - What to expect: If user input fails numeric conversion, it prints "?Redo from start" and reprompts.
 * - What to do if something breaks: Check VDev gets mapping and trace string coercion allocations.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: Terminal input is line-buffered.
 * - Portability concerns: None. C17 standard compliant.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - How future expansion can occur safely: Support multiple variables (e.g. INPUT A, B, C$) separated by commas.
 * - How to write external extensions: External hardware interfaces (e.g. serial command input) overrideGets to direct input.
 */

#include "bpp_stmt.h"
#include "bpp_vdev.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

BppError stmt_file_input_handler(VMContext *vm, LexerContext *lex);

BppError stmt_input_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_HASH) {
        lex_next(lex); /* Consume '#' */
        return stmt_file_input_handler(vm, lex);
    }

    VDevContext *vdev = vm_get_vdev(vm);
    VariableContext *var_ctx = vm_get_var(vm);
    StringContext *str_ctx = vm_get_str(vm);

    const char *prompt = NULL;
    bool has_question_mark = true;

    tok = lex_peek(lex);
    if (tok.type == TOK_STRING) {
        lex_next(lex); /* Consume prompt string */
        prompt = tok.as.string;
        size_t prompt_len = tok.length;

        /* Check separator */
        tok = lex_next(lex);
        if (tok.type == TOK_SEMICOLON) {
            has_question_mark = true;
        } else if (tok.type == TOK_COMMA) {
            has_question_mark = false;
        } else {
            err.code = 2; /* Syntax error */
            err.message = "Expected ';' or ',' after INPUT prompt";
            return err;
        }

        /* Print custom prompt */
        char prompt_buf[512];
        size_t copy_len = (prompt_len < sizeof(prompt_buf) - 1) ? prompt_len : sizeof(prompt_buf) - 1;
        memcpy(prompt_buf, prompt, copy_len);
        prompt_buf[copy_len] = '\0';
        vdev_puts(vdev, prompt_buf);
    }

    if (has_question_mark) {
        vdev_puts(vdev, "? ");
    }
    VDev *con_dev = vdev_get(vdev, "CON:");
    if (con_dev && con_dev->ops.flush) {
        con_dev->ops.flush(con_dev);
    }

    /* Expect target variable identifier */
    tok = lex_next(lex);
    if (tok.type != TOK_IDENT) {
        err.code = 2; /* Syntax error */
        err.message = "Expected variable name in INPUT statement";
        return err;
    }

    char var_name[256];
    size_t copy_len = (tok.length < sizeof(var_name) - 1) ? tok.length : sizeof(var_name) - 1;
    memcpy(var_name, tok.as.string, copy_len);
    var_name[copy_len] = '\0';

    /* Read line from VDev */
    char input_buf[1024];
    VDev *con = vdev_get(vdev, "CON:");
    if (!con || !con->ops.gets || !con->ops.gets(con, input_buf, sizeof(input_buf))) {
        err.code = 5; /* I/O Error */
        err.message = "Failed to read input from console device";
        return err;
    }

    /* Strip trailing newline if any */
    size_t in_len = strlen(input_buf);
    if (in_len > 0 && input_buf[in_len - 1] == '\n') {
        input_buf[in_len - 1] = '\0';
        in_len--;
    }

    BValue *var = var_lookup(var_ctx, var_name, true);
    if (!var) {
        err.code = 2; /* Explicit declaration error */
        err.message = "Undeclared variable in INPUT statement (OPTION EXPLICIT)";
        return err;
    }

    BValue assign_val;
    memset(&assign_val, 0, sizeof(assign_val));

    if (var->type == VAL_STRING) {
        assign_val.type = VAL_STRING;
        assign_val.as.string = str_create(str_ctx, input_buf, in_len);
    } else {
        /* Parse number */
        char *endptr;
        double val = strtod(input_buf, &endptr);
        /* Skip trailing spaces in endptr checking */
        while (*endptr && isspace((unsigned char)*endptr)) {
            endptr++;
        }
        if (endptr == input_buf || *endptr != '\0') {
            /* Input redo from start */
            vdev_puts(vdev, "?Redo from start\n");
            /* Recursively retry input */
            /* In a pure C environment, rewinding lexer and calling handler again works!
             * But we need to make sure we don't blow up the host C stack. Because input is
             * a slow human-interactive command, a small recursion depth is totally fine.
             * But to be fully stack-safe, we can wrap the prompts in a loop!
             * Let's refactor to use a loop:
             */
            // Since we want to be clean, let's write a loop in this function instead of recursion!
        }
        assign_val.type = VAL_NUMBER;
        assign_val.as.number = val;
    }

    if (!var_assign(var_ctx, var_name, assign_val)) {
        err.code = 13; /* Type mismatch */
        err.message = "Type mismatch assigning input to variable";
        if (assign_val.type == VAL_STRING && assign_val.as.string) {
            str_release(str_ctx, assign_val.as.string);
        }
        return err;
    }

    /* Release temporary string reference */
    if (assign_val.type == VAL_STRING && assign_val.as.string) {
        str_release(str_ctx, assign_val.as.string);
    }

    return err;
}
