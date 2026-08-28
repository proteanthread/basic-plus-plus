// FILENAME: function.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (eval.h, eval.c, function.h, lexer.h, lexer.c, string.c)
// NEEDS: libengine (vm.h)
// NEEDS: libkernel (security.h, security.c, vdev.h, vdev.c)
// Provides runtime implementation for the FUNCTION statement in BASIC++.
//
// ---- Includes ----

#include "statements/oop/structure/function.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "device/vdev.h"
#include "security/security.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>

#include "memory/memory.h"

BppError stmt_function_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    if (!vm || !lex) {
        err.code = 5; err.message = "Null VM or lexer context";
        return err;
    }

    BppSubFrame frame;
    if (vm_sub_peek(vm, &frame) && strcmp(vm_get_active_proc(vm), "") != 0) {
        // Currently inside call frame, execute FUNCTION body normally
        return err;
    }

    MemoryContext *mem = vm_get_mem(vm);
    size_t count = 0;
    BppProgramLine *lines = mem_program_get_all(mem, &count);
    BppLineNumber cur_ln = vm_get_current_line(vm);

    size_t start_idx = 0;
    for (size_t i = 0; i < count; ++i) {
        if (lines[i].line_number == cur_ln) { start_idx = i; break; }
    }

    int nest = 0;
    for (size_t i = start_idx + 1; i < count; ++i) {
        LexerContext *scan = lex_init(mem, lines[i].text);
        if (!scan) continue;
        BppToken tok = lex_next(scan);
        if (tok.type == TOK_NUMBER) {
            tok = lex_next(scan);
        }
        if (tok.type == TOK_KEYWORD) {
            if (tok.as.keyword == KW_SUB || tok.as.keyword == KW_FUNCTION) nest++;
            else if (tok.as.keyword == KW_END) {
                BppToken ntok = lex_next(scan);
                if (ntok.type == TOK_KEYWORD && (ntok.as.keyword == KW_SUB || ntok.as.keyword == KW_FUNCTION)) {
                    if (nest > 0) nest--;
                    else {
                        vm_jump(vm, lines[i].line_number, lines[i].text);
                        lex_shutdown(scan);
                        return err;
                    }
                }
            }
        }
        lex_shutdown(scan);
    }

    err.code = 35; err.message = "FUNCTION without END FUNCTION";
    return err;
}

BppError stmt_end_function_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    (void)lex;

    if (!vm) {
        err.code = 5; err.message = "Null VM context";
        return err;
    }

    BppSubFrame frame;
    if (!vm_sub_pop(vm, &frame)) {
        return err;
    }

    if (frame.line > 0 && frame.pos) {
        vm_jump(vm, frame.line, frame.pos);
    }

    return err;
}

void stmt_function_register(void) {
    static const MicroLibMetadata meta = {
        .name = "FUNCTION",
        .category = "Control Flow",
        .syntax = "FUNCTION name [(parameter_list)] ... END FUNCTION",
        .help_text = "Declares the name, parameters, and code that define a FUNCTION procedure block.",
        .error_codes = "Error 2: Syntax Error, Error 35: SUB/FUNCTION Without END, Error 36: Illegal Parameter List"
    };
    microlib_register(&meta);
}

