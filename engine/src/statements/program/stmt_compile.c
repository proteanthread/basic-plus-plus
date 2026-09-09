// FILENAME: stmt_compile.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_stmts.c)
// NEEDS: libcore, libengine, libkernel
// Implements the COMPILE statement for Ahead-Of-Time in-memory compilation.
//
// ---- Includes ----

#include "statements/program/stmt_compile.h"
#include "vm/jit.h"
#include "eval/ast.h"
#include "eval/ast_internal.h"
#include "memory/memory.h"
#include "security/security.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

static const LangDesc g_compile_desc = {
    .name = "COMPILE",
    .category = "Program Mgmt & Editing",
    .syntax = "COMPILE [ALL | SUB | JIT]",
    .description = "Pre-compiles the current program in memory Ahead-Of-Time (AOT) into JIT bytecode blocks.",
    .error_summary = "Error 2: Syntax Error",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_STATEMENT
};

void stmt_compile_register(void) {
    lang_desc_register(&g_compile_desc);
}

BppError stmt_compile_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    if (!vm || !lex) return err;

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_STRING) {
        lex_next(lex);
    } else if (tok.type == TOK_IDENT || tok.type == TOK_KEYWORD) {
        lex_next(lex);
    }

    MemoryContext *mem = vm_get_mem(vm);
    if (!mem) return err;

    size_t count = 0;
    BppProgramLine *lines = mem_program_get_all(mem, &count);
    if (!lines || count == 0) return err;

    // Pre-compile all program lines into AST and JIT blocks
    for (size_t i = 0; i < count; i++) {
        if (!lines[i].ast_valid || lines[i].ast_cache == NULL) {
            size_t skipped = 0;
            EvalAstNode *parsed = eval_ast_try_compile_multiline_block(vm, lines, i, count, &skipped);
            if (!parsed && lines[i].text) {
                parsed = eval_ast_try_parse_line(vm, lines[i].text);
            }
            if (parsed) {
                lines[i].ast_cache = parsed;
                lines[i].ast_valid = true;
                lines[i].ast_skip_lines = skipped;

                if (parsed->type == AST_NODE_FOR_LOOP && !parsed->target_ast) {
                    JitBytecodeProgram *prog = jit_compile_ast(vm, parsed);
                    if (prog) {
                        parsed->target_ast = (EvalAstNode *)prog;
                    }
                }
            }
        }
    }

    return err;
}
