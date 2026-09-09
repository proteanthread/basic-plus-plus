// FILENAME: type.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore, libengine, libkernel, libplatform
// Provides runtime implementation for the TYPE statement in BASIC++.
//
// ---- Includes ----

#include "statements/oop/structure/type.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "device/vdev.h"
#include "security/security.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

#include "memory/memory.h"
#include "core/struct.h"
#include "platform/platform.h"
#include "runtime/variables_internal.h"
#include "runtime/format/snprintf.h"

static const LangDesc g_type_desc = {
    .name = "TYPE",
    .category = "Variables & Memory",
    .syntax = "TYPE type_name \n member_name AS type | member: type \n ... \n END TYPE | ENDTYPE",
    .description = "Defines a user-defined data structure (UDT/record) containing element fields.",
    .error_summary = "Error 2: Syntax Error, Error 38: TYPE Without END TYPE, Error 10: Duplicate Definition",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SYSTEM,
    .type = FEATURE_STATEMENT
};

BppError stmt_type_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    if (!vm || !lex) {
        err.code = 5; err.message = "Null VM or lexer context";
        return err;
    }

    BppToken first_peek = lex_peek(lex);
    bool is_struct_def = false;
    MemoryContext *mem = vm_get_mem(vm);
    size_t count = 0;
    BppProgramLine *lines = mem_program_get_all(mem, &count);
    BppLineNumber cur_ln = vm_get_current_line(vm);
    size_t end_type_line_idx = 0;
    BppUserTypeDef def;
    runtime_memset(&def, 0, sizeof(def));

    if ((first_peek.type == TOK_IDENT || first_peek.type == TOK_KEYWORD) && count > 0 && cur_ln != 0) {
        size_t start_idx = 0;
        bool found_cur = false;
        for (size_t i = 0; i < count; ++i) {
            if (lines[i].line_number == cur_ln) { start_idx = i; found_cur = true; break; }
        }

        if (found_cur) {
            for (size_t i = start_idx + 1; i < count; ++i) {
                LexerContext *scan = lex_init(mem, lines[i].text);
                BppToken stok = lex_next(scan);
                if ((stok.type == TOK_KEYWORD && (stok.as.keyword == KW_ENDTYPE)) ||
                    (stok.type == TOK_IDENT && stok.length == 7 && platform_strncasecmp(stok.start, "ENDTYPE", 7) == 0)) {
                    is_struct_def = true;
                    end_type_line_idx = i;
                    lex_shutdown(scan);
                    break;
                }
                if ((stok.type == TOK_KEYWORD && stok.as.keyword == KW_END) ||
                    (stok.type == TOK_IDENT && stok.length == 3 && platform_strncasecmp(stok.start, "END", 3) == 0)) {
                    BppToken ntok = lex_next(scan);
                    if ((ntok.type == TOK_KEYWORD && ntok.as.keyword == KW_TYPE) ||
                        (ntok.type == TOK_IDENT && ntok.length == 4 && platform_strncasecmp(ntok.start, "TYPE", 4) == 0)) {
                        is_struct_def = true;
                        end_type_line_idx = i;
                        lex_shutdown(scan);
                        break;
                    }
                }
                lex_shutdown(scan);
            }
        }
    }

    if (is_struct_def) {
        BppToken name_tok = lex_next(lex);
        size_t name_len = (name_tok.length < sizeof(def.name) - 1) ? name_tok.length : sizeof(def.name) - 1;
        runtime_memcpy(def.name, name_tok.start, name_len);
        def.is_class = false;

        size_t start_idx = 0;
        for (size_t i = 0; i < count; ++i) {
            if (lines[i].line_number == cur_ln) { start_idx = i; break; }
        }

        for (size_t i = start_idx + 1; i < end_type_line_idx; ++i) {
            LexerContext *scan = lex_init(mem, lines[i].text);
            BppToken stok = lex_next(scan);

            // Check if field definition starts with optional DIM
            if ((stok.type == TOK_KEYWORD && stok.as.keyword == KW_DIM) ||
                (stok.type == TOK_IDENT && stok.length == 3 && platform_strncasecmp(stok.start, "DIM", 3) == 0)) {
                stok = lex_next(scan);
            }

            if ((stok.type == TOK_IDENT || stok.type == TOK_KEYWORD) && def.field_count < MAX_TYPE_FIELDS) {
                BppUserTypeField *field = &def.fields[def.field_count];
                runtime_memset(field, 0, sizeof(*field));
                size_t flen = (stok.length < sizeof(field->name) - 1) ? stok.length : sizeof(field->name) - 1;
                runtime_memcpy(field->name, stok.start, flen);
                field->name[flen] = '\0';

                BppToken as_tok = lex_next(scan);
                // Support both 'field AS type' (QBASIC) and 'field : type' (BASIC09)
                if ((as_tok.type == TOK_KEYWORD && as_tok.as.keyword == KW_AS) ||
                    (as_tok.type == TOK_IDENT && as_tok.length == 2 && platform_strncasecmp(as_tok.start, "AS", 2) == 0) ||
                    (as_tok.start && as_tok.length == 1 && as_tok.start[0] == ':')) {
                    BppToken type_tok = lex_next(scan);
                    char type_str[64] = {0};
                    size_t tlen = (type_tok.length < sizeof(type_str) - 1) ? type_tok.length : sizeof(type_str) - 1;
                    runtime_memcpy(type_str, type_tok.start, tlen);

                    if (platform_strcasecmp(type_str, "INTEGER") == 0 || platform_strcasecmp(type_str, "LONG") == 0 ||
                        platform_strcasecmp(type_str, "SINGLE") == 0 || platform_strcasecmp(type_str, "DOUBLE") == 0 ||
                        platform_strcasecmp(type_str, "BYTE") == 0 || platform_strcasecmp(type_str, "REAL") == 0 ||
                        platform_strcasecmp(type_str, "BOOLEAN") == 0) {
                        field->type = VAL_NUMBER;
                    } else if (platform_strcasecmp(type_str, "STRING") == 0) {
                        field->type = VAL_STRING;
                    } else {
                        field->type = VAL_MAP;
                        runtime_strncpy(field->nested_type, type_str, sizeof(field->nested_type) - 1);
                    }
                } else {
                    field->type = VAL_NUMBER;
                }
                def.field_count++;
            }
            lex_shutdown(scan);
        }

        char err_buf[128] = {0};
        if (!struct_register_type(vm_get_types(vm), &def, err_buf, sizeof(err_buf))) {
            err.code = 10; err.message = "Duplicate TYPE definition";
            return err;
        }

        vm_jump(vm, lines[end_type_line_idx].line_number, lines[end_type_line_idx].text);
        return err;
    }

    err.code = 38;
    err.message = "TYPE without END TYPE";
    return err;
}

void stmt_type_register(void) {
    lang_desc_register(&g_type_desc);
}

BppError stmt_end_type_handler(VMContext *vm, LexerContext *lex) {
    (void)vm;
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    if (lex) {
        lex_next(lex);
    }
    return err;
}

