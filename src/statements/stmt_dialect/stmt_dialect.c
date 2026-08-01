/**
 * @file stmt_dialect.c
 * @brief DIALECT statement handler implementation.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Implements the DIALECT statement, parsing subcommands (LOAD, REGISTER, INFO)
 *   and name strings to load, validate, register, and display custom dialect configurations.
 * - Why it exists: Fulfills the Dialect Configuration Layer requirements to support custom tokenizing,
 *   case sensitivity, operator precedence, custom separators, and preprocessor hooks.
 * - Why it works this way: Parses arguments and delegates to the dialect engine API defined in src/core/dialect.c.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Predefined dialect profiles, default fallback modes.
 * - What cannot be changed: Verification patterns and memory safety boundary limits.
 * - What to expect: Changes to the active dialect immediately modify the lexer's scanning rules.
 * - What to do if something breaks: Verify if the input string or map is parsed correctly by the dialect engine.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: File operations use standard C I/O APIs.
 * - Portability concerns: Path parsing handles directory separators portably.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - How future expansion can occur safely: Add additional dialect validation schemas.
 * - External extension hooks: Custom dialects can register custom callback hooks.
 */

#include "bpp_vm.h"
#include "bpp_lexer.h"
#include "bpp_vdev.h"
#include "bpp_eval.h"
#include "bpp_strings.h"
#include "bpp_dialect.h"
#include "bpp_map.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "bpp_config.h"

static char *read_file_to_string(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (len < 0) {
        fclose(f);
        return NULL;
    }
    char *buf = (char *)calloc(1, len + 1);
    if (!buf) {
        fclose(f);
        return NULL;
    }
    size_t read_bytes = fread(buf, 1, len, f);
    buf[read_bytes] = '\0';
    fclose(f);
    return buf;
}

BppError stmt_dialect_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if (tok.type != TOK_IDENT && tok.type != TOK_STRING && tok.type != TOK_NUMBER && tok.type != TOK_KEYWORD) {
        err.code = 2; err.message = "Syntax error: expected sub-command or dialect name for DIALECT";
        return err;
    }

    if ((tok.type == TOK_IDENT || tok.type == TOK_KEYWORD) && tok.length == 4 && strncasecmp(tok.start, "LOAD", 4) == 0) {
        lex_next(lex); /* Consume "LOAD" */
        
        BValue val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;

        BValue format_val;
        memset(&format_val, 0, sizeof(format_val));
        bool has_format = false;

        tok = lex_peek(lex);
        if (tok.type == TOK_COMMA) {
            lex_next(lex); /* Consume comma */
            format_val = eval_expression(vm, lex, &err);
            if (err.code != 0) {
                if (val.type == VAL_STRING && val.as.string) str_release(vm_get_str(vm), val.as.string);
                return err;
            }
            has_format = true;
        }

        if (val.type != VAL_STRING) {
            err.code = 13; err.message = "DIALECT LOAD expects a string filepath or inline specification";
            if (val.type == VAL_STRING && val.as.string) str_release(vm_get_str(vm), val.as.string);
            if (has_format && format_val.type == VAL_STRING && format_val.as.string) str_release(vm_get_str(vm), format_val.as.string);
            return err;
        }
        if (has_format && format_val.type != VAL_STRING) {
            err.code = 13; err.message = "DIALECT LOAD expects format string as the second argument";
            str_release(vm_get_str(vm), val.as.string);
            return err;
        }

        const char *spec_str = str_data(val.as.string);
        char *file_content = NULL;
        const char *format = NULL;

        if (has_format) {
            format = str_data(format_val.as.string);
            FILE *temp_fp = fopen(spec_str, "r");
            if (temp_fp) {
                fclose(temp_fp);
                file_content = read_file_to_string(spec_str);
            }
        } else {
            FILE *temp_fp = fopen(spec_str, "r");
            if (!temp_fp) {
                str_release(vm_get_str(vm), val.as.string);
                err.code = 5; err.message = "Dialect spec file not found or invalid format";
                return err;
            }
            fclose(temp_fp);
            file_content = read_file_to_string(spec_str);
            if (!file_content) {
                str_release(vm_get_str(vm), val.as.string);
                err.code = 5; err.message = "Failed to read dialect spec file";
                return err;
            }
            const char *ext = strrchr(spec_str, '.');
            if (ext) {
                if (strcasecmp(ext, ".json") == 0) {
                    format = "JSON";
                } else if (strcasecmp(ext, ".ini") == 0) {
                    format = "INI";
                } else if (strcasecmp(ext, ".xml") == 0) {
                    format = "XML";
                } else if (strcasecmp(ext, ".yaml") == 0 || strcasecmp(ext, ".yml") == 0) {
                    format = "YAML";
                }
            }
            if (!format) {
                if (file_content) free(file_content);
                str_release(vm_get_str(vm), val.as.string);
                err.code = 5; err.message = "Could not infer dialect format from file extension";
                return err;
            }
        }

        const char *parse_source = file_content ? file_content : spec_str;
        BppMap *map = NULL;
        if (strcasecmp(format, "JSON") == 0) {
            map = bpp_map_parse_json(vm_get_str(vm), parse_source);
        } else if (strcasecmp(format, "XML") == 0) {
            map = bpp_map_parse_xml(vm_get_str(vm), parse_source);
        } else if (strcasecmp(format, "YAML") == 0) {
            map = bpp_map_parse_yaml(vm_get_str(vm), parse_source);
        } else if (strcasecmp(format, "INI") == 0) {
            map = bpp_map_parse_ini(vm_get_str(vm), parse_source);
        } else {
            err.code = 5; err.message = "Unsupported spec format";
        }

        if (file_content) free(file_content);
        str_release(vm_get_str(vm), val.as.string);
        if (has_format) str_release(vm_get_str(vm), format_val.as.string);

        if (err.code != 0) return err;
        if (!map) {
            err.code = 5; err.message = "Failed to parse dialect spec content";
            return err;
        }

        BppDialect *d = dialect_create();
        char val_err[512] = "";
        if (!d) {
            err.code = 14; err.message = "Out of memory allocating dialect";
            bpp_map_release(vm_get_str(vm), map);
            return err;
        }
        if (!dialect_load_from_map(vm, map, d, val_err, sizeof(val_err))) {
            dialect_free(d);
            bpp_map_release(vm_get_str(vm), map);
            err.code = 5;
            static char err_msg_buf[512];
            strncpy(err_msg_buf, val_err, sizeof(err_msg_buf) - 1);
            err_msg_buf[sizeof(err_msg_buf) - 1] = '\0';
            err.message = err_msg_buf;
            return err;
        }
        bpp_map_release(vm_get_str(vm), map);
        vm_set_active_dialect(vm, d);
    }
    else if ((tok.type == TOK_IDENT || tok.type == TOK_KEYWORD) && tok.length == 8 && strncasecmp(tok.start, "REGISTER", 8) == 0) {
        lex_next(lex); /* Consume "REGISTER" */
        
        BValue val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (val.type != VAL_MAP) {
            err.code = 13; err.message = "DIALECT REGISTER expects a MAP argument";
            return err;
        }

        BppDialect *d = dialect_create();
        char val_err[512] = "";
        if (!d) {
            err.code = 14; err.message = "Out of memory allocating dialect";
            bpp_map_release(vm_get_str(vm), val.as.map);
            return err;
        }
        if (!dialect_load_from_map(vm, val.as.map, d, val_err, sizeof(val_err))) {
            dialect_free(d);
            bpp_map_release(vm_get_str(vm), val.as.map);
            err.code = 5;
            static char err_msg_buf2[512];
            strncpy(err_msg_buf2, val_err, sizeof(err_msg_buf2) - 1);
            err_msg_buf2[sizeof(err_msg_buf2) - 1] = '\0';
            err.message = err_msg_buf2;
            return err;
        }
        bpp_map_release(vm_get_str(vm), val.as.map);
        vm_set_active_dialect(vm, d);
    }
    else if ((tok.type == TOK_IDENT || tok.type == TOK_KEYWORD) && tok.length == 4 && strncasecmp(tok.start, "INFO", 4) == 0) {
        lex_next(lex); /* Consume "INFO" */
        
        BppDialect *d = vm_get_active_dialect(vm);
        if (d) {
            char *docs = dialect_generate_docs(vm, d);
            if (docs) {
                vdev_printf(vm_get_vdev(vm), "%s\n", docs);
                free(docs);
            } else {
                vdev_printf(vm_get_vdev(vm), "Active Dialect: %s\n", d->name);
            }
        } else {
            vdev_printf(vm_get_vdev(vm), "Active Dialect: BASIC++ (Default)\n");
        }
    }
    else {
        /* Evaluate the expression (should evaluate to string, e.g. DIALECT "gwbasic" or "qbasic") */
        BValue val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (val.type != VAL_STRING) {
            err.code = 13; err.message = "Syntax error: expected sub-command or dialect name string";
            return err;
        }

        const char *name = str_data(val.as.string);
        if (strcasecmp(name, "gwbasic") == 0 || strcasecmp(name, "qbasic") == 0 || strcasecmp(name, "basic++") == 0) {
            /* Restore default configuration by setting active dialect to NULL (BASIC++ default handles it) */
            vm_set_active_dialect(vm, NULL);
        } else {
            err.code = 5; err.message = "Unknown predefined dialect name";
        }
        str_release(vm_get_str(vm), val.as.string);
    }

    return err;
}
