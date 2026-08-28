// FILENAME: def.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (eval_expr_internal.h, exec_internal.h, sub_internal.h)
// NEEDS: libcore (ctype.h, ctype.c, micro_lib_metadata.h, micro_lib_metadata.c)
// NEEDS: libcore (string.h, strings.h, strings.c, variables.h, variables.c)
// NEEDS: libengine (def.h, eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h)
// NEEDS: libkernel (errors.h)
// Provides runtime implementation for the DEF statement in BASIC++.
//
// ---- Includes ----

#include "statements/variables/declaration/def.h"
#include "runtime/micro_lib_metadata.h"
#include "types/errors.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "runtime/variables.h"
#include "runtime/strings.h"
#include "eval/eval.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

#ifdef _WIN32
#define strcasecmp _stricmp
#endif

BppError stmt_defseg_handler(VMContext *vm, LexerContext *lex);

typedef struct DefFnEntry {
    char fn_name[64];
    char param_names[4][64];
    int param_count;
    bool is_multiline;
    BppLineNumber start_line;
    BppLineNumber end_line;
    char expr_body[256];
    struct DefFnEntry *next;
} DefFnEntry;

static DefFnEntry *g_def_fn_head = NULL;
static char g_active_def_fn[64] = {0};

void def_fn_register(const char *name, int param_count, char param_names[][64], const char *expr_body) {
    if (!name || !expr_body) return;

    DefFnEntry *curr = g_def_fn_head;
    while (curr) {
        if (strcasecmp(curr->fn_name, name) == 0) {
            curr->param_count = param_count;
            curr->is_multiline = false;
            for (int i = 0; i < param_count && i < 4; i++) {
                strncpy(curr->param_names[i], param_names[i], 63);
                curr->param_names[i][63] = '\0';
            }
            strncpy(curr->expr_body, expr_body, sizeof(curr->expr_body) - 1);
            curr->expr_body[sizeof(curr->expr_body) - 1] = '\0';
            return;
        }
        curr = curr->next;
    }

    DefFnEntry *node = (DefFnEntry *)calloc(1, sizeof(DefFnEntry));
    if (!node) return;
    strncpy(node->fn_name, name, sizeof(node->fn_name) - 1);
    node->param_count = param_count;
    node->is_multiline = false;
    for (int i = 0; i < param_count && i < 4; i++) {
        strncpy(node->param_names[i], param_names[i], 63);
        node->param_names[i][63] = '\0';
    }
    strncpy(node->expr_body, expr_body, sizeof(node->expr_body) - 1);
    node->expr_body[sizeof(node->expr_body) - 1] = '\0';

    node->next = g_def_fn_head;
    g_def_fn_head = node;
}

void def_fn_register_multiline(const char *name, int param_count, char param_names[][64], BppLineNumber start_line, BppLineNumber end_line) {
    if (!name) return;

    DefFnEntry *curr = g_def_fn_head;
    while (curr) {
        if (strcasecmp(curr->fn_name, name) == 0) {
            curr->param_count = param_count;
            curr->is_multiline = true;
            curr->start_line = start_line;
            curr->end_line = end_line;
            for (int i = 0; i < param_count && i < 4; i++) {
                strncpy(curr->param_names[i], param_names[i], 63);
                curr->param_names[i][63] = '\0';
            }
            curr->expr_body[0] = '\0';
            return;
        }
        curr = curr->next;
    }

    DefFnEntry *node = (DefFnEntry *)calloc(1, sizeof(DefFnEntry));
    if (!node) return;
    strncpy(node->fn_name, name, sizeof(node->fn_name) - 1);
    node->param_count = param_count;
    node->is_multiline = true;
    node->start_line = start_line;
    node->end_line = end_line;
    for (int i = 0; i < param_count && i < 4; i++) {
        strncpy(node->param_names[i], param_names[i], 63);
        node->param_names[i][63] = '\0';
    }
    node->expr_body[0] = '\0';

    node->next = g_def_fn_head;
    g_def_fn_head = node;
}

bool def_fn_exists(const char *name) {
    if (!name) return false;
    DefFnEntry *curr = g_def_fn_head;
    while (curr) {
        if (strcasecmp(curr->fn_name, name) == 0) return true;
        curr = curr->next;
    }
    return false;
}

BValue def_fn_eval(VMContext *vm, const char *name, BValue *args, int argc, BppError *err, bool *found) {
    BValue res;
    memset(&res, 0, sizeof(res));
    res.type = VAL_NUMBER;
    res.as.number = 0.0;
    if (found) *found = false;

    if (!name) return res;

    DefFnEntry *curr = g_def_fn_head;
    while (curr) {
        if (strcasecmp(curr->fn_name, name) == 0) {
            if (found) *found = true;
            VariableContext *vc = vm_get_var(vm);

            BValue old_vals[4];
            bool has_old[4] = {false};
            for (int i = 0; i < curr->param_count && i < 4; i++) {
                BValue *old_ptr = var_lookup(vc, curr->param_names[i], false);
                if (old_ptr) {
                    has_old[i] = true;
                    old_vals[i] = *old_ptr;
                    if (old_vals[i].type == VAL_STRING && old_vals[i].as.string) {
                        str_add_ref(old_vals[i].as.string);
                    }
                }
                if (i < argc) {
                    var_assign(vc, curr->param_names[i], args[i]);
                }
            }

            if (!curr->is_multiline) {
                LexerContext *fn_lex = lex_init(vm_get_mem(vm), curr->expr_body);
                BppError fn_err;
                memset(&fn_err, 0, sizeof(fn_err));
                res = eval_expression(vm, fn_lex, &fn_err);
                if (err) *err = fn_err;
                lex_shutdown(fn_lex);
            } else {
                // Multi-line execution
                char prev_active_fn[64];
                strncpy(prev_active_fn, g_active_def_fn, 63);
                prev_active_fn[63] = '\0';
                strncpy(g_active_def_fn, curr->fn_name, 63);
                g_active_def_fn[63] = '\0';

                BValue *old_fn_var = var_lookup(vc, curr->fn_name, false);
                BValue saved_fn_val = {0};
                bool has_old_fn = false;
                if (old_fn_var) {
                    has_old_fn = true;
                    saved_fn_val = *old_fn_var;
                    if (saved_fn_val.type == VAL_STRING && saved_fn_val.as.string) {
                        str_add_ref(saved_fn_val.as.string);
                    }
                }

                MemoryContext *mem = vm_get_mem(vm);
                size_t count = 0;
                BppProgramLine *lines = mem_program_get_all(mem, &count);
                for (size_t i = 0; i < count; i++) {
                    if (lines[i].line_number > curr->start_line && lines[i].line_number <= curr->end_line) {
                        if (lines[i].line_number == curr->end_line) {
                            LexerContext *end_lex = lex_init(mem, lines[i].text);
                            if (end_lex) {
                                BppToken end_tok = lex_next(end_lex);
                                if (end_tok.type == TOK_KEYWORD && end_tok.as.keyword == KW_FNEND) {
                                    BppToken nxt = lex_peek(end_lex);
                                    if (nxt.type != TOK_EOL && nxt.type != TOK_EOF) {
                                        BppError end_err;
                                        memset(&end_err, 0, sizeof(end_err));
                                        BValue end_ret = eval_expression(vm, end_lex, &end_err);
                                        if (end_err.code == 0) {
                                            var_assign(vc, curr->fn_name, end_ret);
                                            if (end_ret.type == VAL_STRING && end_ret.as.string) {
                                                str_release(vm_get_str(vm), end_ret.as.string);
                                            }
                                        }
                                    }
                                }
                                lex_shutdown(end_lex);
                            }
                            break;
                        }
                        BppError line_err = vm_execute_line(vm, lines[i].text);
                        if (line_err.code != 0) {
                            if (err) *err = line_err;
                            break;
                        }
                    }
                }

                BValue *res_ptr = var_lookup(vc, curr->fn_name, false);
                if (res_ptr) {
                    res = *res_ptr;
                    if (res.type == VAL_STRING && res.as.string) {
                        str_add_ref(res.as.string);
                    }
                }

                if (has_old_fn) {
                    var_assign(vc, curr->fn_name, saved_fn_val);
                    if (saved_fn_val.type == VAL_STRING && saved_fn_val.as.string) {
                        str_release(vm_get_str(vm), saved_fn_val.as.string);
                    }
                } else {
                    var_assign(vc, curr->fn_name, (BValue){.type = VAL_NUMBER, .as.number = 0.0});
                }
                strncpy(g_active_def_fn, prev_active_fn, 63);
            }

            for (int i = 0; i < curr->param_count && i < 4; i++) {
                if (has_old[i]) {
                    var_assign(vc, curr->param_names[i], old_vals[i]);
                    if (old_vals[i].type == VAL_STRING && old_vals[i].as.string) {
                        str_release(vm_get_str(vm), old_vals[i].as.string);
                    }
                } else {
                    var_assign(vc, curr->param_names[i], (BValue){.type = VAL_NUMBER, .as.number = 0.0});
                }
            }

            return res;
        }
        curr = curr->next;
    }

    return res;
}

BppError stmt_fnend_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    if (!vm || !lex) return err;

    BppToken tok = lex_peek(lex);
    if (tok.type != TOK_EOL && tok.type != TOK_EOF) {
        BValue val = eval_expression(vm, lex, &err);
        if (err.code == 0) {
            if (g_active_def_fn[0] != '\0') {
                var_assign(vm_get_var(vm), g_active_def_fn, val);
            }
            if (val.type == VAL_STRING && val.as.string) {
                str_release(vm_get_str(vm), val.as.string);
            }
        }
    }
    return err;
}

BppError stmt_def_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_DEF) {
        lex_next(lex);
        tok = lex_peek(lex);
    }

    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_USR) {
        return stmt_defusr_handler(vm, lex);
    }

    char fn_name[64] = {0};
    if (tok.type == TOK_IDENT) {
        tok = lex_next(lex);
        size_t len = (tok.length < sizeof(fn_name) - 1) ? tok.length : sizeof(fn_name) - 1;
        memcpy(fn_name, tok.start, len);
        fn_name[len] = '\0';
        if (strcasecmp(fn_name, "SEG") == 0) {
            return stmt_defseg_handler(vm, lex);
        }
        if (strcasecmp(fn_name, "FN") == 0 && lex_peek(lex).type == TOK_IDENT) {
            BppToken next_ident = lex_next(lex);
            size_t nlen = (next_ident.length < 50) ? next_ident.length : 50;
            memcpy(fn_name + 2, next_ident.start, nlen);
            fn_name[2 + nlen] = '\0';
        }
    } else {
        err.code = 2; err.message = "Syntax error in DEF statement";
        return err;
    }

    char param_names[4][64] = {{0}};
    int param_count = 0;

    tok = lex_peek(lex);
    if (tok.type == TOK_LPAREN) {
        lex_next(lex);
        while (true) {
            BppToken ptok = lex_next(lex);
            if (ptok.type != TOK_IDENT) {
                err.code = 2; err.message = "Expected parameter name in DEF FN";
                return err;
            }
            if (param_count < 4) {
                size_t plen = (ptok.length < 63) ? ptok.length : 63;
                memcpy(param_names[param_count], ptok.start, plen);
                param_names[param_count][plen] = '\0';
                param_count++;
            }
            BppToken sep = lex_peek(lex);
            if (sep.type == TOK_COMMA) {
                lex_next(lex);
            } else if (sep.type == TOK_RPAREN) {
                lex_next(lex);
                break;
            } else {
                err.code = 2; err.message = "Expected ',' or ')' in parameter list";
                return err;
            }
        }
    }

    tok = lex_peek(lex);
    if (tok.type == TOK_EQ) {
        lex_next(lex); // Consume '='
        const char *expr_start = lex_get_pos(lex);
        char expr_body[256] = {0};
        size_t elen = 0;
        while (*expr_start && *expr_start != '\n' && *expr_start != '\r' && *expr_start != ':' && elen < sizeof(expr_body) - 1) {
            expr_body[elen++] = *expr_start++;
        }
        expr_body[elen] = '\0';

        def_fn_register(fn_name, param_count, param_names, expr_body);

        while (lex_peek(lex).type != TOK_EOL && lex_peek(lex).type != TOK_EOF) {
            lex_next(lex);
        }
        return err;
    }

    // Multi-line DEF FN ... FNEND
    MemoryContext *mem = vm_get_mem(vm);
    size_t count = 0;
    BppProgramLine *lines = mem_program_get_all(mem, &count);
    BppLineNumber cur_line = vm_get_current_line(vm);
    size_t fn_end_idx = count;

    for (size_t i = 0; i < count; i++) {
        if (lines[i].line_number == cur_line) {
            for (size_t j = i + 1; j < count; j++) {
                LexerContext *scan = lex_init(mem, lines[j].text);
                if (scan) {
                    BppToken stok = lex_next(scan);
                    if (stok.type == TOK_KEYWORD && stok.as.keyword == KW_FNEND) {
                        fn_end_idx = j;
                        lex_shutdown(scan);
                        break;
                    } else if (stok.type == TOK_KEYWORD && stok.as.keyword == KW_END) {
                        BppToken next_stok = lex_peek(scan);
                        if (next_stok.type == TOK_KEYWORD && next_stok.as.keyword == KW_DEF) {
                            fn_end_idx = j;
                            lex_shutdown(scan);
                            break;
                        }
                    }
                    lex_shutdown(scan);
                }
            }
            break;
        }
    }

    BppLineNumber end_line = (fn_end_idx < count) ? lines[fn_end_idx].line_number : cur_line;
    def_fn_register_multiline(fn_name, param_count, param_names, cur_line, end_line);

    // Skip executing function body during declaration pass
    if (fn_end_idx + 1 < count) {
        vm_jump(vm, lines[fn_end_idx + 1].line_number, lines[fn_end_idx + 1].text);
    } else {
        vm_jump(vm, end_line + 1, "");
    }

    while (lex_peek(lex).type != TOK_EOL && lex_peek(lex).type != TOK_EOF) {
        lex_next(lex);
    }
    return err;
}

static BppError stmt_def_range_helper(VMContext *vm, LexerContext *lex, ValueType type) {
    BppError err;
    memset(&err, 0, sizeof(err));
    VariableContext *vc = vm_get_var(vm);

    while (true) {
        BppToken tok = lex_next(lex);
        if (tok.type == TOK_EOL || tok.type == TOK_EOF) {
            break;
        }
        if (tok.type != TOK_IDENT) {
            err.code = 2; err.message = "Expected letter in DEF type range";
            return err;
        }

        char start_letter = (char)toupper((unsigned char)tok.start[0]);
        char end_letter = start_letter;

        BppToken next = lex_peek(lex);
        if (next.type == TOK_MINUS) {
            lex_next(lex); // Consume '-'
            BppToken end_tok = lex_next(lex);
            if (end_tok.type != TOK_IDENT) {
                err.code = 2; err.message = "Expected end letter in DEF range";
                return err;
            }
            end_letter = (char)toupper((unsigned char)end_tok.start[0]);
        }

        var_set_def_type(vc, NULL, start_letter, end_letter, type);

        next = lex_peek(lex);
        if (next.type == TOK_COMMA) {
            lex_next(lex); // Consume ','
        } else {
            break;
        }
    }
    return err;
}

BppError stmt_defcpx_handler(VMContext *vm, LexerContext *lex) {
    return stmt_def_range_helper(vm, lex, VAL_COMPLEX);
}

BppError stmt_defusr_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    (void)vm; (void)lex;
    return err;
}

void stmt_def_register(void) {
    MicroLibMetadata meta = {
        .name = "DEF",
        .category = "Function Definitions",
        .syntax = "DEF FNname[(args)] = expr",
        .help_text = "Defines a user-defined numeric or string function.",
        .error_codes = "Error 2: Syntax Error, Error 13: Type Mismatch"
    };
    microlib_register(&meta);
}

