// FILENAME: append.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (ctype.h, ctype.c, file.h, file.c, memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (append.h, eval.h, eval.c, string.c, vm.h)
// NEEDS: libkernel (errors.h)
// Provides runtime implementation for the APPEND statement in BASIC++.
//
// ---- Includes ----

#include "statements/filesystem/file_ops/append.h"
#include "eval/eval.h"
#include "runtime/file.h"
#include "runtime/language_descriptor.h"
#include "types/errors.h"
#include "vm/vm.h"
#include "memory/memory.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/format/snprintf.h"
#include "runtime/memory/alloc.h"
#include "runtime/ctype/ctype.h"
#include <stdbool.h>
#include "runtime/conv/float_parse.h"
#include "platform/platform.h"

static const LangDesc g_append_desc = {
    .name = "APPEND",
    .category = "Filesystem I/O",
    .syntax = "APPEND [#]channel, \"filespec$\"",
    .description = "Opens file channel in append mode, or streams/appends program lines in memory to the end/merged positions of the disk file.",
    .error_summary = "Error 2: Syntax Error, Error 52: Bad File Number, Error 53: File Not Found, Error 55: File Already Open, Error 70: Permission Denied",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_STATEMENT
};

void stmt_append_register(void) {
    lang_desc_register(&g_append_desc);
}

BppError stmt_append_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_APPEND) {
        lex_next(lex);
        tok = lex_peek(lex);
    }

    if (tok.type == TOK_HASH) {
        // 1. File channel append: APPEND #channel, "filespec"
        lex_next(lex);
        BValue ch_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (ch_val.type != VAL_NUMBER && ch_val.type != VAL_INTEGER) {
            if (ch_val.type == VAL_STRING && ch_val.as.string) str_release(vm_get_str(vm), ch_val.as.string);
            err.code = ERR_TYPE_MISMATCH;
            return err;
        }
        int channel = (int)ch_val.as.number;

        BppToken comma = lex_next(lex);
        if (comma.type != TOK_COMMA) {
            err.code = ERR_SYNTAX;
            err.message = "Expected ',' after channel number in APPEND";
            return err;
        }

        BValue fn_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (fn_val.type != VAL_STRING || !fn_val.as.string) {
            if (fn_val.type == VAL_STRING && fn_val.as.string) str_release(vm_get_str(vm), fn_val.as.string);
            err.code = ERR_TYPE_MISMATCH;
            err.message = "Expected string filename in APPEND";
            return err;
        }

        err = file_open(vm_get_file(vm), vm_get_vdev(vm), channel, str_data(fn_val.as.string),
                        FILE_MODE_APPEND, FILE_ACCESS_WRITE, FILE_LOCK_DEFAULT, 128);
        str_release(vm_get_str(vm), fn_val.as.string);
        return err;
    }

    // 2. Program Disk Output: APPEND "filename" (output memory lines to disk file)
    BValue val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (val.type != VAL_STRING || !val.as.string) {
        if (val.type == VAL_STRING && val.as.string) str_release(vm_get_str(vm), val.as.string);
        err.code = ERR_TYPE_MISMATCH;
        return err;
    }

    const char *raw_name = str_data(val.as.string);
    char filename[260];
    runtime_snprintf(filename, sizeof(filename), "%s", raw_name);

    size_t mem_count = 0;
    BppProgramLine *mem_lines = mem_program_get_all(vm_get_mem(vm), &mem_count);

    // Check if target file already exists
    void *in_fp = platform_file_open(filename, "r");
    if (!in_fp && !runtime_strstr(filename, ".bas") && !runtime_strstr(filename, ".BAS")) {
        char alt_name[260];
        runtime_snprintf(alt_name, sizeof(alt_name), "%s.bas", filename);
        in_fp = platform_file_open(alt_name, "r");
        if (in_fp) {
            runtime_snprintf(filename, sizeof(filename), "%s", alt_name);
        }
    }

    if (!in_fp) {
        // File does not exist: create new file directly with memory lines
        void *out_fp = platform_file_open(filename, "w");
        if (!out_fp) {
            str_release(vm_get_str(vm), val.as.string);
            err.code = ERR_PERMISSION_DENIED;
            return err;
        }
        for (size_t i = 0; i < mem_count; i++) {
            platform_file_printf(out_fp, "%lld %s\n", (long long)mem_lines[i].line_number,
                    mem_lines[i].text ? mem_lines[i].text : "");
        }
        platform_file_close(out_fp);
        str_release(vm_get_str(vm), val.as.string);
        return err;
    }

    // File exists: Stream line-by-line into temporary file
    char tmp_filename[300];
    runtime_snprintf(tmp_filename, sizeof(tmp_filename), "%s.tmp", filename);
    void *out_fp = platform_file_open(tmp_filename, "w");
    if (!out_fp) {
        platform_file_close(in_fp);
        str_release(vm_get_str(vm), val.as.string);
        err.code = ERR_PERMISSION_DENIED;
        return err;
    }

    // Track which memory lines have been emitted
    bool *mem_emitted = (bool *)runtime_calloc(mem_count > 0 ? mem_count : 1, sizeof(bool));

    char line_buf[2048];
    while (platform_file_gets(line_buf, sizeof(line_buf), in_fp)) {
        // Trim trailing newlines
        size_t len = runtime_strlen(line_buf);
        while (len > 0 && (line_buf[len - 1] == '\r' || line_buf[len - 1] == '\n')) {
            line_buf[--len] = '\0';
        }
        char *p = line_buf;
        while (runtime_isspace((unsigned char)*p)) p++;

        if (runtime_isdigit((unsigned char)*p)) {
            BppLineNumber disk_line_num = (BppLineNumber)runtime_atof(p);

            // Emit any memory lines that come strictly BEFORE disk_line_num
            for (size_t i = 0; i < mem_count; i++) {
                if (!mem_emitted[i] && mem_lines[i].line_number < disk_line_num) {
                    platform_file_printf(out_fp, "%lld %s\n", (long long)mem_lines[i].line_number,
                            mem_lines[i].text ? mem_lines[i].text : "");
                    mem_emitted[i] = true;
                }
            }

            // Check if memory has a line with the EXACT same line number
            bool duplicate_found = false;
            for (size_t i = 0; i < mem_count; i++) {
                if (mem_lines[i].line_number == disk_line_num) {
                    // Memory line takes precedence, replacing disk duplicate
                    if (!mem_emitted[i]) {
                        platform_file_printf(out_fp, "%lld %s\n", (long long)mem_lines[i].line_number,
                                mem_lines[i].text ? mem_lines[i].text : "");
                        mem_emitted[i] = true;
                    }
                    duplicate_found = true;
                    break;
                }
            }

            if (!duplicate_found) {
                // Write original disk line
                platform_file_printf(out_fp, "%s\n", line_buf);
            }
        } else {
            // Non-numbered line on disk: write through
            platform_file_printf(out_fp, "%s\n", line_buf);
        }
    }

    // Emit any remaining memory lines
    for (size_t i = 0; i < mem_count; i++) {
        if (!mem_emitted[i]) {
            platform_file_printf(out_fp, "%lld %s\n", (long long)mem_lines[i].line_number,
                    mem_lines[i].text ? mem_lines[i].text : "");
            mem_emitted[i] = true;
        }
    }

    runtime_free(mem_emitted);
    platform_file_close(in_fp);
    platform_file_close(out_fp);

    // Atomically replace original file with temp file
    platform_remove(filename);
    platform_rename(tmp_filename, filename);

    str_release(vm_get_str(vm), val.as.string);
    return err;
}
