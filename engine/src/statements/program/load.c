// FILENAME: load.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (ctype.h, ctype.c, memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c, variables.h, variables.c)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, load.h, string.c, vm.h)
// NEEDS: libkernel (errors.h, security.h, security.c, vdev.h, vdev.c)
// Provides runtime implementation for the LOAD statement in BASIC++.
//
// ---- Includes ----

#include "statements/program/load.h"
#include "types/errors.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "memory/memory.h"
#include "runtime/variables.h"
#include "runtime/strings.h"
#include "runtime/arrays.h"
#include "device/vdev.h"
#include "security/security.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/format/snprintf.h"
#include "runtime/memory/alloc.h"
#include "runtime/ctype/ctype.h"
#include "runtime/conv/float_parse.h"
#include "platform/platform.h"

static const LangDesc g_load_desc = {
    .name = "LOAD",
    .category = "Program Mgmt & Editing",
    .syntax = "LOAD filename_expr [, R]",
    .description = "Loads a program file into memory from disk storage, optionally running it.",
    .error_summary = "Error 2: Syntax Error, Error 53: File Not Found, Error 70: Permission Denied",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_STATEMENT
};

BppError stmt_load_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    if (!vm || !lex) {
        err.code = ERR_ILLEGAL_FUNCTION_CALL;
        return err;
    }

    BValue fn_val = eval_expression(vm, lex, &err);
    if (err.code != 0) {
        return err;
    }
    if (fn_val.type != VAL_STRING || !fn_val.as.string) {
        if (fn_val.type == VAL_STRING && fn_val.as.string) {
            str_release(vm_get_str(vm), fn_val.as.string);
        }
        err.code = ERR_TYPE_MISMATCH;
        return err;
    }

    const char *filename = str_data(fn_val.as.string);
    bool run_after = false;
    bool renum_mode = false;

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_COMMA) {
        lex_next(lex);
        tok = lex_peek(lex);
        if (tok.type == TOK_IDENT && tok.length == 1 && (tok.start[0] == 'R' || tok.start[0] == 'r')) {
            lex_next(lex);
            run_after = true;
        } else if ((tok.type == TOK_KEYWORD && tok.as.keyword == KW_RENUM) ||
                   (tok.type == TOK_IDENT && runtime_strncasecmp(tok.start, "RENUM", 5) == 0)) {
            lex_next(lex);
            renum_mode = true;
        }
    }

    mem_program_clear(vm_get_mem(vm));
    if (!run_after) {
        var_clear_all(vm_get_var(vm));
        arr_clear_all(vm_get_arr(vm));
    }
    vm_reset_for_run(vm);

    BppError load_err = vm_load_program_file_ex(vm, filename, renum_mode);
    str_release(vm_get_str(vm), fn_val.as.string);
    if (load_err.code != 0) {
        return load_err;
    }

    if (run_after) {
        vm_run_program(vm);
    }
    return err;
}

static void vm_process_include_line(VMContext *vm, BppLineNumber base_line, const char *line) {
    const char *p = runtime_strstr(line, "$INCLUDE");
    if (!p) return;
    p += 8;
    while (*p == ':' || runtime_isspace((unsigned char)*p)) p++;
    char quote = *p;
    if (quote != '\'' && quote != '"') return;
    p++;
    char inc_path[256];
    size_t k = 0;
    while (*p && *p != quote && k < sizeof(inc_path) - 1) {
        inc_path[k++] = *p++;
    }
    inc_path[k] = '\0';
    if (k > 0) {
        void *ifp = platform_file_open(inc_path, "r");
        if (ifp) {
            char inc_line[1024];
            int inc_idx = 1;
            while (platform_file_gets(inc_line, sizeof(inc_line), ifp)) {
                size_t ilen = runtime_strlen(inc_line);
                while (ilen > 0 && (inc_line[ilen - 1] == '\r' || inc_line[ilen - 1] == '\n')) {
                    inc_line[--ilen] = '\0';
                }
                char *ip = inc_line;
                while (runtime_isspace((unsigned char)*ip)) ip++;
                if (*ip != '\0' && *ip != '\'') {
                    if (runtime_isdigit((unsigned char)*ip)) {
                        BppLineNumber lnum = (BppLineNumber)runtime_strtod(ip, NULL);
                        while (runtime_isdigit((unsigned char)*ip) || *ip == '.') ip++;
                        while (runtime_isspace((unsigned char)*ip)) ip++;
                        if (*ip != '\0') mem_program_store(vm_get_mem(vm), lnum, ip);
                    } else {
                        BppLineNumber sub_line = (base_line > 0.0) ? (base_line + (double)inc_idx * 0.0001) : (0.0001 * (double)inc_idx);
                        mem_program_store(vm_get_mem(vm), sub_line, ip);
                        inc_idx++;
                    }
                }
            }
            platform_file_close(ifp);
        }
    }
}

BppError vm_load_program_file_ex(VMContext *vm, const char *filename, bool auto_renum) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    if (!vm || !filename) {
        err.code = ERR_ILLEGAL_FUNCTION_CALL;
        return err;
    }
    vm_set_current_filename(vm, filename);
    void *fp = platform_file_open(filename, "r");
    if (!fp) {
        err.code = ERR_FILE_NOT_FOUND;
        return err;
    }
    char line_buf[1024];
    BppLineNumber auto_num = 10.0;
    while (platform_file_gets(line_buf, sizeof(line_buf), fp)) {
        size_t len = runtime_strlen(line_buf);
        while (len > 0 && (line_buf[len - 1] == '\r' || line_buf[len - 1] == '\n')) {
            line_buf[--len] = '\0';
        }
        char *p = line_buf;
        while (runtime_isspace((unsigned char)*p)) p++;
        if (*p != '\0') {
            BppLineNumber line_num = 0.0;
            if (runtime_isdigit((unsigned char)*p)) {
                line_num = (BppLineNumber)runtime_strtod(p, NULL);
            }
            if (runtime_strstr(p, "$INCLUDE") != NULL) {
                vm_process_include_line(vm, line_num, p);
            }
            if (auto_renum) {
                if (runtime_isdigit((unsigned char)*p)) {
                    while (runtime_isdigit((unsigned char)*p) || *p == '.') p++;
                    while (runtime_isspace((unsigned char)*p)) p++;
                }
                if (*p != '\0') {
                    mem_program_store(vm_get_mem(vm), auto_num, p);
                    auto_num += 10.0;
                }
            } else if (runtime_isdigit((unsigned char)*p)) {
                while (runtime_isdigit((unsigned char)*p) || *p == '.') p++;
                while (runtime_isspace((unsigned char)*p)) p++;
                if (*p == '\0') {
                    mem_program_remove(vm_get_mem(vm), line_num);
                } else {
                    mem_program_store(vm_get_mem(vm), line_num, p);
                }
            } else {
                vm_execute_line(vm, p);
            }
        }
    }
    platform_file_close(fp);
    return err;
}

BppError vm_load_program_file(VMContext *vm, const char *filename) {
    return vm_load_program_file_ex(vm, filename, false);
}

void stmt_load_register(void) {
    lang_desc_register(&g_load_desc);
}

