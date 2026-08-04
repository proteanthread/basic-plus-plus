/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file state.c
 * @brief Session State Save/Load implementation.
 */

#include "runtime/state.h"
#include "runtime/variables.h"
#include "runtime/arrays.h"
#include "memory/memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

BppError vm_state_save(VMContext *vm, const char *filename) {
    BppError err;
    memset(&err, 0, sizeof(err));

    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        err.code = 58;
        err.message = "Failed to open state file for writing";
        return err;
    }

    /* 1. Header */
    fwrite("BPPSTATE", 1, 8, fp);
    uint32_t version = 100; /* 1.00 */
    fwrite(&version, sizeof(version), 1, fp);

    extern BppLineNumber vm_get_current_line(VMContext *vm);
    BppLineNumber cur_line = vm_get_current_line(vm);
    fwrite(&cur_line, sizeof(cur_line), 1, fp);

    /* 2. Program Lines */
    size_t count = 0;
    BppProgramLine *lines = mem_program_get_all(vm_get_mem(vm), &count);
    uint32_t line_count = (uint32_t)count;
    fwrite(&line_count, sizeof(line_count), 1, fp);

    for (uint32_t i = 0; i < line_count; i++) {
        fwrite(&lines[i].line_number, sizeof(BppLineNumber), 1, fp);
        uint32_t text_len = lines[i].text ? (uint32_t)strlen(lines[i].text) : 0;
        fwrite(&text_len, sizeof(text_len), 1, fp);
        if (text_len > 0) {
            fwrite(lines[i].text, 1, text_len, fp);
        }
    }

    /* 3. Variables */
    if (!var_serialize(vm_get_var(vm), fp)) {
        err.code = 58;
        err.message = "Failed to serialize variables";
        fclose(fp);
        return err;
    }

    /* 4. Arrays */
    if (!arr_serialize(vm_get_arr(vm), fp)) {
        err.code = 58;
        err.message = "Failed to serialize arrays";
        fclose(fp);
        return err;
    }

    fclose(fp);
    return err;
}

BppError vm_state_load(VMContext *vm, const char *filename) {
    BppError err;
    memset(&err, 0, sizeof(err));

    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        err.code = 53;
        err.message = "Failed to open state file for reading";
        return err;
    }

    /* 1. Header */
    char magic[8];
    if (fread(magic, 1, 8, fp) != 8 || memcmp(magic, "BPPSTATE", 8) != 0) {
        err.code = 53;
        err.message = "Invalid state file format";
        fclose(fp);
        return err;
    }

    uint32_t version = 0;
    if (fread(&version, sizeof(version), 1, fp) != 1 || version != 100) {
        err.code = 53;
        err.message = "Incompatible state file version";
        fclose(fp);
        return err;
    }

    BppLineNumber cur_line = 0;
    if (fread(&cur_line, sizeof(cur_line), 1, fp) != 1) {
        err.code = 53;
        err.message = "Corrupted state file (line number)";
        fclose(fp);
        return err;
    }
    extern void vm_set_current_line(VMContext *vm, BppLineNumber line);
    vm_set_current_line(vm, cur_line);

    /* 2. Program Lines */
    mem_program_clear(vm_get_mem(vm));
    uint32_t line_count = 0;
    if (fread(&line_count, sizeof(line_count), 1, fp) != 1) {
        err.code = 53;
        err.message = "Corrupted state file (line count)";
        fclose(fp);
        return err;
    }

    for (uint32_t i = 0; i < line_count; i++) {
        BppLineNumber line_num = 0;
        if (fread(&line_num, sizeof(line_num), 1, fp) != 1) {
            err.code = 53;
            err.message = "Corrupted state file (line number entry)";
            fclose(fp);
            return err;
        }

        uint32_t text_len = 0;
        if (fread(&text_len, sizeof(text_len), 1, fp) != 1) {
            err.code = 53;
            err.message = "Corrupted state file (text length)";
            fclose(fp);
            return err;
        }

        char *text = NULL;
        if (text_len > 0) {
            text = (char *)calloc(1, text_len + 1);
            if (!text) {
                err.code = 53;
                err.message = "Out of memory loading state program lines";
                fclose(fp);
                return err;
            }
            if (fread(text, 1, text_len, fp) != text_len) {
                free(text);
                err.code = 53;
                err.message = "Corrupted state file (text data)";
                fclose(fp);
                return err;
            }
            text[text_len] = '\0';
        }

        mem_program_insert(vm_get_mem(vm), line_num, text ? text : "");
        if (text) free(text);
    }

    /* 3. Variables */
    if (!var_deserialize(vm_get_var(vm), fp)) {
        err.code = 53;
        err.message = "Failed to deserialize variables";
        fclose(fp);
        return err;
    }

    /* 4. Arrays */
    if (!arr_deserialize(vm_get_arr(vm), fp)) {
        err.code = 53;
        err.message = "Failed to deserialize arrays";
        fclose(fp);
        return err;
    }

    fclose(fp);
    return err;
}

char *vm_state_info(VMContext *vm, const char *filename, BppError *err) {
    (void)vm;
    if (err) memset(err, 0, sizeof(*err));

    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        if (err) { err->code = 53; err->message = "Failed to open state file"; }
        return NULL;
    }

    char magic[8];
    if (fread(magic, 1, 8, fp) != 8 || memcmp(magic, "BPPSTATE", 8) != 0) {
        if (err) { err->code = 53; err->message = "Invalid magic bytes"; }
        fclose(fp);
        return NULL;
    }

    uint32_t version = 0;
    if (fread(&version, sizeof(version), 1, fp) != 1) {
        if (err) { err->code = 53; err->message = "Failed to read version"; }
        fclose(fp);
        return NULL;
    }

    BppLineNumber cur_line = 0;
    if (fread(&cur_line, sizeof(cur_line), 1, fp) != 1) {
        if (err) { err->code = 53; err->message = "Failed to read current line"; }
        fclose(fp);
        return NULL;
    }

    uint32_t line_count = 0;
    if (fread(&line_count, sizeof(line_count), 1, fp) != 1) {
        if (err) { err->code = 53; err->message = "Failed to read line count"; }
        fclose(fp);
        return NULL;
    }

    fclose(fp);

    char info_buf[256];
    snprintf(info_buf, sizeof(info_buf), "Format: BPPSTATE, Version: %lld, Current Line: %lld, Program Lines: %u",
             (long long)version, (long long)cur_line, line_count);

    size_t len = strlen(info_buf);
    char *res = (char *)calloc(1, len + 1);
    if (res) {
        memcpy(res, info_buf, len + 1);
    }
    return res;
}
