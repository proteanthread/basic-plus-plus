// FILENAME: print_using.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (ctype.h, ctype.c, file.h, file.c, string.h, using.h)
// NEEDS: libengine (eval.h, eval.c, stmt.h, string.c)
// NEEDS: libkernel (vdev.h, vdev.c, vprinter.h, vprinter.c)
// Provides core logic and interface definitions for print_using within BASIC++.
//
// ---- Includes ----

#include "stmt/stmt.h"
#include "eval/eval.h"
#include "device/vdev.h"
#include "device/vprinter.h"
#include "runtime/file.h"
#include "runtime/using.h"
#include "runtime/format/snprintf.h"
#include "runtime/memory/alloc.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/ctype/ctype.h"
#include "platform/platform.h"

void print_using_internal_ex(VMContext *vm, LexerContext *lex, int channel, void *stream) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    // 1. Format string expression or IMAGE line number
    BValue fmt_val = eval_expression(vm, lex, &err);
    if (err.code != 0) {
        vm_set_error(vm, err.code, err.message);
        return;
    }
    
    char image_buf[512] = "";
    const char *fmt_str = NULL;
    if (fmt_val.type == VAL_STRING) {
        fmt_str = str_data(fmt_val.as.string);
    } else if (fmt_val.type == VAL_NUMBER || fmt_val.type == VAL_INTEGER) {
        BppLineNumber target_line = fmt_val.as.number;
        const char *line_text = mem_program_get(vm_get_mem(vm), target_line);
        if (!line_text) {
            vm_set_error(vm, 8, "Undefined line number in PRINT USING");
            return;
        }
        const char *p = line_text;
        while (runtime_isspace((unsigned char)*p)) p++;
        if (runtime_strncasecmp(p, "IMAGE", 5) == 0) {
            p += 5;
            while (runtime_isspace((unsigned char)*p) || *p == ':') p++;
            runtime_strncpy(image_buf, p, sizeof(image_buf) - 1);
            image_buf[sizeof(image_buf) - 1] = '\0';
            fmt_str = image_buf;
        } else if (runtime_strncasecmp(p, "FORM", 4) == 0) {
            p += 4;
            while (runtime_isspace((unsigned char)*p) || *p == ':') p++;
            runtime_strncpy(image_buf, p, sizeof(image_buf) - 1);
            image_buf[sizeof(image_buf) - 1] = '\0';
            fmt_str = image_buf;
        } else {
            runtime_strncpy(image_buf, p, sizeof(image_buf) - 1);
            image_buf[sizeof(image_buf) - 1] = '\0';
            fmt_str = image_buf;
        }
    } else {
        vm_set_error(vm, 13, "Type mismatch: PRINT USING expects format string or line number");
        return;
    }

    // 2. Semicolon or comma separator
    BppToken tok = lex_next(lex);
    if (tok.type != TOK_SEMICOLON && tok.type != TOK_COMMA) {
        vm_set_error(vm, 2, "Expected ';' or ',' in PRINT USING statement");
        if (fmt_val.type == VAL_STRING && fmt_val.as.string) str_release(vm_get_str(vm), fmt_val.as.string);
        return;
    }

    // Parse USING format mask
    UsingMask mask;
    using_parse_mask(fmt_str, &mask);

    if (mask.token_count == 0) {
        if (fmt_val.type == VAL_STRING && fmt_val.as.string) str_release(vm_get_str(vm), fmt_val.as.string);
        return;
    }

    int mask_idx = 0;
    bool last_was_sep = false;
    BValue val_none;
    runtime_memset(&val_none, 0, sizeof(val_none));
    val_none.type = VAL_NUMBER;
    val_none.as.number = 0.0;

    while (true) {
        tok = lex_peek(lex);
        if (tok.type == TOK_EOL || tok.type == TOK_EOF || (tok.type == TOK_KEYWORD && tok.as.keyword == KW_ELSE)) {
            break;
        }

        // 3. Output literal tokens up to next format specifier
        while (mask_idx < mask.token_count) {
            UsingTokenType t_type = mask.tokens[mask_idx].type;
            if (t_type == USING_TOK_LITERAL || t_type == USING_TOK_ATTR || 
                t_type == USING_TOK_FORM_FEED || t_type == USING_TOK_LINE_FEED || 
                t_type == USING_TOK_BELL) {
                
                char out_buf[256];
                using_format_output(vm, &mask, &mask_idx, val_none, out_buf, sizeof(out_buf));
                if (stream) {
                    platform_file_printf(stream, "%s", out_buf);
                } else if (channel == PRINT_CHANNEL_CONSOLE) {
                    vdev_puts(vm_get_vdev(vm), out_buf);
                } else if (channel == PRINT_CHANNEL_PRINTER) {
                    vprinter_write_str(out_buf);
                } else {
                    file_puts(vm_get_file(vm), channel, out_buf);
                }
            } else {
                break;
            }
        }

        // Wrap mask if exhausted
        if (mask_idx >= mask.token_count) {
            mask_idx = 0;
            while (mask_idx < mask.token_count) {
                UsingTokenType t_type = mask.tokens[mask_idx].type;
                if (t_type == USING_TOK_LITERAL || t_type == USING_TOK_ATTR || 
                    t_type == USING_TOK_FORM_FEED || t_type == USING_TOK_LINE_FEED || 
                    t_type == USING_TOK_BELL) {
                    
                    char out_buf[256];
                    using_format_output(vm, &mask, &mask_idx, val_none, out_buf, sizeof(out_buf));
                    if (stream) {
                        platform_file_printf(stream, "%s", out_buf);
                    } else if (channel == PRINT_CHANNEL_CONSOLE) {
                        vdev_puts(vm_get_vdev(vm), out_buf);
                    } else if (channel == PRINT_CHANNEL_PRINTER) {
                        vprinter_write_str(out_buf);
                    } else {
                        file_puts(vm_get_file(vm), channel, out_buf);
                    }
                } else {
                    break;
                }
            }
        }

        // Format variable value using the specifier token
        char out_buf[256];
        out_buf[0] = '\0';
        BValue val = eval_expression(vm, lex, &err);
        if (err.code != 0) {
            vm_set_error(vm, err.code, err.message);
            str_release(vm_get_str(vm), fmt_val.as.string);
            return;
        }

        using_format_output(vm, &mask, &mask_idx, val, out_buf, sizeof(out_buf));

        if (stream) {
            platform_file_printf(stream, "%s", out_buf);
        } else if (channel == PRINT_CHANNEL_CONSOLE) {
            vdev_puts(vm_get_vdev(vm), out_buf);
        } else if (channel == PRINT_CHANNEL_PRINTER) {
            vprinter_write_str(out_buf);
        } else {
            file_puts(vm_get_file(vm), channel, out_buf);
        }

        if (val.type == VAL_STRING && val.as.string) {
            str_release(vm_get_str(vm), val.as.string);
        }

        // Check for comma/semicolon separator
        BppToken sep = lex_peek(lex);
        if (sep.type == TOK_COMMA || sep.type == TOK_SEMICOLON) {
            lex_next(lex); // consume separator
            last_was_sep = true;
            continue;
        } else {
            last_was_sep = false;
            break;
        }
    }

    // Flush any trailing literal tokens from mask
    while (mask_idx < mask.token_count) {
        UsingTokenType t_type = mask.tokens[mask_idx].type;
        if (t_type == USING_TOK_LITERAL || t_type == USING_TOK_ATTR || 
            t_type == USING_TOK_FORM_FEED || t_type == USING_TOK_LINE_FEED || 
            t_type == USING_TOK_BELL) {
            
            char out_buf[256];
            using_format_output(vm, &mask, &mask_idx, val_none, out_buf, sizeof(out_buf));
            if (stream) {
                platform_file_printf(stream, "%s", out_buf);
            } else if (channel == PRINT_CHANNEL_CONSOLE) {
                vdev_puts(vm_get_vdev(vm), out_buf);
            } else if (channel == PRINT_CHANNEL_PRINTER) {
                vprinter_write_str(out_buf);
            } else {
                file_puts(vm_get_file(vm), channel, out_buf);
            }
        } else {
            break;
        }
    }

    if (!last_was_sep) {
        if (stream) {
            platform_file_printf(stream, "\n");
        } else if (channel == PRINT_CHANNEL_CONSOLE) {
            vdev_putc(vm_get_vdev(vm), '\n');
        } else if (channel == PRINT_CHANNEL_PRINTER) {
            vprinter_write_str("\n");
        } else {
            file_putc(vm_get_file(vm), channel, '\n');
        }
    }
    if (stream) {
        platform_file_flush(stream);
    } else if (channel >= 0) {
        file_flush(vm_get_file(vm), channel);
    }
    if (fmt_val.type == VAL_STRING && fmt_val.as.string) {
        str_release(vm_get_str(vm), fmt_val.as.string);
    }
}

void print_using_internal(VMContext *vm, LexerContext *lex, int channel) {
    print_using_internal_ex(vm, lex, channel, NULL);
}

BppError stmt_print_using_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    print_using_internal(vm, lex, -1);
    return err;
}
