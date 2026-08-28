// FILENAME: class.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (record.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (struct.h, struct.c)
// NEEDS: libengine (class.h, eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h)
// Provides runtime implementation for the CLASS statement in BASIC++.
//
// ---- Includes ----

#include "statements/oop/structure/class.h"
#include "core/struct.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "memory/memory.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>

static BppError parse_struct_block(VMContext *vm, LexerContext *lex, bool is_class, bool is_record) {
    BppError err;
    memset(&err, 0, sizeof(err));

    if (!vm || !lex) {
        err.code = 5; err.message = "Null VM or lexer context";
        return err;
    }

    BppToken name_tok = lex_next(lex);
    if (name_tok.type != TOK_IDENT && name_tok.type != TOK_KEYWORD) {
        err.code = 2; err.message = is_class ? "Expected class name after CLASS" : "Expected record name after RECORD";
        return err;
    }

    BppUserTypeDef def;
    memset(&def, 0, sizeof(def));
    size_t name_len = (name_tok.length < sizeof(def.name) - 1) ? name_tok.length : sizeof(def.name) - 1;
    memcpy(def.name, name_tok.start, name_len);
    def.is_class = is_class;
    def.is_record = is_record;

    // Check for optional EXTENDS parent
    BppToken next_tok = lex_peek(lex);
    if ((next_tok.type == TOK_KEYWORD && next_tok.as.keyword == KW_EXTENDS) ||
        (next_tok.type == TOK_IDENT && next_tok.length == 7 && strncasecmp(next_tok.start, "EXTENDS", 7) == 0)) {
        lex_next(lex); // Consume EXTENDS
        BppToken parent_tok = lex_next(lex);
        if (parent_tok.type == TOK_IDENT || parent_tok.type == TOK_KEYWORD) {
            size_t plen = (parent_tok.length < sizeof(def.parent_name) - 1) ? parent_tok.length : sizeof(def.parent_name) - 1;
            memcpy(def.parent_name, parent_tok.start, plen);
        }
    }

    MemoryContext *mem = vm_get_mem(vm);
    size_t count = 0;
    BppProgramLine *lines = mem_program_get_all(mem, &count);
    BppLineNumber cur_ln = vm_get_current_line(vm);

    size_t start_idx = 0;
    for (size_t i = 0; i < count; ++i) {
        if (lines[i].line_number == cur_ln) { start_idx = i; break; }
    }

    bool found_end = false;
    size_t end_line_idx = start_idx;

    bool cur_private = false;

    for (size_t i = start_idx + 1; i < count; ++i) {
        LexerContext *scan = lex_init(mem, lines[i].text);
        if (!scan) continue;
        BppToken stok = lex_next(scan);

        if ((stok.type == TOK_KEYWORD && stok.as.keyword == KW_END) ||
            (stok.type == TOK_IDENT && stok.length == 3 && strncasecmp(stok.start, "END", 3) == 0)) {
            BppToken ntok = lex_next(scan);
            bool match = false;
            if (is_class && ((ntok.type == TOK_KEYWORD && ntok.as.keyword == KW_CLASS) ||
                            (ntok.type == TOK_IDENT && ntok.length == 5 && strncasecmp(ntok.start, "CLASS", 5) == 0))) {
                match = true;
            } else if (is_record && ((ntok.type == TOK_KEYWORD && ntok.as.keyword == KW_RECORD) ||
                                     (ntok.type == TOK_IDENT && ntok.length == 6 && strncasecmp(ntok.start, "RECORD", 6) == 0))) {
                match = true;
            } else if (!is_class && !is_record && ((ntok.type == TOK_KEYWORD && ntok.as.keyword == KW_TYPE) ||
                                                   (ntok.type == TOK_IDENT && ntok.length == 4 && strncasecmp(ntok.start, "TYPE", 4) == 0))) {
                match = true;
            }

            if (match) {
                found_end = true;
                end_line_idx = i;
                lex_shutdown(scan);
                break;
            }
        }

        // Check access specifier sections: PUBLIC:, PRIVATE:, PROTECTED:
        if (stok.type == TOK_KEYWORD && (stok.as.keyword == KW_PUBLIC || stok.as.keyword == KW_PRIVATE || stok.as.keyword == KW_PROTECTED)) {
            BppToken colon = lex_peek(scan);
            if (colon.type == TOK_EOL || (colon.start && *colon.start == ':')) {
                cur_private = (stok.as.keyword == KW_PRIVATE);
                lex_shutdown(scan);
                continue;
            }
        } else if (stok.type == TOK_IDENT) {
            if (stok.length == 6 && strncasecmp(stok.start, "PUBLIC", 6) == 0) {
                BppToken colon = lex_peek(scan);
                if (colon.type == TOK_EOL || (colon.start && *colon.start == ':')) {
                    cur_private = false;
                    lex_shutdown(scan);
                    continue;
                }
            } else if (stok.length == 7 && strncasecmp(stok.start, "PRIVATE", 7) == 0) {
                BppToken colon = lex_peek(scan);
                if (colon.type == TOK_EOL || (colon.start && *colon.start == ':')) {
                    cur_private = true;
                    lex_shutdown(scan);
                    continue;
                }
            } else if (stok.length == 9 && strncasecmp(stok.start, "PROTECTED", 9) == 0) {
                BppToken colon = lex_peek(scan);
                if (colon.type == TOK_EOL || (colon.start && *colon.start == ':')) {
                    cur_private = false;
                    lex_shutdown(scan);
                    continue;
                }
            }
        }

        // Check if line starts with inline PUBLIC, PRIVATE, or PROTECTED prefix
        bool line_private = cur_private;
        if (stok.type == TOK_KEYWORD && stok.as.keyword == KW_PUBLIC) {
            line_private = false;
            stok = lex_next(scan);
        } else if (stok.type == TOK_KEYWORD && stok.as.keyword == KW_PRIVATE) {
            line_private = true;
            stok = lex_next(scan);
        } else if (stok.type == TOK_KEYWORD && stok.as.keyword == KW_PROTECTED) {
            line_private = false;
            stok = lex_next(scan);
        } else if (stok.type == TOK_IDENT && stok.length == 6 && strncasecmp(stok.start, "PUBLIC", 6) == 0) {
            line_private = false;
            stok = lex_next(scan);
        } else if (stok.type == TOK_IDENT && stok.length == 7 && strncasecmp(stok.start, "PRIVATE", 7) == 0) {
            line_private = true;
            stok = lex_next(scan);
        } else if (stok.type == TOK_IDENT && stok.length == 9 && strncasecmp(stok.start, "PROTECTED", 9) == 0) {
            line_private = false;
            stok = lex_next(scan);
        }

        // Check for ABSTRACT prefix before SUB/FUNCTION
        if ((stok.type == TOK_KEYWORD && stok.as.keyword == KW_ABSTRACT) ||
            (stok.type == TOK_IDENT && stok.length == 8 && strncasecmp(stok.start, "ABSTRACT", 8) == 0)) {
            stok = lex_next(scan);
        }

        // If this line is a method definition (SUB, FUNCTION, PROPERTY, CONSTRUCTOR, DESTRUCTOR, OPERATOR), skip until its END
        bool is_method_header = false;
        if (stok.type == TOK_KEYWORD && (stok.as.keyword == KW_SUB || stok.as.keyword == KW_FUNCTION ||
                                         stok.as.keyword == KW_PROPERTY || stok.as.keyword == KW_CONSTRUCTOR ||
                                         stok.as.keyword == KW_DESTRUCTOR || stok.as.keyword == KW_OPERATOR)) {
            is_method_header = true;
        } else if (stok.type == TOK_IDENT) {
            if ((stok.length == 3 && strncasecmp(stok.start, "SUB", 3) == 0) ||
                (stok.length == 8 && strncasecmp(stok.start, "FUNCTION", 8) == 0) ||
                (stok.length == 8 && strncasecmp(stok.start, "PROPERTY", 8) == 0) ||
                (stok.length == 11 && strncasecmp(stok.start, "CONSTRUCTOR", 11) == 0) ||
                (stok.length == 10 && strncasecmp(stok.start, "DESTRUCTOR", 10) == 0) ||
                (stok.length == 8 && strncasecmp(stok.start, "OPERATOR", 8) == 0)) {
                is_method_header = true;
            }
        }

        if (is_method_header) {
            // Skip method body lines up to END SUB / END FUNCTION / END PROPERTY / END OPERATOR
            lex_shutdown(scan);
            while (i + 1 < count) {
                i++;
                LexerContext *mscan = lex_init(mem, lines[i].text);
                if (!mscan) continue;
                BppToken mtok = lex_next(mscan);
                if ((mtok.type == TOK_KEYWORD && mtok.as.keyword == KW_END) ||
                    (mtok.type == TOK_IDENT && mtok.length == 3 && strncasecmp(mtok.start, "END", 3) == 0)) {
                    BppToken ntok = lex_next(mscan);
                    if ((ntok.type == TOK_KEYWORD && (ntok.as.keyword == KW_SUB || ntok.as.keyword == KW_FUNCTION ||
                                                      ntok.as.keyword == KW_PROPERTY || ntok.as.keyword == KW_CONSTRUCTOR ||
                                                      ntok.as.keyword == KW_DESTRUCTOR || ntok.as.keyword == KW_OPERATOR)) ||
                        (ntok.type == TOK_IDENT && ((ntok.length == 3 && strncasecmp(ntok.start, "SUB", 3) == 0) ||
                                                    (ntok.length == 8 && strncasecmp(ntok.start, "FUNCTION", 8) == 0) ||
                                                    (ntok.length == 8 && strncasecmp(ntok.start, "PROPERTY", 8) == 0) ||
                                                    (ntok.length == 11 && strncasecmp(ntok.start, "CONSTRUCTOR", 11) == 0) ||
                                                    (ntok.length == 10 && strncasecmp(ntok.start, "DESTRUCTOR", 10) == 0) ||
                                                    (ntok.length == 8 && strncasecmp(ntok.start, "OPERATOR", 8) == 0)))) {
                        lex_shutdown(mscan);
                        break;
                    }
                }
                lex_shutdown(mscan);
            }
            continue;
        }

        // Check if field definition starts with optional DIM
        if ((stok.type == TOK_KEYWORD && stok.as.keyword == KW_DIM) ||
            (stok.type == TOK_IDENT && stok.length == 3 && strncasecmp(stok.start, "DIM", 3) == 0)) {
            stok = lex_next(scan);
        }

        if ((stok.type == TOK_IDENT || stok.type == TOK_KEYWORD) && def.field_count < MAX_TYPE_FIELDS) {
            BppUserTypeField *field = &def.fields[def.field_count];
            memset(field, 0, sizeof(*field));
            size_t flen = (stok.length < sizeof(field->name) - 1) ? stok.length : sizeof(field->name) - 1;
            memcpy(field->name, stok.start, flen);
            field->name[flen] = '\0';
            field->is_private = line_private;

            BppToken as_tok = lex_next(scan);
            if ((as_tok.type == TOK_KEYWORD && as_tok.as.keyword == KW_AS) ||
                (as_tok.type == TOK_IDENT && as_tok.length == 2 && strncasecmp(as_tok.start, "AS", 2) == 0)) {
                BppToken type_tok = lex_next(scan);
                char type_str[64] = {0};
                size_t tlen = (type_tok.length < sizeof(type_str) - 1) ? type_tok.length : sizeof(type_str) - 1;
                memcpy(type_str, type_tok.start, tlen);

                if (strcasecmp(type_str, "INTEGER") == 0 || strcasecmp(type_str, "LONG") == 0 ||
                    strcasecmp(type_str, "SINGLE") == 0 || strcasecmp(type_str, "DOUBLE") == 0) {
                    field->type = VAL_NUMBER;
                } else if (strcasecmp(type_str, "STRING") == 0) {
                    field->type = VAL_STRING;
                } else if (strcasecmp(type_str, "COMPLEX") == 0) {
                    field->type = VAL_COMPLEX;
                } else {
                    field->type = VAL_MAP;
                    strncpy(field->nested_type, type_str, sizeof(field->nested_type) - 1);
                }
            } else {
                field->type = VAL_NUMBER;
            }
            def.field_count++;
        }
        lex_shutdown(scan);
    }

    if (!found_end) {
        err.code = 38;
        err.message = is_class ? "CLASS Without END CLASS" : "RECORD Without END RECORD";
        return err;
    }

    char err_buf[128] = {0};
    if (!struct_register_type(vm_get_types(vm), &def, err_buf, sizeof(err_buf))) {
        err.code = 10; err.message = "Duplicate definition";
        return err;
    }

    vm_jump(vm, lines[end_line_idx].line_number, lines[end_line_idx].text);
    return err;
}

BppError stmt_class_handler(VMContext *vm, LexerContext *lex) {
    return parse_struct_block(vm, lex, true, false);
}

BppError stmt_record_type_handler(VMContext *vm, LexerContext *lex) {
    return parse_struct_block(vm, lex, false, true);
}

void stmt_class_register(void) {
    static const MicroLibMetadata meta = {
        .name = "CLASS",
        .category = "Object-Oriented Programming",
        .syntax = "CLASS class_name [EXTENDS parent] \n member_name AS type \n ... \n END CLASS",
        .help_text = "Defines an Object-Oriented class with inheritance and encapsulation support.",
        .error_codes = "Error 2: Syntax Error, Error 38: CLASS Without END CLASS, Error 10: Duplicate Definition"
    };
    microlib_register(&meta);
}

void stmt_record_type_register(void) {
    static const MicroLibMetadata meta = {
        .name = "RECORD",
        .category = "Variables & Memory",
        .syntax = "RECORD record_name \n member_name AS type \n ... \n END RECORD",
        .help_text = "Defines a VAX BASIC / DEC RECORD structured composite data type.",
        .error_codes = "Error 2: Syntax Error, Error 38: RECORD Without END RECORD, Error 10: Duplicate Definition"
    };
    microlib_register(&meta);
}
