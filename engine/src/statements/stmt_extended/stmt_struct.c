/**
 * @file stmt_struct.c
 * @brief TYPE ... END TYPE and CLASS ... END CLASS statement handler implementation.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Implements block parsing for user-defined structures and class declarations.
 * - Why it exists: Fulfills Dartmouth and QBasic compatibility for custom structures and object classes.
 * - Why it works this way: It scans ahead in program memory to extract field attributes, registers the UDT/CLASS schema,
 *   and skips execution past the block using vm_jump.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Type/class syntax validators, additional numeric categories mapping.
 * - What cannot be changed: End block matching checks.
 * - What to expect: Parsing happens at runtime execution of the block header.
 * - What to do if something breaks: Trace start_idx matching line number checks.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: Program lines are stored sorted and contiguous.
 * - Portability concerns: Standard C17 compliant.
 */

#include "stmt/stmt.h"
#include "core/struct.h"
#include "lexer/lexer.h"
#include "vm/vm.h"
#include "eval/eval.h"
#include "device/vdev.h"
#include "runtime/using.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

BppError stmt_type_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    MemoryContext *mem = vm_get_mem(vm);
    BppTypeRegistry *reg = vm_get_types(vm);

    BppToken name_tok = lex_peek(lex);
    if (name_tok.type == TOK_KEYWORD && name_tok.as.keyword == KW_USING) {
        lex_next(lex); /* Consume USING */
        
        /* 1. Format string expression */
        BValue fmt_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (fmt_val.type != VAL_STRING) {
            err.code = 13; err.message = "Type mismatch: TYPE USING expects format string";
            return err;
        }
        const char *fmt_str = str_data(fmt_val.as.string);

        /* 2. Semicolon separator */
        BppToken tok = lex_next(lex);
        if (tok.type != TOK_SEMICOLON) {
            err.code = 2; err.message = "Expected ';' in TYPE USING statement";
            str_release(vm_get_str(vm), fmt_val.as.string);
            return err;
        }

        /* 3. File path expression */
        BValue file_val = eval_expression(vm, lex, &err);
        if (err.code != 0) {
            str_release(vm_get_str(vm), fmt_val.as.string);
            return err;
        }
        if (file_val.type != VAL_STRING) {
            err.code = 13; err.message = "Type mismatch: TYPE USING expects file path string";
            str_release(vm_get_str(vm), fmt_val.as.string);
            return err;
        }
        const char *file_path = str_data(file_val.as.string);

        /* Parse USING format mask */
        UsingMask mask;
        using_parse_mask(fmt_str, &mask);

        /* Open file and read lines */
        FILE *fp = fopen(file_path, "r");
        if (!fp) {
            err.code = 53; err.message = "File not found";
            str_release(vm_get_str(vm), fmt_val.as.string);
            str_release(vm_get_str(vm), file_val.as.string);
            return err;
        }

        char line[512];
        int mask_idx = 0;
        VDevContext *vdev = vm_get_vdev(vm);

        while (fgets(line, sizeof(line), fp)) {
            size_t len = strlen(line);
            if (len > 0 && line[len - 1] == '\n') {
                line[len - 1] = '\0';
                len--;
            }
            if (len > 0 && line[len - 1] == '\r') {
                line[len - 1] = '\0';
                len--;
            }

            BValue line_val;
            line_val.type = VAL_STRING;
            line_val.as.string = str_create(vm_get_str(vm), line, len);

            char out_buf[1024];
            using_format_output(vm, &mask, &mask_idx, line_val, out_buf, sizeof(out_buf));
            vdev_puts(vdev, out_buf);
            vdev_putc(vdev, '\n');

            str_release(vm_get_str(vm), line_val.as.string);
        }

        fclose(fp);
        str_release(vm_get_str(vm), fmt_val.as.string);
        str_release(vm_get_str(vm), file_val.as.string);
        return err;
    }

    /* Read Type Name */
    name_tok = lex_next(lex);
    if (name_tok.type != TOK_IDENT) {
        err.code = 2; err.message = "Expected type name in TYPE statement";
        return err;
    }

    BppUserTypeDef def;
    memset(&def, 0, sizeof(def));
    size_t nlen = (name_tok.length < 63) ? name_tok.length : 63;
    memcpy(def.name, name_tok.start, nlen);
    def.name[nlen] = '\0';
    def.is_class = false;

    /* Scan subsequent lines for fields */
    size_t count = 0;
    BppProgramLine *lines = mem_program_get_all(mem, &count);
    BppLineNumber cur_ln = vm_get_current_line(vm);
    size_t start_idx = 0;
    bool found = false;

    for (size_t i = 0; i < count; ++i) {
        if (lines[i].line_number == cur_ln) {
            start_idx = i;
            found = true;
            break;
        }
    }

    if (!found) {
        err.code = 11; err.message = "State corruption during UDT scan";
        return err;
    }

    for (size_t i = start_idx + 1; i < count; ++i) {
        LexerContext *scan_lex = lex_init(mem, lines[i].text);
        if (!scan_lex) continue;

        BppToken tok = lex_next(scan_lex);
        if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_END) {
            BppToken next_tok = lex_next(scan_lex);
            if (next_tok.type == TOK_KEYWORD && next_tok.as.keyword == KW_TYPE) {
                /* End of type definition */
                char reg_err[128];
                if (!struct_register_type(reg, &def, reg_err, sizeof(reg_err))) {
                    err.code = 2; err.message = "Type registration failed";
                    lex_shutdown(scan_lex);
                    return err;
                }
                /* Jump past END TYPE */
                vm_jump(vm, lines[i].line_number, lines[i].text);
                lex_shutdown(scan_lex);
                return err;
            }
        } else if (tok.type == TOK_IDENT) {
            /* Field: FieldName AS TypeName */
            if (def.field_count >= MAX_TYPE_FIELDS) {
                err.code = 2; err.message = "Too many fields in TYPE definition";
                lex_shutdown(scan_lex);
                return err;
            }
            BppUserTypeField *f = &def.fields[def.field_count++];
            size_t flen = (tok.length < 63) ? tok.length : 63;
            memcpy(f->name, tok.start, flen);
            f->name[flen] = '\0';

            /* Expect AS */
            BppToken as_tok = lex_next(scan_lex);
            if (as_tok.type != TOK_KEYWORD || as_tok.as.keyword != KW_AS) {
                err.code = 2; err.message = "Expected AS in field definition";
                lex_shutdown(scan_lex);
                return err;
            }

            /* Read type */
            BppToken type_tok = lex_next(scan_lex);
            if (type_tok.type != TOK_IDENT) {
                err.code = 2; err.message = "Expected type name after AS";
                lex_shutdown(scan_lex);
                return err;
            }

            char type_str[64];
            size_t tlen = (type_tok.length < 63) ? type_tok.length : 63;
            memcpy(type_str, type_tok.start, tlen);
            type_str[tlen] = '\0';

            if (strcasecmp(type_str, "INTEGER") == 0 || strcasecmp(type_str, "LONG") == 0 ||
                strcasecmp(type_str, "SINGLE") == 0 || strcasecmp(type_str, "DOUBLE") == 0 ||
                strcasecmp(type_str, "NUMBER") == 0) {
                f->type = VAL_NUMBER;
            } else if (strcasecmp(type_str, "STRING") == 0) {
                f->type = VAL_STRING;
                /* Check fixed string: * N */
                BppToken next_tok = lex_peek(scan_lex);
                if (next_tok.type == TOK_MUL || (next_tok.type == TOK_UNKNOWN && *next_tok.start == '*')) {
                    lex_next(scan_lex); /* Consume '*' */
                    BppToken num_tok = lex_next(scan_lex);
                    if (num_tok.type != TOK_NUMBER) {
                        err.code = 2; err.message = "Expected length after STRING *";
                        lex_shutdown(scan_lex);
                        return err;
                    }
                }
            } else {
                /* Nested Type: must be already registered */
                const BppUserTypeDef *nested_def = struct_find_type(reg, type_str);
                if (!nested_def) {
                    err.code = 35; err.message = "Field type not defined";
                    lex_shutdown(scan_lex);
                    return err;
                }
                f->type = VAL_MAP;
                snprintf(f->nested_type, sizeof(f->nested_type), "%s", nested_def->name);
            }
        }
        lex_shutdown(scan_lex);
    }

    err.code = 2; err.message = "TYPE defined without END TYPE";
    return err;
}

BppError stmt_class_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    MemoryContext *mem = vm_get_mem(vm);
    BppTypeRegistry *reg = vm_get_types(vm);

    /* Read Class Name */
    BppToken name_tok = lex_next(lex);
    if (name_tok.type != TOK_IDENT) {
        err.code = 2; err.message = "Expected class name in CLASS statement";
        return err;
    }

    BppUserTypeDef def;
    memset(&def, 0, sizeof(def));
    size_t nlen = (name_tok.length < 63) ? name_tok.length : 63;
    memcpy(def.name, name_tok.start, nlen);
    def.name[nlen] = '\0';
    def.is_class = true;

    /* Scan subsequent lines for fields and methods */
    size_t count = 0;
    BppProgramLine *lines = mem_program_get_all(mem, &count);
    BppLineNumber cur_ln = vm_get_current_line(vm);
    size_t start_idx = 0;
    bool found = false;

    for (size_t i = 0; i < count; ++i) {
        if (lines[i].line_number == cur_ln) {
            start_idx = i;
            found = true;
            break;
        }
    }

    if (!found) {
        err.code = 11; err.message = "State corruption during CLASS scan";
        return err;
    }

    for (size_t i = start_idx + 1; i < count; ++i) {
        LexerContext *scan_lex = lex_init(mem, lines[i].text);
        if (!scan_lex) continue;

        BppToken tok = lex_next(scan_lex);
        if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_END) {
            BppToken next_tok = lex_next(scan_lex);
            if (next_tok.type == TOK_KEYWORD && next_tok.as.keyword == KW_CLASS) {
                /* End of class definition */
                char reg_err[128];
                if (!struct_register_type(reg, &def, reg_err, sizeof(reg_err))) {
                    err.code = 2; err.message = "Class registration failed";
                    lex_shutdown(scan_lex);
                    return err;
                }
                /* Jump past END CLASS */
                vm_jump(vm, lines[i].line_number, lines[i].text);
                lex_shutdown(scan_lex);
                return err;
            }
        } else if (tok.type == TOK_KEYWORD && (tok.as.keyword == KW_SUB || tok.as.keyword == KW_FUNCTION)) {
            /* Skip method body */
            BppKeywordId m_kw = tok.as.keyword;
            int nesting = 0;
            for (size_t j = i + 1; j < count; ++j) {
                LexerContext *m_lex = lex_init(mem, lines[j].text);
                if (!m_lex) continue;
                BppToken mt = lex_next(m_lex);
                if (mt.type == TOK_KEYWORD) {
                    if (mt.as.keyword == KW_SUB || mt.as.keyword == KW_FUNCTION) {
                        nesting++;
                    } else if (mt.as.keyword == KW_END) {
                        BppToken next_mt = lex_next(m_lex);
                        if (next_mt.type == TOK_KEYWORD && next_mt.as.keyword == m_kw) {
                            if (nesting > 0) {
                                nesting--;
                            } else {
                                i = j; /* Skip past this method */
                                lex_shutdown(m_lex);
                                break;
                            }
                        }
                    }
                }
                lex_shutdown(m_lex);
            }
        } else if (tok.type == TOK_IDENT) {
            /* Field: FieldName AS TypeName */
            if (def.field_count >= MAX_TYPE_FIELDS) {
                err.code = 2; err.message = "Too many fields in CLASS definition";
                lex_shutdown(scan_lex);
                return err;
            }
            BppUserTypeField *f = &def.fields[def.field_count++];
            size_t flen = (tok.length < 63) ? tok.length : 63;
            memcpy(f->name, tok.start, flen);
            f->name[flen] = '\0';

            /* Expect AS */
            BppToken as_tok = lex_next(scan_lex);
            if (as_tok.type != TOK_KEYWORD || as_tok.as.keyword != KW_AS) {
                err.code = 2; err.message = "Expected AS in field definition";
                lex_shutdown(scan_lex);
                return err;
            }

            /* Read type */
            BppToken type_tok = lex_next(scan_lex);
            if (type_tok.type != TOK_IDENT) {
                err.code = 2; err.message = "Expected type name after AS";
                lex_shutdown(scan_lex);
                return err;
            }

            char type_str[64];
            size_t tlen = (type_tok.length < 63) ? type_tok.length : 63;
            memcpy(type_str, type_tok.start, tlen);
            type_str[tlen] = '\0';

            if (strcasecmp(type_str, "INTEGER") == 0 || strcasecmp(type_str, "LONG") == 0 ||
                strcasecmp(type_str, "SINGLE") == 0 || strcasecmp(type_str, "DOUBLE") == 0 ||
                strcasecmp(type_str, "NUMBER") == 0) {
                f->type = VAL_NUMBER;
            } else if (strcasecmp(type_str, "STRING") == 0) {
                f->type = VAL_STRING;
                /* Check fixed string: * N */
                BppToken next_tok = lex_peek(scan_lex);
                if (next_tok.type == TOK_MUL || (next_tok.type == TOK_UNKNOWN && *next_tok.start == '*')) {
                    lex_next(scan_lex); /* Consume '*' */
                    BppToken num_tok = lex_next(scan_lex);
                    if (num_tok.type != TOK_NUMBER) {
                        err.code = 2; err.message = "Expected length after STRING *";
                        lex_shutdown(scan_lex);
                        return err;
                    }
                }
            } else {
                /* Nested Type: must be already registered */
                const BppUserTypeDef *nested_def = struct_find_type(reg, type_str);
                if (!nested_def) {
                    err.code = 35; err.message = "Field type not defined";
                    lex_shutdown(scan_lex);
                    return err;
                }
                f->type = VAL_MAP;
                snprintf(f->nested_type, sizeof(f->nested_type), "%s", nested_def->name);
            }
        }
        lex_shutdown(scan_lex);
    }

    err.code = 2; err.message = "CLASS defined without END CLASS";
    return err;
}

BppError stmt_enum_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    MemoryContext *mem = vm_get_mem(vm);

    /* Read Enum Name */
    BppToken name_tok = lex_next(lex);
    if (name_tok.type != TOK_IDENT && name_tok.type != TOK_KEYWORD) {
        err.code = 2; err.message = "Expected enum name in ENUM statement";
        return err;
    }

    char enum_name[64];
    size_t nlen = (name_tok.length < 63) ? name_tok.length : 63;
    memcpy(enum_name, name_tok.start, nlen);
    enum_name[nlen] = '\0';

    /* Scan subsequent lines for fields */
    size_t count = 0;
    BppProgramLine *lines = mem_program_get_all(mem, &count);
    BppLineNumber cur_ln = vm_get_current_line(vm);
    size_t start_idx = 0;
    bool found = false;

    for (size_t i = 0; i < count; ++i) {
        if (lines[i].line_number == cur_ln) {
            start_idx = i;
            found = true;
            break;
        }
    }

    if (!found) {
        err.code = 11; err.message = "State corruption during ENUM scan";
        return err;
    }

    double current_value = 0.0;

    for (size_t i = start_idx + 1; i < count; ++i) {
        LexerContext *scan_lex = lex_init(mem, lines[i].text);
        if (!scan_lex) continue;

        BppToken tok = lex_next(scan_lex);
        if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_END) {
            BppToken next_tok = lex_next(scan_lex);
            if (next_tok.type == TOK_KEYWORD && next_tok.as.keyword == KW_ENUM) {
                /* End of enum definition */
                vm_jump(vm, lines[i].line_number, lines[i].text);
                lex_shutdown(scan_lex);
                return err;
            }
        } else if (tok.type == TOK_IDENT || tok.type == TOK_KEYWORD) {
            /* Enum element: ElementName or ElementName = Value */
            char elem_name[64];
            size_t elen = (tok.length < 63) ? tok.length : 63;
            memcpy(elem_name, tok.start, elen);
            elem_name[elen] = '\0';

            BppToken eq_tok = lex_peek(scan_lex);
            if (eq_tok.type == TOK_EQ) {
                lex_next(scan_lex); /* Consume '=' */
                BppToken val_tok = lex_next(scan_lex);
                if (val_tok.type != TOK_NUMBER) {
                    err.code = 2; err.message = "Expected numeric value for enum element";
                    lex_shutdown(scan_lex);
                    return err;
                }
                current_value = val_tok.as.number;
            }

            /* Create global variable name: EnumName.ElementName */
            char fq_name[384];
            snprintf(fq_name, sizeof(fq_name), "%s.%s", enum_name, elem_name);

            /* Assign in variable context */
            BValue val;
            val.type = VAL_NUMBER;
            val.as.number = current_value;

            BValue *var_ptr = var_lookup(vm_get_var(vm), fq_name, true);
            if (var_ptr) {
                *var_ptr = val;
            }

            current_value += 1.0;
        }
        lex_shutdown(scan_lex);
    }

    err.code = 2; err.message = "ENUM defined without END ENUM";
    return err;
}
