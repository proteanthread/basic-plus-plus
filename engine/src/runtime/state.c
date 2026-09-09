// FILENAME: state.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (arrays.h, arrays.c, memory.h, memory.c, state.h, string.h)
// NEEDS: libcore (variables.h, variables.c)
// NEEDS: libengine (string.c)
// Provides core logic and interface definitions for state within BASIC++.
//
// ---- Includes ----

#include "runtime/state.h"
#include "runtime/variables.h"
#include "runtime/arrays.h"
#include "memory/memory.h"
#include "platform/platform.h"
#include "runtime/format/snprintf.h"
#include "runtime/memory/alloc.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

BppError vm_state_save(VMContext *vm, const char *filename) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    void *fp = platform_file_open(filename, "wb");
    if (!fp) {
        err.code = 58;
        err.message = "Failed to open state file for writing";
        return err;
    }

    // 1. Header
    platform_file_write(fp, "BPPSTATE", 8);
    uint32_t version = 100; // 1.00
    platform_file_write(fp, &version, sizeof(version));

    extern BppLineNumber vm_get_current_line(VMContext *vm);
    BppLineNumber cur_line = vm_get_current_line(vm);
    platform_file_write(fp, &cur_line, sizeof(cur_line));

    // 2. Program Lines
    size_t count = 0;
    BppProgramLine *lines = mem_program_get_all(vm_get_mem(vm), &count);
    uint32_t line_count = (uint32_t)count;
    platform_file_write(fp, &line_count, sizeof(line_count));

    for (uint32_t i = 0; i < line_count; i++) {
        platform_file_write(fp, &lines[i].line_number, sizeof(BppLineNumber));
        uint32_t text_len = lines[i].text ? (uint32_t)runtime_strlen(lines[i].text) : 0;
        platform_file_write(fp, &text_len, sizeof(text_len));
        if (text_len > 0) {
            platform_file_write(fp, lines[i].text, text_len);
        }
    }

    // 3. Variables
    if (!var_serialize(vm_get_var(vm), fp)) {
        err.code = 58;
        err.message = "Failed to serialize variables";
        platform_file_close(fp);
        return err;
    }

    // 4. Arrays
    if (!arr_serialize(vm_get_arr(vm), fp)) {
        err.code = 58;
        err.message = "Failed to serialize arrays";
        platform_file_close(fp);
        return err;
    }

    platform_file_close(fp);
    return err;
}

BppError vm_state_load(VMContext *vm, const char *filename) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    void *fp = platform_file_open(filename, "rb");
    if (!fp) {
        err.code = 53;
        err.message = "Failed to open state file for reading";
        return err;
    }

    // 1. Header
    char magic[8];
    if (platform_file_read(fp, magic, 8) != 8 || runtime_memcmp(magic, "BPPSTATE", 8) != 0) {
        err.code = 53;
        err.message = "Invalid state file format";
        platform_file_close(fp);
        return err;
    }

    uint32_t version = 0;
    if (platform_file_read(fp, &version, sizeof(version)) != sizeof(version) || version != 100) {
        err.code = 53;
        err.message = "Incompatible state file version";
        platform_file_close(fp);
        return err;
    }

    BppLineNumber cur_line = 0;
    if (platform_file_read(fp, &cur_line, sizeof(cur_line)) != sizeof(cur_line)) {
        err.code = 53;
        err.message = "Corrupted state file (line number)";
        platform_file_close(fp);
        return err;
    }
    extern void vm_set_current_line(VMContext *vm, BppLineNumber line);
    vm_set_current_line(vm, cur_line);

    // 2. Program Lines
    mem_program_clear(vm_get_mem(vm));
    uint32_t line_count = 0;
    if (platform_file_read(fp, &line_count, sizeof(line_count)) != sizeof(line_count)) {
        err.code = 53;
        err.message = "Corrupted state file (line count)";
        platform_file_close(fp);
        return err;
    }

    for (uint32_t i = 0; i < line_count; i++) {
        BppLineNumber line_num = 0;
        if (platform_file_read(fp, &line_num, sizeof(line_num)) != sizeof(line_num)) {
            err.code = 53;
            err.message = "Corrupted state file (line number entry)";
            platform_file_close(fp);
            return err;
        }

        uint32_t text_len = 0;
        if (platform_file_read(fp, &text_len, sizeof(text_len)) != sizeof(text_len)) {
            err.code = 53;
            err.message = "Corrupted state file (text length)";
            platform_file_close(fp);
            return err;
        }

        char *text = NULL;
        if (text_len > 0) {
            text = (char *)runtime_calloc(1, text_len + 1);
            if (!text) {
                err.code = 53;
                err.message = "Out of memory loading state program lines";
                platform_file_close(fp);
                return err;
            }
            if (platform_file_read(fp, text, text_len) != text_len) {
                runtime_free(text);
                err.code = 53;
                err.message = "Corrupted state file (text data)";
                platform_file_close(fp);
                return err;
            }
            text[text_len] = '\0';
        }

        mem_program_insert(vm_get_mem(vm), line_num, text ? text : "");
        if (text) runtime_free(text);
    }

    // 3. Variables
    if (!var_deserialize(vm_get_var(vm), fp)) {
        err.code = 53;
        err.message = "Failed to deserialize variables";
        platform_file_close(fp);
        return err;
    }

    // 4. Arrays
    if (!arr_deserialize(vm_get_arr(vm), fp)) {
        err.code = 53;
        err.message = "Failed to deserialize arrays";
        platform_file_close(fp);
        return err;
    }

    platform_file_close(fp);
    return err;
}

char *vm_state_info(VMContext *vm, const char *filename, BppError *err) {
    (void)vm;
    if (err) runtime_memset(err, 0, sizeof(*err));

    void *fp = platform_file_open(filename, "rb");
    if (!fp) {
        if (err) { err->code = 53; err->message = "Failed to open state file"; }
        return NULL;
    }

    char magic[8];
    if (platform_file_read(fp, magic, 8) != 8 || runtime_memcmp(magic, "BPPSTATE", 8) != 0) {
        if (err) { err->code = 53; err->message = "Invalid magic bytes"; }
        platform_file_close(fp);
        return NULL;
    }

    uint32_t version = 0;
    if (platform_file_read(fp, &version, sizeof(version)) != sizeof(version)) {
        if (err) { err->code = 53; err->message = "Failed to read version"; }
        platform_file_close(fp);
        return NULL;
    }

    BppLineNumber cur_line = 0;
    if (platform_file_read(fp, &cur_line, sizeof(cur_line)) != sizeof(cur_line)) {
        if (err) { err->code = 53; err->message = "Failed to read current line"; }
        platform_file_close(fp);
        return NULL;
    }

    uint32_t line_count = 0;
    if (platform_file_read(fp, &line_count, sizeof(line_count)) != sizeof(line_count)) {
        if (err) { err->code = 53; err->message = "Failed to read line count"; }
        platform_file_close(fp);
        return NULL;
    }

    platform_file_close(fp);

    char info_buf[256];
    runtime_snprintf(info_buf, sizeof(info_buf), "Format: BPPSTATE, Version: %lld, Current Line: %lld, Program Lines: %u",
             (long long)version, (long long)cur_line, line_count);

    size_t len = runtime_strlen(info_buf);
    char *res = (char *)runtime_calloc(1, len + 1);
    if (res) {
        runtime_memcpy(res, info_buf, len + 1);
    }
    return res;
}
