// FILENAME: for.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore, libengine, libkernel, libplatform
// Provides runtime implementation for the FOR statement in BASIC++.
//
// ---- Includes ----

#include "statements/loops/counter/for.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/variables.h"
#include "memory/memory.h"
#include "device/vdev.h"
#include "security/security.h"
#include "platform/platform.h"
#include <string.h>

void stmt_for_register(void) {
    MicroLibMetadata meta = {
        .name = "FOR",
        .category = "Looping / Control Flow",
        .syntax = "FOR var = start TO end [STEP step]",
        .help_text = "Initiates a counter-controlled loop block executing statements until var reaches end.",
        .error_codes = "Error 2: Syntax Error, Error 13: Type Mismatch, Error 26: FOR Without NEXT"
    };
    microlib_register(&meta);
}

BppError stmt_for_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    if (!vm || !lex) {
        err.code = 5; err.message = "Null VM or lexer context";
        return err;
    }

    char var_names[8][64];
    int var_count = 0;

    while (var_count < 8) {
        BppToken var_tok = lex_next(lex);
        if (var_tok.type != TOK_IDENT && var_tok.type != TOK_KEYWORD) {
            err.code = 2; err.message = "Expected loop variable name after FOR";
            return err;
        }
        size_t vlen = (var_tok.length < sizeof(var_names[var_count]) - 1) ? var_tok.length : sizeof(var_names[var_count]) - 1;
        memcpy(var_names[var_count], var_tok.start, vlen);
        var_names[var_count][vlen] = '\0';
        var_count++;

        BppToken sep = lex_peek(lex);
        if (sep.type == TOK_COMMA) {
            lex_next(lex); // Consume ','
            continue;
        }
        if (sep.type == TOK_EQ || (sep.start && sep.start[0] == '=')) {
            // Check if followed by another variable name and another '=' or ','
            LexerContext *look_lex = lex_init(vm_get_mem(vm), lex_get_pos(lex));
            lex_next(look_lex); // Consume '='
            BppToken look_tok = lex_next(look_lex);
            bool is_chained_var = false;
            if (look_tok.type == TOK_IDENT || look_tok.type == TOK_KEYWORD) {
                BppToken after_tok = lex_peek(look_lex);
                if (after_tok.type == TOK_EQ || (after_tok.start && after_tok.start[0] == '=') || after_tok.type == TOK_COMMA) {
                    is_chained_var = true;
                }
            }
            lex_shutdown(look_lex);
            if (is_chained_var) {
                lex_next(lex); // Consume '='
                continue;
            }
            lex_next(lex); // Consume final '=' before start expression
            break;
        }

        err.code = 2; err.message = "Expected '=' after loop variable";
        return err;
    }

    BValue start_val = eval_expression(vm, lex, &err);
    if (err.code != 0) {
        return err;
    }
    if (start_val.type == VAL_STRING) {
        str_release(vm_get_str(vm), start_val.as.string);
        err.code = 13; err.message = "Type mismatch in FOR start expression";
        return err;
    }

    double end_num = 0.0;
    double step_num = 1.0;

    BppToken to_or_by = lex_next(lex);
    bool is_to = (to_or_by.type == TOK_KEYWORD && to_or_by.as.keyword == KW_TO) ||
                 (to_or_by.type == TOK_IDENT && to_or_by.length == 2 && strncasecmp(to_or_by.start, "TO", 2) == 0);
    bool is_by = (to_or_by.type == TOK_KEYWORD && (to_or_by.as.keyword == KW_BY || to_or_by.as.keyword == KW_STEP)) ||
                 (to_or_by.type == TOK_IDENT && ((to_or_by.length == 2 && strncasecmp(to_or_by.start, "BY", 2) == 0) ||
                                                 (to_or_by.length == 4 && strncasecmp(to_or_by.start, "STEP", 4) == 0)));

    if (is_to) {
        // Standard: FOR var = start TO limit [STEP/BY step]
        BValue end_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (end_val.type == VAL_STRING) {
            str_release(vm_get_str(vm), end_val.as.string);
            err.code = 13; err.message = "Type mismatch in FOR end expression";
            return err;
        }
        end_num = end_val.as.number;

        BppToken step_tok = lex_peek(lex);
        bool has_step = (step_tok.type == TOK_KEYWORD && (step_tok.as.keyword == KW_STEP || step_tok.as.keyword == KW_BY)) ||
                        (step_tok.type == TOK_IDENT && ((step_tok.length == 4 && strncasecmp(step_tok.start, "STEP", 4) == 0) ||
                                                        (step_tok.length == 2 && strncasecmp(step_tok.start, "BY", 2) == 0)));
        if (has_step) {
            lex_next(lex);
            BValue step_val = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            if (step_val.type == VAL_STRING) {
                str_release(vm_get_str(vm), step_val.as.string);
                err.code = 13; err.message = "Type mismatch in FOR step expression";
                return err;
            }
            step_num = step_val.as.number;
        }
    } else if (is_by) {
        // Super BASIC: FOR var = start BY step [TO limit]
        BValue step_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (step_val.type == VAL_STRING) {
            str_release(vm_get_str(vm), step_val.as.string);
            err.code = 13; err.message = "Type mismatch in FOR step expression";
            return err;
        }
        step_num = step_val.as.number;

        BppToken peek_to = lex_peek(lex);
        bool has_to = (peek_to.type == TOK_KEYWORD && peek_to.as.keyword == KW_TO) ||
                      (peek_to.type == TOK_IDENT && peek_to.length == 2 && strncasecmp(peek_to.start, "TO", 2) == 0);
        if (has_to) {
            lex_next(lex); // Consume TO
            BValue end_val = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            if (end_val.type == VAL_STRING) {
                str_release(vm_get_str(vm), end_val.as.string);
                err.code = 13; err.message = "Type mismatch in FOR end expression";
                return err;
            }
            end_num = end_val.as.number;
        } else {
            // Open-ended FOR..BY loop: iterates indefinitely until explicit exit/jump
            end_num = (step_num >= 0.0) ? 1e308 : -1e308;
        }
    } else {
        err.code = 2; err.message = "Expected 'TO' or 'BY' in FOR statement";
        return err;
    }

    VariableContext *vc = vm_get_var(vm);
    BValue init_val = { .type = VAL_NUMBER, .as.number = start_val.as.number };
    for (int i = 0; i < var_count; i++) {
        var_assign(vc, var_names[i], init_val);
    }

    BppLineNumber cur_line = vm_get_current_line(vm);
    const char *pos = vm_map_source_pos(vm, lex_get_pos(lex));

    BppToken next_tok = lex_peek(lex);
    if (next_tok.type == TOK_EOF || (next_tok.type == TOK_EOL && *next_tok.start != ':')) {
        // FOR is at the end of the line: loop body begins on the next program line
        MemoryContext *mem = vm_get_mem(vm);
        size_t count = 0;
        BppProgramLine *lines = mem_program_get_all(mem, &count);
        for (size_t i = 0; i < count; ++i) {
            if (lines[i].line_number == cur_line && i + 1 < count) {
                cur_line = lines[i + 1].line_number;
                pos = NULL;
                break;
            }
        }
    }

    const char *names_ptrs[8];
    for (int i = 0; i < var_count; i++) names_ptrs[i] = var_names[i];
    if (!vm_for_push_multi(vm, names_ptrs, var_count, end_num, step_num, cur_line, pos)) {
        err.code = 26; err.message = "FOR loop stack overflow";
        return err;
    }

    bool skip_loop = (step_num >= 0.0) ? (start_val.as.number > end_num)
                                       : (start_val.as.number < end_num);
    if (skip_loop) {
        BppForFrame frame;
        vm_for_pop(vm, var_names[0], &frame);

        MemoryContext *mem = vm_get_mem(vm);
        size_t count = 0;
        BppProgramLine *lines = mem_program_get_all(mem, &count);
        for (size_t i = 0; i < count; ++i) {
            if (lines[i].line_number == cur_line && i + 1 < count) {
                for (size_t j = i + 1; j < count; ++j) {
                    LexerContext *scan = lex_init(mem, lines[j].text);
                    BppToken ntok = lex_next(scan);
                    if (ntok.type == TOK_KEYWORD && ntok.as.keyword == KW_NEXT) {
                        lex_shutdown(scan);
                        if (j + 1 < count) {
                            vm_jump(vm, lines[j + 1].line_number, lines[j + 1].text);
                        } else {
                            vm_jump(vm, lines[j].line_number + 1, "");
                        }
                        return err;
                    }
                    lex_shutdown(scan);
                }
            }
        }
    }

    return err;
}
