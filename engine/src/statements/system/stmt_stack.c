// FILENAME: stmt_stack.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, exec_dispatch.c, common_reg_stmts.c
// NEEDS: eval/rpn.h, eval/pn.h, runtime/variables.h, vm/vm.h
// Implements runtime statements for HP-based STACK operations (STACK POP, STACK EXPORT, STACK DUMP, STACK CLEAR).

#include "statements/system/stmt_stack.h"
#include "eval/rpn.h"
#include "eval/pn.h"
#include "runtime/variables.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"
#include "runtime/language_descriptor.h"
#include "device/vdev.h"
#include "vm/vm.h"

static const LangDesc g_stmt_stack_desc = {
    .name = "STACK",
    .category = "Math & Calculations",
    .syntax = "STACK POP var1 [, var2, ...] | STACK EXPORT var1 [, var2] | STACK DUMP | STACK CLEAR | STACK SWAP",
    .description = "Controls and exports HP operational stack levels and storage registers to BASIC++ program variables.",
    .error_summary = "Error 2: Syntax Error, Error 5: Illegal Function Call",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_STATEMENT
};

BppError stmt_stack_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    BppToken subcmd = lex_next(lex);
    if (subcmd.type != TOK_IDENT && subcmd.type != TOK_KEYWORD) {
        err.code = 2;
        err.message = "Syntax error: Expected STACK subcommand (POP, EXPORT, DUMP, CLEAR, SWAP, ROLLUP, ROLLDN)";
        return err;
    }

    if (runtime_strncasecmp(subcmd.start, "POP", subcmd.length) == 0) {
        // Syntax: STACK POP var1 [, var2, ...]
        while (true) {
            BppToken var_tok = lex_next(lex);
            if (var_tok.type != TOK_IDENT && var_tok.type != TOK_KEYWORD) {
                err.code = 2;
                err.message = "Syntax error: Expected target variable for STACK POP";
                return err;
            }

            char var_name[64];
            size_t vlen = var_tok.length < sizeof(var_name) - 1 ? var_tok.length : sizeof(var_name) - 1;
            runtime_memcpy(var_name, var_tok.start, vlen);
            var_name[vlen] = '\0';

            BValue val;
            runtime_memset(&val, 0, sizeof(val));
            if (!rpn_stack_pop(&val)) {
                err.code = 5;
                err.message = "Stack underflow in STACK POP";
                return err;
            }

            if (!var_assign(vm_get_var(vm), var_name, val)) {
                err.code = 13;
                err.message = "Type mismatch in STACK POP assignment";
                return err;
            }

            if (lex_peek(lex).type == TOK_COMMA) {
                lex_next(lex); // consume comma and pop next
            } else {
                break;
            }
        }
        return err;
    }

    if (runtime_strncasecmp(subcmd.start, "EXPORT", subcmd.length) == 0) {
        // Syntax: STACK EXPORT var1 [, var2, ...] (non-destructive export: var1=X, var2=Y, etc.)
        RpnContext *ctx = rpn_get_context();
        BValue stack_levels[4] = { ctx->x, ctx->y, ctx->z, ctx->t };
        int level_idx = 0;

        while (level_idx < 4) {
            BppToken var_tok = lex_next(lex);
            if (var_tok.type != TOK_IDENT && var_tok.type != TOK_KEYWORD) {
                err.code = 2;
                err.message = "Syntax error: Expected target variable for STACK EXPORT";
                return err;
            }

            char var_name[64];
            size_t vlen = var_tok.length < sizeof(var_name) - 1 ? var_tok.length : sizeof(var_name) - 1;
            runtime_memcpy(var_name, var_tok.start, vlen);
            var_name[vlen] = '\0';

            if (!var_assign(vm_get_var(vm), var_name, stack_levels[level_idx++])) {
                err.code = 13;
                err.message = "Type mismatch in STACK EXPORT assignment";
                return err;
            }

            if (lex_peek(lex).type == TOK_COMMA) {
                lex_next(lex);
            } else {
                break;
            }
        }
        return err;
    }

    if (runtime_strncasecmp(subcmd.start, "CLEAR", subcmd.length) == 0) {
        rpn_stack_clear();
        return err;
    }

    if (runtime_strncasecmp(subcmd.start, "SWAP", subcmd.length) == 0) {
        rpn_stack_swap();
        return err;
    }

    if (runtime_strncasecmp(subcmd.start, "ROLLUP", subcmd.length) == 0) {
        rpn_stack_rollup();
        return err;
    }

    if (runtime_strncasecmp(subcmd.start, "ROLLDN", subcmd.length) == 0) {
        rpn_stack_rolldn();
        return err;
    }

    if (runtime_strncasecmp(subcmd.start, "DUMP", subcmd.length) == 0) {
        RpnContext *ctx = rpn_get_context();
        VDevContext *vdev = vm_get_vdev(vm);
        vdev_printf(vdev, "=== HP RPN OPERATIONAL STACK ===\n");
        vdev_printf(vdev, "  T:     %f\n", ctx->t.as.number);
        vdev_printf(vdev, "  Z:     %f\n", ctx->z.as.number);
        vdev_printf(vdev, "  Y:     %f\n", ctx->y.as.number);
        vdev_printf(vdev, "  X:     %f\n", ctx->x.as.number);
        vdev_printf(vdev, "  LASTX: %f\n", ctx->last_x.as.number);
        vdev_printf(vdev, "  WSIZE: %d, CARRY: %d, OVERFLOW: %d\n", ctx->wsize, ctx->carry ? 1 : 0, ctx->overflow ? 1 : 0);
        return err;
    }

    err.code = 2;
    err.message = "Syntax error: Unknown STACK subcommand";
    return err;
}

void stmt_stack_register(void) {
    lang_desc_register(&g_stmt_stack_desc);
}
