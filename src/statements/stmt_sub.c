/**
 * @file stmt_sub.c
 * @brief Statement handlers for SUB, FUNCTION, DECLARE, and CALL.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Implements modular subprogram and function scoping and calls.
 * - Why it exists: Enables procedural, reusable, and structured programming in BASIC++.
 * - Why it works this way: It uses the SubStack for call frames, prepends the active procedure
 *   to local variable names, and executes FUNCTIONs synchronously by running an iterative
 *   fetch-decode loop over the function definition's statements.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Limits on nesting depths or maximum argument counts (currently 8).
 * - What cannot be changed: Variable prefixing scoping rules and synchronous execution limits.
 * - What to expect: Entering a SUB skips its body during sequential program execution.
 * - What to do if something breaks: If variables leak, verify var_clear_scope removes all prefixed entries.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: Procedure names are case-insensitive. Parameter types match argument types.
 * - Portability concerns: None. C17 standard compliant.
 */

#include "bpp_stmt.h"
#include "bpp_eval.h"
#include "bpp_vm.h"
#include "bpp_variables.h"
#include "bpp_map.h"
#include "bpp_strings.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_PARAMS 8

/* Forward declarations */


/**
 * @brief Helper to skip lines until END SUB or END FUNCTION is found at current nesting level.
 */
static BppError skip_to_end_proc(VMContext *vm, BppKeywordId end_kw) {
    BppError err;
    memset(&err, 0, sizeof(err));

    MemoryContext *mem = vm_get_mem(vm);
    size_t count = 0;
    BppProgramLine *lines = mem_program_get_all(mem, &count);

    BppLineNumber cur_ln = vm_get_current_line(vm);
    size_t start_idx = 0;
    bool found = false;
    bool is_lib = false;

    for (size_t i = 0; i < count; ++i) {
        if (lines[i].line_number == cur_ln) {
            start_idx = i;
            found = true;
            break;
        }
    }

    size_t lib_count = 0;
    BppProgramLine *lib_lines = NULL;
    if (!found) {
        lib_lines = mem_lib_program_get_all(mem, &lib_count);
        for (size_t i = 0; i < lib_count; ++i) {
            if (lib_lines[i].line_number == cur_ln) {
                start_idx = i;
                found = true;
                is_lib = true;
                break;
            }
        }
    }

    if (!found) {
        err.code = 11; err.message = "Execution state corruption during procedure scan";
        return err;
    }

    int nesting = 0;
    BppProgramLine *active_lines = is_lib ? lib_lines : lines;
    size_t active_count = is_lib ? lib_count : count;

    for (size_t i = start_idx + 1; i < active_count; ++i) {
        LexerContext *scan_lex = lex_init(mem, active_lines[i].text);
        BppToken tok = lex_next(scan_lex);

        if (tok.type == TOK_KEYWORD) {
            if (tok.as.keyword == KW_SUB || tok.as.keyword == KW_FUNCTION || tok.as.keyword == KW_PROCEDURE) {
                nesting++;
            } else if (tok.as.keyword == KW_END) {
                BppToken next_tok = lex_next(scan_lex);
                if (next_tok.type == TOK_KEYWORD &&
                    (next_tok.as.keyword == end_kw ||
                     (end_kw == KW_SUB && next_tok.as.keyword == KW_PROCEDURE))) {
                    if (nesting > 0) {
                        nesting--;
                    } else {
                        if (i + 1 < active_count) {
                            vm_jump(vm, active_lines[i + 1].line_number, active_lines[i + 1].text);
                        } else {
                            vm_jump(vm, 999999.0, NULL);
                            if (sub_stack_depth(vm_get_sub_stack(vm)) == 0) {
                                vm_halt(vm);
                            }
                        }
                        lex_shutdown(scan_lex);
                        return err;
                    }
                }
            } else if (tok.as.keyword == KW_ENDFUNC && end_kw == KW_FUNCTION) {
                if (nesting > 0) {
                    nesting--;
                } else {
                    if (i + 1 < active_count) {
                        vm_jump(vm, active_lines[i + 1].line_number, active_lines[i + 1].text);
                    } else {
                        vm_jump(vm, 999999.0, NULL);
                        if (sub_stack_depth(vm_get_sub_stack(vm)) == 0) {
                            vm_halt(vm);
                        }
                    }
                    lex_shutdown(scan_lex);
                    return err;
                }
            } else if (tok.as.keyword == KW_ENDPROC && end_kw == KW_SUB) {
                if (nesting > 0) {
                    nesting--;
                } else {
                    if (i + 1 < active_count) {
                        vm_jump(vm, active_lines[i + 1].line_number, active_lines[i + 1].text);
                    } else {
                        vm_jump(vm, 999999.0, NULL);
                        if (sub_stack_depth(vm_get_sub_stack(vm)) == 0) {
                            vm_halt(vm);
                        }
                    }
                    lex_shutdown(scan_lex);
                    return err;
                }
            }
        }
        lex_shutdown(scan_lex);
    }

    err.code = 2; err.message = "SUB/FUNCTION defined without END SUB/FUNCTION";
    return err;
}

/**
 * @brief Helper to scan the program for SUB/FUNCTION definition.
 */
#include "bpp_metadata.h"

static void get_namespace_at_line(VMContext *vm, BppLineNumber target_line, char *out_ns, size_t max_len) {
    out_ns[0] = '\0';
    size_t count = 0;
    BppProgramLine *lines = mem_program_get_all(vm_get_mem(vm), &count);
    char current_ns[64] = "";

    for (size_t i = 0; i < count; i++) {
        if (lines[i].line_number > target_line) {
            break;
        }
        LexerContext *scan_lex = lex_init(vm_get_mem(vm), lines[i].text);
        if (scan_lex) {
            BppToken tok = lex_next(scan_lex);
            if (tok.type == TOK_NAMESPACE_DECL) {
                int len = (int)(tok.length < sizeof(current_ns) - 1 ? tok.length : sizeof(current_ns) - 1);
                memcpy(current_ns, tok.as.string, len);
                current_ns[len] = '\0';
                if (strcasecmp(current_ns, "DEFAULT") == 0) {
                    current_ns[0] = '\0';
                }
            }
            lex_shutdown(scan_lex);
        }
    }
    size_t copy_len = strlen(current_ns);
    if (copy_len >= max_len) copy_len = max_len - 1;
    memcpy(out_ns, current_ns, copy_len);
    out_ns[copy_len] = '\0';
}

bool find_procedure(VMContext *vm, const char *name, BppKeywordId proc_kw, BppLineNumber *out_line, const char **out_text) {
    size_t count = 0;
    BppProgramLine *lines = mem_program_get_all(vm_get_mem(vm), &count);

    char name_upper[256];
    size_t nlen = strlen(name);
    if (nlen > sizeof(name_upper) - 1) nlen = sizeof(name_upper) - 1;
    for (size_t i = 0; i < nlen; i++) name_upper[i] = (char)toupper((unsigned char)name[i]);
    name_upper[nlen] = '\0';

    // 1. Get caller's namespace
    char caller_ns[64];
    get_namespace_at_line(vm, vm_get_current_line(vm), caller_ns, sizeof(caller_ns));

    // Determine target name options to match (qualified vs unqualified)
    char target1[512] = ""; // Prefixed with caller_ns: e.g. "MATH.ADD"
    char target2[512] = ""; // As provided: e.g. "ADD" or "MATH.ADD"
    
    if (strchr(name_upper, '.') != NULL) {
        snprintf(target2, sizeof(target2), "%s", name_upper);
    } else {
        if (caller_ns[0] != '\0') {
            snprintf(target1, sizeof(target1), "%s.%s", caller_ns, name_upper);
            for (size_t i = 0; target1[i]; i++) target1[i] = (char)toupper((unsigned char)target1[i]);
        }
        snprintf(target2, sizeof(target2), "%s", name_upper);
    }

    char current_ns[64] = "";

    // Helper search function to find matches
    for (int pass = 0; pass < 2; pass++) {
        // Pass 0: check target1 (prefixed namespaced)
        // Pass 1: check target2 (global or qualified namespaced)
        const char *search_target = (pass == 0) ? target1 : target2;
        if (search_target[0] == '\0') continue;

        current_ns[0] = '\0';
        char current_class[64] = "";

        for (size_t i = 0; i < count; ++i) {
            LexerContext *scan_lex = lex_init(vm_get_mem(vm), lines[i].text);
            if (!scan_lex) continue;

            BppToken tok = lex_next(scan_lex);
            if (tok.type == TOK_NAMESPACE_DECL) {
                int len = (int)(tok.length < sizeof(current_ns) - 1 ? tok.length : sizeof(current_ns) - 1);
                memcpy(current_ns, tok.as.string, len);
                current_ns[len] = '\0';
                if (strcasecmp(current_ns, "DEFAULT") == 0) {
                    current_ns[0] = '\0';
                }
            }
            else if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_CLASS) {
                BppToken c_tok = lex_next(scan_lex);
                if (c_tok.type == TOK_IDENT) {
                    size_t clen = (c_tok.length < sizeof(current_class) - 1) ? c_tok.length : sizeof(current_class) - 1;
                    memcpy(current_class, c_tok.start, clen);
                    current_class[clen] = '\0';
                }
            }
            else if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_END) {
                BppToken c_tok = lex_next(scan_lex);
                if (c_tok.type == TOK_KEYWORD && c_tok.as.keyword == KW_CLASS) {
                    current_class[0] = '\0';
                }
            }
            else if (tok.type == TOK_KEYWORD && (tok.as.keyword == proc_kw || (proc_kw == KW_SUB && tok.as.keyword == KW_PROCEDURE))) {
                tok = lex_next(scan_lex);
                if (tok.type == TOK_IDENT) {
                    char temp[256];
                    size_t tlen = (tok.length < sizeof(temp) - 1) ? tok.length : sizeof(temp) - 1;
                    memcpy(temp, tok.start, tlen);
                    for (size_t k = 0; k < tlen; k++) temp[k] = (char)toupper((unsigned char)temp[k]);
                    temp[tlen] = '\0';

                    char fully_qualified[512];
                    if (current_class[0] != '\0') {
                        snprintf(fully_qualified, sizeof(fully_qualified), "%s.%s", current_class, temp);
                    } else if (current_ns[0] != '\0') {
                        snprintf(fully_qualified, sizeof(fully_qualified), "%s.%s", current_ns, temp);
                    } else {
                        snprintf(fully_qualified, sizeof(fully_qualified), "%s", temp);
                    }
                    for (size_t k = 0; fully_qualified[k]; k++) fully_qualified[k] = (char)toupper((unsigned char)fully_qualified[k]);

                    if (strcmp(fully_qualified, search_target) == 0) {
                        if (out_line) *out_line = lines[i].line_number;
                        if (out_text) *out_text = lines[i].text;
                        lex_shutdown(scan_lex);
                        return true;
                    }
                }
            }
            lex_shutdown(scan_lex);
        }

        /* Scan companion library program store */
        size_t lib_count = 0;
        BppProgramLine *lib_lines = mem_lib_program_get_all(vm_get_mem(vm), &lib_count);
        if (lib_lines) {
            current_ns[0] = '\0';
            current_class[0] = '\0';
            for (size_t i = 0; i < lib_count; ++i) {
                LexerContext *scan_lex = lex_init(vm_get_mem(vm), lib_lines[i].text);
                if (!scan_lex) continue;

                BppToken tok = lex_next(scan_lex);
                if (tok.type == TOK_NAMESPACE_DECL) {
                    int len = (int)(tok.length < sizeof(current_ns) - 1 ? tok.length : sizeof(current_ns) - 1);
                    memcpy(current_ns, tok.as.string, len);
                    current_ns[len] = '\0';
                    if (strcasecmp(current_ns, "DEFAULT") == 0) {
                        current_ns[0] = '\0';
                    }
                }
                else if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_CLASS) {
                    BppToken c_tok = lex_next(scan_lex);
                    if (c_tok.type == TOK_IDENT) {
                        size_t clen = (c_tok.length < sizeof(current_class) - 1) ? c_tok.length : sizeof(current_class) - 1;
                        memcpy(current_class, c_tok.start, clen);
                        current_class[clen] = '\0';
                    }
                }
                else if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_END) {
                    BppToken c_tok = lex_next(scan_lex);
                    if (c_tok.type == TOK_KEYWORD && c_tok.as.keyword == KW_CLASS) {
                        current_class[0] = '\0';
                    }
                }
                else if (tok.type == TOK_KEYWORD && tok.as.keyword == proc_kw) {
                    tok = lex_next(scan_lex);
                    if (tok.type == TOK_IDENT) {
                        char temp[256];
                        size_t tlen = (tok.length < sizeof(temp) - 1) ? tok.length : sizeof(temp) - 1;
                        memcpy(temp, tok.start, tlen);
                        for (size_t k = 0; k < tlen; k++) temp[k] = (char)toupper((unsigned char)temp[k]);
                        temp[tlen] = '\0';

                        char fully_qualified[512];
                        if (current_class[0] != '\0') {
                            snprintf(fully_qualified, sizeof(fully_qualified), "%s.%s", current_class, temp);
                        } else if (current_ns[0] != '\0') {
                            snprintf(fully_qualified, sizeof(fully_qualified), "%s.%s", current_ns, temp);
                        } else {
                            snprintf(fully_qualified, sizeof(fully_qualified), "%s", temp);
                        }
                        for (size_t k = 0; fully_qualified[k]; k++) fully_qualified[k] = (char)toupper((unsigned char)fully_qualified[k]);

                        if (strcmp(fully_qualified, search_target) == 0) {
                            if (out_line) *out_line = lib_lines[i].line_number;
                            if (out_text) *out_text = lib_lines[i].text;
                            lex_shutdown(scan_lex);
                            return true;
                        }
                    }
                }
                lex_shutdown(scan_lex);
            }
        }
    }

    return false;
}

/**
 * @brief SUB statement definition handler (skips body during sequential runs).
 */
BppError stmt_sub_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    /* Skip past body to END SUB */
    return skip_to_end_proc(vm, KW_SUB);
}

/**
 * @brief PROCEDURE statement definition handler (skips body during sequential runs).
 */
BppError stmt_procedure_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    /* Skip past body to ENDPROC */
    return skip_to_end_proc(vm, KW_SUB);
}

/**
 * @brief FUNCTION statement definition handler (skips body during sequential runs).
 */
BppError stmt_function_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    /* Skip past body to END FUNCTION */
    return skip_to_end_proc(vm, KW_FUNCTION);
}

/**
 * @brief DECLARE statement handler (NOP in interpreter mode).
 */
BppError stmt_declare_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    /* Skip to EOL */
    BppToken tok = lex_next(lex);
    while (tok.type != TOK_EOL && tok.type != TOK_EOF) {
        tok = lex_next(lex);
    }
    return err;
}

/**
 * @brief CALL statement handler.
 * Syntax: CALL subname(arg1, arg2, ...)
 */
BppError stmt_call_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    /* 1. Parse Sub name */
    BppToken name_tok = lex_next(lex);
    if (name_tok.type != TOK_IDENT) {
        err.code = 2; err.message = "Expected subprogram name in CALL";
        return err;
    }
    char sub_name[256];
    size_t slen = (name_tok.length < sizeof(sub_name) - 1) ? name_tok.length : sizeof(sub_name) - 1;
    memcpy(sub_name, name_tok.start, slen);
    sub_name[slen] = '\0';

    /* 2. Find SUB definition */
    BppLineNumber def_line = 0.0;
    const char *def_text = NULL;
    if (!find_procedure(vm, sub_name, KW_SUB, &def_line, &def_text)) {
        err.code = 35; err.message = "Subprogram not defined";
        return err;
    }

    /* 3. Evaluate arguments in caller's scope */
    BValue args[MAX_PARAMS];
    int arg_count = 0;

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_LPAREN) {
        lex_next(lex); /* Consume '(' */
        while (true) {
            tok = lex_peek(lex);
            if (tok.type == TOK_RPAREN) {
                lex_next(lex);
                break;
            }

            if (arg_count >= MAX_PARAMS) {
                err.code = 2; err.message = "Too many arguments in CALL";
                for (int i = 0; i < arg_count; i++) {
                    if (args[i].type == VAL_STRING) str_release(vm_get_str(vm), args[i].as.string);
                }
                return err;
            }

            args[arg_count++] = eval_expression(vm, lex, &err);
            if (err.code != 0) {
                for (int i = 0; i < arg_count - 1; i++) {
                    if (args[i].type == VAL_STRING) str_release(vm_get_str(vm), args[i].as.string);
                }
                return err;
            }

            tok = lex_peek(lex);
            if (tok.type == TOK_COMMA) {
                lex_next(lex);
            } else if (tok.type == TOK_RPAREN) {
                lex_next(lex);
                break;
            } else {
                err.code = 2; err.message = "Expected ',' or ')' in argument list";
                for (int i = 0; i < arg_count; i++) {
                    if (args[i].type == VAL_STRING) str_release(vm_get_str(vm), args[i].as.string);
                }
                return err;
            }
        }
    }

    /* 4. Parse parameter names from definition text */
    LexerContext *def_lex = lex_init(vm_get_mem(vm), def_text);
    /* Skip SUB MYPROC */
    lex_next(def_lex);
    lex_next(def_lex);

    char params[MAX_PARAMS][64];
    int param_count = 0;

    tok = lex_peek(def_lex);
    if (tok.type == TOK_LPAREN) {
        lex_next(def_lex);
        while (true) {
            tok = lex_next(def_lex);
            if (tok.type == TOK_IDENT) {
                if (param_count >= MAX_PARAMS) {
                    err.code = 2; err.message = "Too many parameters in SUB definition";
                    lex_shutdown(def_lex);
                    for (int i = 0; i < arg_count; i++) {
                        if (args[i].type == VAL_STRING) str_release(vm_get_str(vm), args[i].as.string);
                    }
                    return err;
                }
                size_t plen = (tok.length < 63) ? tok.length : 63;
                memcpy(params[param_count], tok.start, plen);
                params[param_count][plen] = '\0';
                param_count++;

                tok = lex_peek(def_lex);
                if (tok.type == TOK_COMMA) {
                    lex_next(def_lex);
                } else if (tok.type == TOK_RPAREN) {
                    lex_next(def_lex);
                    break;
                } else {
                    err.code = 2; err.message = "Expected ',' or ')' in parameter list";
                    lex_shutdown(def_lex);
                    for (int i = 0; i < arg_count; i++) {
                        if (args[i].type == VAL_STRING) str_release(vm_get_str(vm), args[i].as.string);
                    }
                    return err;
                }
            } else if (tok.type == TOK_RPAREN) {
                break;
            } else {
                err.code = 2; err.message = "Expected parameter name in SUB definition";
                lex_shutdown(def_lex);
                for (int i = 0; i < arg_count; i++) {
                    if (args[i].type == VAL_STRING) str_release(vm_get_str(vm), args[i].as.string);
                }
                return err;
            }
        }
    }
    const char *first_stmt = lex_get_pos(def_lex);
    lex_shutdown(def_lex);

    if (arg_count != param_count) {
        err.code = 37; err.message = "Argument count mismatch";
        for (int i = 0; i < arg_count; i++) {
            if (args[i].type == VAL_STRING) str_release(vm_get_str(vm), args[i].as.string);
        }
        return err;
    }

    /* 5. Push Call Frame */
    const char *ret_pos = lex_get_pos(lex);
    if (!vm_sub_push(vm, sub_name, vm_get_current_line(vm), ret_pos, false)) {
        err.code = 7; err.message = "SUB stack overflow";
        for (int i = 0; i < arg_count; i++) {
            if (args[i].type == VAL_STRING) str_release(vm_get_str(vm), args[i].as.string);
        }
        return err;
    }

    /* 6. Bind variables in new scope */
    var_set_scope(vm_get_var(vm), sub_name);
    for (int i = 0; i < param_count; i++) {
        BValue *p_var = var_declare(vm_get_var(vm), params[i]);
        if (p_var) {
            *p_var = args[i];
            /* Keep string references */
        }
    }

    /* 7. Jump execution to sub body first statement */
    vm_jump(vm, def_line, first_stmt);

    return err;
}

/**
 * @brief END SUB statement handler.
 */
BppError stmt_end_sub_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppSubFrame frame;
    if (!vm_sub_pop(vm, &frame)) {
        err.code = 2; err.message = "END SUB without CALL";
        return err;
    }

    /* Restore parent scope */
    BppSubFrame parent;
    if (vm_sub_peek(vm, &parent)) {
        var_set_scope(vm_get_var(vm), parent.name);
    } else {
        var_set_scope(vm_get_var(vm), NULL);
    }

    /* Jump back */
    vm_jump(vm, frame.line, frame.pos);

    return err;
}

/**
 * @brief END FUNCTION statement handler.
 */
BppError stmt_end_function_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    /* Consume any trailing arguments/tokens on the line */
    BppToken tok = lex_next(lex);
    while (tok.type != TOK_EOL && tok.type != TOK_EOF) {
        tok = lex_next(lex);
    }

    return err;
}

/**
 * @brief Helper for synchronous FUNCTION execution. Called from expression evaluator.
 */
BValue invoke_user_function(VMContext *vm, const char *name, BValue *args, int argc, BppError *err) {
    BValue res = {0};
    res.type = VAL_NUMBER;
    res.as.number = 0.0;

    /* 1. Find FUNCTION definition */
    BppLineNumber def_line = 0.0;
    const char *def_text = NULL;
    if (!find_procedure(vm, name, KW_FUNCTION, &def_line, &def_text)) {
        err->code = 35; err->message = "Function not defined";
        return res;
    }

    /* 2. Parse Parameter names */
    LexerContext *def_lex = lex_init(vm_get_mem(vm), def_text);
    lex_next(def_lex); /* Consume FUNCTION */
    lex_next(def_lex); /* Consume name */

    char params[MAX_PARAMS][64];
    int param_count = 0;

    BppToken tok = lex_peek(def_lex);
    if (tok.type == TOK_LPAREN) {
        lex_next(def_lex);
        while (true) {
            tok = lex_next(def_lex);
            if (tok.type == TOK_IDENT) {
                if (param_count >= MAX_PARAMS) {
                    err->code = 2; err->message = "Too many parameters in FUNCTION definition";
                    lex_shutdown(def_lex);
                    return res;
                }
                size_t plen = (tok.length < 63) ? tok.length : 63;
                memcpy(params[param_count], tok.start, plen);
                params[param_count][plen] = '\0';
                param_count++;

                tok = lex_peek(def_lex);
                if (tok.type == TOK_COMMA) {
                    lex_next(def_lex);
                } else if (tok.type == TOK_RPAREN) {
                    lex_next(def_lex);
                    break;
                } else {
                    err->code = 2; err->message = "Expected ',' or ')' in parameter list";
                    lex_shutdown(def_lex);
                    return res;
                }
            } else if (tok.type == TOK_RPAREN) {
                break;
            } else {
                err->code = 2; err->message = "Expected parameter name in FUNCTION definition";
                lex_shutdown(def_lex);
                return res;
            }
        }
    }
    lex_shutdown(def_lex);

    /* Check if this is a class method call */
    bool is_method = false;
    BppMap *method_this = NULL;
    if (strchr(name, '.') != NULL && argc > 0 && args[0].type == VAL_MAP && args[0].as.map) {
        /* Verify it has __type__ metadata */
        BValue type_val;
        if (bpp_map_get(args[0].as.map, "__type__", &type_val) && type_val.type == VAL_STRING) {
            is_method = true;
            method_this = args[0].as.map;
        }
    }

    int actual_argc = is_method ? (argc - 1) : argc;
    if (actual_argc != param_count) {
        err->code = 37; err->message = "Argument count mismatch in function call";
        return res;
    }

    /* Save parent execution state */
    BppVMState saved_state;
    vm_save_state(vm, &saved_state);
    const char *parent_scope = vm_get_active_proc(vm);

    /* Push Call Frame */
    if (!vm_sub_push(vm, name, saved_state.current_line, saved_state.current_pos, true)) {
        err->code = 7; err->message = "Call stack overflow in function invocation";
        return res;
    }

    /* Bind variables in new scope */
    var_set_scope(vm_get_var(vm), name);
    for (int i = 0; i < param_count; i++) {
        BValue *p_var = var_declare(vm_get_var(vm), params[i]);
        if (p_var) {
            *p_var = args[is_method ? (i + 1) : i];
            if (p_var->type == VAL_STRING && p_var->as.string) {
                str_add_ref(p_var->as.string);
            } else if (p_var->type == VAL_MAP && p_var->as.map) {
                bpp_map_add_ref(p_var->as.map);
            }
        }
    }

    /* Copy-in fields from 'this' object map to local scope variables */
    if (is_method && method_this) {
        for (int i = 0; i < method_this->count; i++) {
            if (method_this->entries[i].key != NULL) {
                const char *key = method_this->entries[i].key;
                if (strcmp(key, "__type__") == 0) continue;
                
                BValue val = method_this->entries[i].val;
                BValue *local_var = var_declare(vm_get_var(vm), key);
                if (local_var) {
                    *local_var = val;
                    if (local_var->type == VAL_STRING && local_var->as.string) {
                        str_add_ref(local_var->as.string);
                    } else if (local_var->type == VAL_MAP && local_var->as.map) {
                        bpp_map_add_ref(local_var->as.map);
                    }
                }
            }
        }
    }

    /* Set up return value placeholder */
    const char *base_name = strrchr(name, '.');
    if (base_name) {
        base_name++; /* skip the dot */
    } else {
        base_name = name;
    }
    BValue *ret_val_ptr = var_declare(vm_get_var(vm), base_name);
    if (ret_val_ptr) {
        /* Determine return type by suffix of function name */
        size_t nlen = strlen(name);
        if (name[nlen - 1] == '$') {
            ret_val_ptr->type = VAL_STRING;
            ret_val_ptr->as.string = NULL;
        } else {
            ret_val_ptr->type = VAL_NUMBER;
            ret_val_ptr->as.number = 0.0;
        }
    }

    /* 3. Run synchronous fetch-decode loop for the function body statements */
    BppLineNumber curr_line = def_line;

    size_t count = 0;
    BppProgramLine *lines = mem_program_get_all(vm_get_mem(vm), &count);

    bool is_lib = false;
    size_t lib_count = 0;
    BppProgramLine *lib_lines = mem_lib_program_get_all(vm_get_mem(vm), &lib_count);
    if (lib_lines) {
        for (size_t i = 0; i < lib_count; i++) {
            if (lib_lines[i].line_number == def_line) {
                is_lib = true;
                break;
            }
        }
    }

    BppProgramLine *active_lines = is_lib ? lib_lines : lines;
    size_t active_count = is_lib ? lib_count : count;

    bool running = true;
    while (running && !vm_has_error(vm)) {
        vm_set_current_line(vm, curr_line);
        /* Fetch next statement */
        if (is_lib) {
            active_lines = mem_lib_program_get_all(vm_get_mem(vm), &active_count);
        } else {
            active_lines = mem_program_get_all(vm_get_mem(vm), &active_count);
        }
        size_t idx = 0;
        bool found = false;
        for (size_t i = 0; i < active_count; ++i) {
            if (active_lines[i].line_number == curr_line) {
                idx = i; found = true; break;
            }
        }
        if (!found) {
            for (size_t i = 0; i < active_count; ++i) {
                if (active_lines[i].line_number >= curr_line) {
                    idx = i; found = true; curr_line = active_lines[i].line_number; break;
                }
            }
        }
        if (!found) break;

        /* Check if we hit END FUNCTION or ENDFUNC */
        LexerContext *lex = lex_init(vm_get_mem(vm), active_lines[idx].text);
        BppToken end_tok = lex_next(lex);
        if (end_tok.type == TOK_KEYWORD) {
            if (end_tok.as.keyword == KW_ENDFUNC) {
                lex_shutdown(lex);
                break; /* Finished function execution successfully */
            }
            if (end_tok.as.keyword == KW_END) {
                end_tok = lex_next(lex);
                if (end_tok.type == TOK_KEYWORD && end_tok.as.keyword == KW_FUNCTION) {
                    lex_shutdown(lex);
                    break; /* Finished function execution successfully */
                }
            }
        }
        lex_shutdown(lex);

        BppError exec_err = vm_execute_line(vm, active_lines[idx].text);
        if (exec_err.code != 0) {
            *err = exec_err;
            running = false;
            break;
        }

        if (active_lines[idx].line_number == def_line) {
            /* Discard the definition skip jump set by FUNCTION/SUB statement itself */
            vm_clear_header_jump(vm, def_line);
        }

        if (vm_handle_jump_active(vm, &curr_line)) {
            /* Jump handled */
        } else if (vm_get_current_line(vm) != curr_line) {
            /* Jump active inside function */
            curr_line = vm_get_current_line(vm);
        } else {
            if (idx + 1 < active_count) {
                curr_line = active_lines[idx + 1].line_number;
            } else {
                break;
            }
        }
    }

    /* 4. Read return value */
    BValue *ret_var = var_lookup(vm_get_var(vm), base_name, false);
    if (ret_var) {
        res = *ret_var;
        if (res.type == VAL_STRING && res.as.string) {
            str_add_ref(res.as.string);
        }
    }

    /* Copy-back fields from local scope variables to 'this' object map */
    if (is_method && method_this) {
        for (int i = 0; i < method_this->count; i++) {
            if (method_this->entries[i].key != NULL) {
                const char *key = method_this->entries[i].key;
                if (strcmp(key, "__type__") == 0) continue;
                
                BValue *local_var = var_lookup(vm_get_var(vm), key, false);
                if (local_var) {
                    BValue old_val = method_this->entries[i].val;
                    if (old_val.type == VAL_STRING && old_val.as.string) {
                        str_release(vm_get_str(vm), old_val.as.string);
                    } else if (old_val.type == VAL_MAP && old_val.as.map) {
                        bpp_map_release(vm_get_str(vm), old_val.as.map);
                    }
                    
                    method_this->entries[i].val = *local_var;
                    if (local_var->type == VAL_STRING && local_var->as.string) {
                        str_add_ref(local_var->as.string);
                    } else if (local_var->type == VAL_MAP && local_var->as.map) {
                        bpp_map_add_ref(local_var->as.map);
                    }
                }
            }
        }
    }

    /* 5. Pop Call Frame & restore parent scope */
    BppSubFrame dummy;
    vm_sub_pop(vm, &dummy);

    if (parent_scope[0] != '\0') {
        var_set_scope(vm_get_var(vm), parent_scope);
    } else {
        var_set_scope(vm_get_var(vm), NULL);
    }

    /* Restore parent execution pointer */
    vm_restore_state(vm, &saved_state);

    return res;
}

BppError vm_call_sub_procedure(VMContext *vm, const char *sub_name, BValue *args, int arg_count, const char *ret_pos) {
    BppError err;
    memset(&err, 0, sizeof(err));

    /* 1. Find SUB definition */
    BppLineNumber def_line = 0.0;
    const char *def_text = NULL;
    if (!find_procedure(vm, sub_name, KW_SUB, &def_line, &def_text)) {
        err.code = 35; err.message = "Subprogram not defined";
        return err;
    }

    /* 2. Parse parameter names from definition text */
    LexerContext *def_lex = lex_init(vm_get_mem(vm), def_text);
    /* Skip SUB MYPROC */
    lex_next(def_lex);
    lex_next(def_lex);

    char params[MAX_PARAMS][64];
    int param_count = 0;

    BppToken tok = lex_peek(def_lex);
    if (tok.type == TOK_LPAREN) {
        lex_next(def_lex);
        while (true) {
            tok = lex_next(def_lex);
            if (tok.type == TOK_IDENT) {
                if (param_count >= MAX_PARAMS) {
                    err.code = 2; err.message = "Too many parameters in SUB definition";
                    lex_shutdown(def_lex);
                    return err;
                }
                size_t plen = (tok.length < 63) ? tok.length : 63;
                memcpy(params[param_count], tok.start, plen);
                params[param_count][plen] = '\0';
                param_count++;

                tok = lex_peek(def_lex);
                if (tok.type == TOK_COMMA) {
                    lex_next(def_lex);
                } else if (tok.type == TOK_RPAREN) {
                    lex_next(def_lex);
                    break;
                } else {
                    err.code = 2; err.message = "Expected ',' or ')' in parameter list";
                    lex_shutdown(def_lex);
                    return err;
                }
            } else if (tok.type == TOK_RPAREN) {
                break;
            } else {
                err.code = 2; err.message = "Expected parameter name in SUB definition";
                lex_shutdown(def_lex);
                return err;
            }
        }
    }
    const char *first_stmt = lex_get_pos(def_lex);
    lex_shutdown(def_lex);

    if (arg_count != param_count) {
        err.code = 37; err.message = "Argument count mismatch";
        return err;
    }

    /* 3. Push Call Frame */
    if (!vm_sub_push(vm, sub_name, vm_get_current_line(vm), ret_pos, false)) {
        err.code = 7; err.message = "SUB stack overflow";
        return err;
    }

    /* 4. Bind variables in new scope */
    var_set_scope(vm_get_var(vm), sub_name);
    for (int i = 0; i < param_count; i++) {
        BValue *p_var = var_declare(vm_get_var(vm), params[i]);
        if (p_var) {
            *p_var = args[i];
            if (p_var->type == VAL_STRING && p_var->as.string) {
                str_add_ref(p_var->as.string);
            }
        }
    }

    /* 5. Jump execution to sub body first statement */
    vm_jump(vm, def_line, first_stmt);

    return err;
}
