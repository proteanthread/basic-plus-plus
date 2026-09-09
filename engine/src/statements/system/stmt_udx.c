// FILENAME: stmt_udx.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, exec_dispatch.c, common_reg_stmts.c
// NEEDS: libreg (reg_buffer.h, reg_udx.h, reg_hw.h), eval/eval.h, vm/vm.h
// Implements runtime statements for Universal Data Exchange (UDX, XCHG, PUSH, POP).

#include "statements/system/stmt_udx.h"
#include "reg/reg_buffer.h"
#include "reg/reg_udx.h"
#include "reg/reg_hw.h"
#include "eval/eval.h"
#include "runtime/variables.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"
#include "runtime/language_descriptor.h"

static const LangDesc g_udx_desc = {
    .name = "UDX",
    .category = "System & Hardware",
    .syntax = "UDX TRANSFER src, dst | UDX CHANNEL name$, discipline$",
    .description = "Manages inter-process and inter-function data exchange routing and channels across the UDX bus.",
    .error_summary = "Error 2: Syntax Error, Error 5: Illegal Function Call",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_STATEMENT
};

static const LangDesc g_push_desc = {
    .name = "PUSH",
    .category = "System & Hardware",
    .syntax = "PUSH [FIFO|LIFO|LILO|FILO] [, channel$] expr",
    .description = "Pushes an expression value into a designated UDX buffer or channel.",
    .error_summary = "Error 2: Syntax Error, Error 5: Illegal Function Call",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_STATEMENT
};

static const LangDesc g_pop_desc = {
    .name = "POP",
    .category = "System & Hardware",
    .syntax = "POP [FIFO|LIFO|LILO|FILO] [, channel$] var",
    .description = "Pops a value from a designated UDX buffer or channel into a target variable.",
    .error_summary = "Error 2: Syntax Error, Error 5: Illegal Function Call",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_STATEMENT
};

BppError stmt_push_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    char target[64] = "FIFO";

    // Optional discipline or channel name: PUSH [FIFO|LIFO|LILO|FILO]
    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_IDENT || tok.type == TOK_KEYWORD) {
        if (runtime_strncasecmp(tok.start, "FIFO", tok.length) == 0 ||
            runtime_strncasecmp(tok.start, "LIFO", tok.length) == 0 ||
            runtime_strncasecmp(tok.start, "LILO", tok.length) == 0 ||
            runtime_strncasecmp(tok.start, "FILO", tok.length) == 0) {
            size_t len = tok.length < sizeof(target) - 1 ? tok.length : sizeof(target) - 1;
            runtime_memcpy(target, tok.start, len);
            target[len] = '\0';
            lex_next(lex);

            // Optional comma before expression: PUSH FIFO, expr
            if (lex_peek(lex).type == TOK_COMMA) {
                lex_next(lex);
            }
        }
    }

    BValue val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    if (!reg_buffer_push_by_name(target, val)) {
        err.code = 5;
        err.message = "UDX buffer overflow: channel capacity reached";
        return err;
    }

    return err;
}

BppError stmt_pop_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    char target[64] = "FIFO";

    // Optional discipline: POP [FIFO|LIFO|LILO|FILO]
    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_IDENT || tok.type == TOK_KEYWORD) {
        if (runtime_strncasecmp(tok.start, "FIFO", tok.length) == 0 ||
            runtime_strncasecmp(tok.start, "LIFO", tok.length) == 0 ||
            runtime_strncasecmp(tok.start, "LILO", tok.length) == 0 ||
            runtime_strncasecmp(tok.start, "FILO", tok.length) == 0) {
            size_t len = tok.length < sizeof(target) - 1 ? tok.length : sizeof(target) - 1;
            runtime_memcpy(target, tok.start, len);
            target[len] = '\0';
            lex_next(lex);

            if (lex_peek(lex).type == TOK_COMMA) {
                lex_next(lex);
            }
        }
    }

    tok = lex_next(lex);
    if (tok.type != TOK_IDENT && tok.type != TOK_KEYWORD) {
        err.code = 2;
        err.message = "Syntax error: Expected target variable for POP";
        return err;
    }

    char var_name[64];
    size_t vlen = tok.length < sizeof(var_name) - 1 ? tok.length : sizeof(var_name) - 1;
    runtime_memcpy(var_name, tok.start, vlen);
    var_name[vlen] = '\0';

    BValue val;
    runtime_memset(&val, 0, sizeof(val));
    if (!reg_buffer_pop_by_name(target, &val)) {
        err.code = 5;
        err.message = "UDX buffer underflow: channel is empty";
        return err;
    }

    if (!var_assign(vm_get_var(vm), var_name, val)) {
        err.code = 13;
        err.message = "Type mismatch in POP variable assignment";
        return err;
    }

    return err;
}

BppError stmt_udx_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    BppToken subcmd = lex_next(lex);
    if (subcmd.type != TOK_IDENT && subcmd.type != TOK_KEYWORD) {
        err.code = 2;
        err.message = "Syntax error: Expected subcommand for UDX/XCHG (TRANSFER, CHANNEL, PUSH, POP)";
        return err;
    }

    if (runtime_strncasecmp(subcmd.start, "TRANSFER", subcmd.length) == 0) {
        // Syntax: UDX TRANSFER src, dst
        BppToken tok_src = lex_next(lex);
        char src_buf[64];
        size_t slen = tok_src.length < sizeof(src_buf) - 1 ? tok_src.length : sizeof(src_buf) - 1;
        runtime_memcpy(src_buf, tok_src.start, slen);
        src_buf[slen] = '\0';

        if (lex_peek(lex).type != TOK_COMMA) {
            err.code = 2;
            err.message = "Syntax error: Expected comma between source and destination";
            return err;
        }
        lex_next(lex); // consume comma

        BppToken tok_dst = lex_next(lex);
        char dst_buf[64];
        size_t dlen = tok_dst.length < sizeof(dst_buf) - 1 ? tok_dst.length : sizeof(dst_buf) - 1;
        runtime_memcpy(dst_buf, tok_dst.start, dlen);
        dst_buf[dlen] = '\0';

        if (!reg_buffer_transfer(src_buf, dst_buf)) {
            err.code = 5;
            err.message = "UDX transfer failed: invalid endpoints or underflow";
            return err;
        }
        return err;
    }

    if (runtime_strncasecmp(subcmd.start, "PUSH", subcmd.length) == 0) {
        return stmt_push_handler(vm, lex);
    }

    if (runtime_strncasecmp(subcmd.start, "POP", subcmd.length) == 0) {
        return stmt_pop_handler(vm, lex);
    }

    err.code = 2;
    err.message = "Syntax error: Unknown UDX/XCHG subcommand";
    return err;
}

void stmt_udx_register(void) {
    lang_desc_register(&g_udx_desc);
    lang_desc_register(&g_push_desc);
    lang_desc_register(&g_pop_desc);
}
