// FILENAME: whenever.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (ctype.h, ctype.c, file.h, file.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (eval.h, eval.c, string.c, whenever.h)
// Provides runtime implementation for the WHENEVER statement in BASIC++.
//
// ---- Includes ----

#include "statements/event/trapping/whenever.h"
#include "eval/eval.h"
#include "runtime/file.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>
#include <ctype.h>

#ifdef _WIN32
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#else
#include <strings.h>
#endif

void stmt_whenever_register(void) {
    static const MicroLibMetadata meta = {
        .name = "WHENEVER",
        .category = "Event Trapping",
        .syntax = "WHENEVER {ERROR | [NOT] EOF #channel} THEN {GOTO line | statement}",
        .help_text = "Establishes conditional event traps and exception handlers (IBM VS BASIC / CMS).",
        .error_codes = "Error 2: Syntax Error, Error 5: Illegal Function Call, Error 52: Bad File Number"
    };
    microlib_register(&meta);
}

BppError stmt_whenever_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    if (!vm || !lex) {
        err.code = 5; err.message = "Null VM or lexer context";
        return err;
    }

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_WHENEVER) {
        lex_next(lex);
        tok = lex_peek(lex);
    }

    // Check for WHENEVER OFF or WHENEVER NONE
    if ((tok.type == TOK_KEYWORD && tok.as.keyword == KW_OFF) ||
        (tok.type == TOK_IDENT && (strncasecmp(tok.start, "OFF", 3) == 0 || strncasecmp(tok.start, "NONE", 4) == 0))) {
        lex_next(lex);
        vm_set_error_trap(vm, 0);
        return err;
    }

    // Check for WHENEVER ERROR THEN ...
    if ((tok.type == TOK_KEYWORD && tok.as.keyword == KW_ERROR) ||
        (tok.type == TOK_IDENT && tok.length == 5 && strncasecmp(tok.start, "ERROR", 5) == 0)) {
        lex_next(lex);
        tok = lex_next(lex);
        if (tok.type != TOK_KEYWORD || tok.as.keyword != KW_THEN) {
            err.code = 2; err.message = "Expected THEN after WHENEVER ERROR";
            return err;
        }

        tok = lex_peek(lex);
        if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_GOTO) {
            lex_next(lex);
            BValue val = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            if (val.type == VAL_NUMBER || val.type == VAL_INTEGER) {
                vm_set_error_trap(vm, val.as.number);
            } else {
                err.code = 13; err.message = "Type mismatch in line number";
                if (val.type == VAL_STRING && val.as.string) str_release(vm_get_str(vm), val.as.string);
                return err;
            }
        } else if (tok.type == TOK_NUMBER) {
            BValue val = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            vm_set_error_trap(vm, val.as.number);
        } else {
            // Execute statement
            err = vm_execute_line(vm, lex_get_pos(lex));
        }
        return err;
    }

    // Check for WHENEVER [NOT] EOF #channel THEN ...
    bool is_not = false;
    if ((tok.type == TOK_KEYWORD && tok.as.keyword == KW_NOT) ||
        (tok.type == TOK_IDENT && tok.length == 3 && strncasecmp(tok.start, "NOT", 3) == 0)) {
        is_not = true;
        lex_next(lex);
        tok = lex_peek(lex);
    }

    if (tok.type == TOK_IDENT && tok.length == 3 && strncasecmp(tok.start, "EOF", 3) == 0) {
        lex_next(lex);
        tok = lex_peek(lex);
        int channel = 1;
        if (tok.type == TOK_HASH) {
            lex_next(lex);
            BValue ch_val = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            channel = (int)ch_val.as.number;
        }

        tok = lex_next(lex);
        if (tok.type != TOK_KEYWORD || tok.as.keyword != KW_THEN) {
            err.code = 2; err.message = "Expected THEN after WHENEVER EOF";
            return err;
        }

        FileContext *fc = vm_get_file(vm);
        bool is_eof = file_eof(fc, channel);
        bool should_trigger = is_not ? !is_eof : is_eof;

        if (should_trigger) {
            tok = lex_peek(lex);
            if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_GOTO) {
                lex_next(lex);
                BValue val = eval_expression(vm, lex, &err);
                if (err.code != 0) return err;
                if (val.type == VAL_NUMBER || val.type == VAL_INTEGER) {
                    BppLineNumber target_line = (BppLineNumber)val.as.number;
                    const char *target_src = mem_program_get(vm_get_mem(vm), target_line);
                    if (target_src) {
                        vm_jump(vm, target_line, target_src);
                    } else {
                        err.code = 8; err.message = "Undefined line number in WHENEVER GOTO";
                        return err;
                    }
                }
            } else if (tok.type == TOK_NUMBER) {
                BValue val = eval_expression(vm, lex, &err);
                if (err.code != 0) return err;
                BppLineNumber target_line = (BppLineNumber)val.as.number;
                const char *target_src = mem_program_get(vm_get_mem(vm), target_line);
                if (target_src) {
                    vm_jump(vm, target_line, target_src);
                } else {
                    err.code = 8; err.message = "Undefined line number in WHENEVER GOTO";
                    return err;
                }
            } else {
                err = vm_execute_line(vm, lex_get_pos(lex));
            }
        }
        return err;
    }

    err.code = 2;
    err.message = "Syntax error in WHENEVER statement";
    return err;
}
