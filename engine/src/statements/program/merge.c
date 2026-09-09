// FILENAME: merge.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (ctype.h, ctype.c, memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c, variables.h, variables.c)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, merge.h, string.c, vm.h)
// NEEDS: libkernel (errors.h, security.h, security.c, vdev.h, vdev.c)
// Provides runtime implementation for the MERGE statement in BASIC++.
//
// ---- Includes ----

#include "statements/program/merge.h"
#include "types/errors.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "memory/memory.h"
#include "runtime/variables.h"
#include "runtime/strings.h"
#include "device/vdev.h"
#include "security/security.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/format/snprintf.h"
#include "runtime/memory/alloc.h"
#include "runtime/ctype/ctype.h"
#include "platform/platform.h"

static const LangDesc g_merge_desc = {
    .name = "MERGE",
    .category = "Program Mgmt & Editing",
    .syntax = "MERGE filename_expr",
    .description = "Merges specified ASCII BASIC source file into current memory program without clearing existing lines.",
    .error_summary = "Error 53: File Not Found, Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_STATEMENT
};

BppError stmt_merge_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    if (!vm || !lex) {
        err.code = ERR_ILLEGAL_FUNCTION_CALL;
        return err;
    }

    BValue val = eval_expression(vm, lex, &err);
    if (err.code != 0) {
        return err;
    }
    if (val.type != VAL_STRING || !val.as.string) {
        if (val.type == VAL_STRING && val.as.string) {
            str_release(vm_get_str(vm), val.as.string);
        }
        err.code = ERR_TYPE_MISMATCH;
        return err;
    }

    const char *filename = str_data(val.as.string);
    void *fp = platform_file_open(filename, "r");
    if (!fp) {
        str_release(vm_get_str(vm), val.as.string);
        err.code = ERR_FILE_NOT_FOUND;
        return err;
    }

    char line_buf[1024];
    while (platform_file_gets(line_buf, sizeof(line_buf), fp)) {
        size_t len = runtime_strlen(line_buf);
        while (len > 0 && (line_buf[len - 1] == '\r' || line_buf[len - 1] == '\n')) {
            line_buf[--len] = '\0';
        }
        char *p = line_buf;
        while (runtime_isspace((unsigned char)*p)) p++;
        if (*p != '\0') {
            if (runtime_isdigit((unsigned char)*p)) {
                BppLineNumber line_num = 0;
                while (runtime_isdigit((unsigned char)*p)) {
                    line_num = line_num * 10 + (*p - '0');
                    p++;
                }
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
    str_release(vm_get_str(vm), val.as.string);
    return err;
}

void stmt_merge_register(void) {
    lang_desc_register(&g_merge_desc);
}
