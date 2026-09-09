// FILENAME: print_pos.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (print.c)
// NEEDS: libcore (snprintf.h, strops.h, memops.h)
// NEEDS: libengine (eval.h, stmt.h, print_pos.h)
// Implements 2D grid and 1D buffer cursor positioning for PRINT AT and PRINT @.
//
// ---- Includes ----

#include "stmt/print_pos.h"
#include "stmt/stmt.h"
#include "device/vdev.h"
#include "device/vcon.h"
#include "device/bgi.h"
#include "eval/eval.h"
#include "runtime/variables.h"
#include "runtime/ctype/ctype.h"
#include "runtime/format/snprintf.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/conv/num_parse.h"
#include "runtime/conv/float_parse.h"
#include "runtime/language_descriptor.h"
#include "platform/platform.h"

static const LangDesc g_print_at_desc = {
    .name = "PRINT AT",
    .category = "Console and Terminal I/O",
    .syntax = "PRINT AT(x, y) [expr] | PRINT @pos [expr] | PRINT AT{x: col, y: row} [expr]",
    .description = "Positions cursor at 2D grid coordinates or 1D buffer offset before printing.",
    .error_summary = "Error 2: Syntax Error, Error 5: Illegal Function Call",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_STATEMENT
};

static int hex_char_to_int(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return 0;
}

static int parse_color_name(const char *name) {
    if (!name || !*name) return -1;
    if (runtime_strcasecmp(name, "black") == 0) return 0;
    if (runtime_strcasecmp(name, "blue") == 0) return 1;
    if (runtime_strcasecmp(name, "green") == 0) return 2;
    if (runtime_strcasecmp(name, "cyan") == 0) return 3;
    if (runtime_strcasecmp(name, "red") == 0) return 4;
    if (runtime_strcasecmp(name, "magenta") == 0) return 5;
    if (runtime_strcasecmp(name, "brown") == 0) return 6;
    if (runtime_strcasecmp(name, "white") == 0 || runtime_strcasecmp(name, "lightgray") == 0) return 7;
    if (runtime_strcasecmp(name, "darkgray") == 0 || runtime_strcasecmp(name, "gray") == 0) return 8;
    if (runtime_strcasecmp(name, "lightblue") == 0) return 9;
    if (runtime_strcasecmp(name, "lightgreen") == 0) return 10;
    if (runtime_strcasecmp(name, "lightcyan") == 0) return 11;
    if (runtime_strcasecmp(name, "lightred") == 0) return 12;
    if (runtime_strcasecmp(name, "lightmagenta") == 0) return 13;
    if (runtime_strcasecmp(name, "yellow") == 0) return 14;
    if (runtime_strcasecmp(name, "brightwhite") == 0) return 15;
    return -1;
}

static void apply_print_colors(VConContext *vcon, VDevContext *vdev, int fg, int bg, const char *hex_fg, const char *hex_bg, int style) {
    static const int ansi_fg_map[16] = {30, 34, 32, 36, 31, 35, 33, 37, 90, 94, 92, 96, 91, 95, 93, 97};
    static const int ansi_bg_map[16] = {40, 44, 42, 46, 41, 45, 43, 47, 100, 104, 102, 106, 101, 105, 103, 107};

    if (vcon && (fg >= 0 || bg >= 0)) {
        vcon_set_color(vcon, 0, fg, bg);
    }
    if (vdev) {
        if (style == 1) vdev_printf(vdev, "\033[1m");
        else if (style == 2) vdev_printf(vdev, "\033[3m");
        else if (style == 3) vdev_printf(vdev, "\033[4m");
        else if (style == 4) vdev_printf(vdev, "\033[5m");
        else if (style == 5) vdev_printf(vdev, "\033[7m");
        else if (style == 0) vdev_printf(vdev, "\033[0m");

        if (fg >= 0 && fg <= 15) {
            vdev_printf(vdev, "\033[%dm", ansi_fg_map[fg]);
        } else if (fg > 15 && fg <= 255) {
            vdev_printf(vdev, "\033[38;5;%dm", fg);
        } else if (hex_fg && hex_fg[0] == '#' && runtime_strlen(hex_fg) >= 7) {
            int r = (hex_char_to_int(hex_fg[1]) << 4) | hex_char_to_int(hex_fg[2]);
            int g = (hex_char_to_int(hex_fg[3]) << 4) | hex_char_to_int(hex_fg[4]);
            int b = (hex_char_to_int(hex_fg[5]) << 4) | hex_char_to_int(hex_fg[6]);
            vdev_printf(vdev, "\033[38;2;%d;%d;%dm", r, g, b);
        }

        if (bg >= 0 && bg <= 15) {
            vdev_printf(vdev, "\033[%dm", ansi_bg_map[bg]);
        } else if (bg > 15 && bg <= 255) {
            vdev_printf(vdev, "\033[48;5;%dm", bg);
        } else if (hex_bg && hex_bg[0] == '#' && runtime_strlen(hex_bg) >= 7) {
            int r = (hex_char_to_int(hex_bg[1]) << 4) | hex_char_to_int(hex_bg[2]);
            int g = (hex_char_to_int(hex_bg[3]) << 4) | hex_char_to_int(hex_bg[4]);
            int b = (hex_char_to_int(hex_bg[5]) << 4) | hex_char_to_int(hex_bg[6]);
            vdev_printf(vdev, "\033[48;2;%d;%d;%dm", r, g, b);
        }
    }
}

BppError parse_print_at_modifier(VMContext *vm, LexerContext *lex, size_t *col, bool *last_was_sep) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    VConContext *vcon = vm_get_vcon(vm);
    VDevContext *vdev = vm_get_vdev(vm);
    int cur_r = 0, cur_c = 0;
    if (vcon) {
        vcon_get_cursor(vcon, 0, &cur_r, &cur_c);
    } else {
        cur_c = (int)*col;
    }

    BppToken open_tok = lex_peek(lex);

    if (open_tok.type == TOK_LBRACE || open_tok.type == TOK_RPN_LITERAL) {
        char map_buf[512] = {0};
        if (open_tok.type == TOK_RPN_LITERAL) {
            lex_next(lex); // Consume TOK_RPN_LITERAL
            size_t mlen = open_tok.length < sizeof(map_buf) - 1 ? open_tok.length : sizeof(map_buf) - 1;
            runtime_memcpy(map_buf, open_tok.start, mlen);
            map_buf[mlen] = '\0';
        } else {
            lex_next(lex); // Consume '{'
            size_t mlen = 0;
            while (lex_peek(lex).type != TOK_RBRACE && lex_peek(lex).type != TOK_EOF && lex_peek(lex).type != TOK_EOL) {
                BppToken t = lex_next(lex);
                if (t.type == TOK_RBRACE) break;
                if (mlen + t.length + 1 < sizeof(map_buf)) {
                    runtime_memcpy(map_buf + mlen, t.start, t.length);
                    mlen += t.length;
                    map_buf[mlen++] = ' ';
                }
            }
            if (lex_peek(lex).type == TOK_RBRACE) {
                lex_next(lex);
            }
            map_buf[mlen] = '\0';
        }

        int x = -1, y = -1, z = -1;
        bool is_rel_x = false, is_rel_y = false, is_rel_z = false;
        int fg = -1, bg = -1;
        int base = 0;
        int width = 80;
        bool is_pixel = false;
        int style = -1;
        int font_id = -1, font_size = -1, font_dir = -1;
        char hex_fg[16] = {0};
        char hex_bg[16] = {0};

        const char *p = map_buf;
        while (*p) {
            while (*p == ' ' || *p == '\t' || *p == ',' || *p == ';' || *p == '\r' || *p == '\n') p++;
            if (!*p || *p == '}') break;

            const char *k_start = p;
            while (runtime_isalnum((unsigned char)*p) || *p == '_') p++;
            size_t k_len = (size_t)(p - k_start);
            if (k_len == 0) {
                p++;
                continue;
            }
            char k_str[32] = {0};
            runtime_snprintf(k_str, sizeof(k_str), "%.*s", (int)(k_len < 31 ? k_len : 31), k_start);

            while (*p == ' ' || *p == '\t') p++;
            if (*p == ':' || *p == '=') {
                p++;
                while (*p == ' ' || *p == '\t') p++;
            }

            char v_str[64] = {0};
            bool has_sign = (*p == '+' || *p == '-');
            if (*p == '"' || *p == '\'') {
                char quote = *p++;
                const char *v_start = p;
                while (*p && *p != quote && *p != '\n') p++;
                size_t v_len = (size_t)(p - v_start);
                if (*p == quote) p++;
                runtime_snprintf(v_str, sizeof(v_str), "%.*s", (int)(v_len < 63 ? v_len : 63), v_start);
            } else {
                const char *v_start = p;
                while (*p && *p != ',' && *p != ';' && *p != ' ' && *p != '\t' && *p != '}' && *p != '\r' && *p != '\n') p++;
                size_t v_len = (size_t)(p - v_start);
                runtime_snprintf(v_str, sizeof(v_str), "%.*s", (int)(v_len < 63 ? v_len : 63), v_start);
            }

            double num = 0.0;
            bool is_num = false;
            char *endptr = NULL;
            num = runtime_strtod(v_str, &endptr);
            if (endptr && endptr != v_str && *endptr == '\0') {
                is_num = true;
            } else {
                BValue *v = var_lookup(vm_get_var(vm), v_str, false);
                if (v) {
                    if (v->type == VAL_NUMBER) {
                        num = v->as.number;
                        is_num = true;
                    } else if (v->type == VAL_STRING && v->as.string) {
                        runtime_snprintf(v_str, sizeof(v_str), "%s", str_data(v->as.string));
                    }
                }
            }

            if (runtime_strcasecmp(k_str, "x") == 0 || runtime_strcasecmp(k_str, "col") == 0 || runtime_strcasecmp(k_str, "column") == 0) {
                is_rel_x = has_sign || (v_str[0] == '+' || v_str[0] == '-');
                x = is_num ? (int)num : runtime_atoi(v_str);
            } else if (runtime_strcasecmp(k_str, "y") == 0 || runtime_strcasecmp(k_str, "row") == 0) {
                is_rel_y = has_sign || (v_str[0] == '+' || v_str[0] == '-');
                y = is_num ? (int)num : runtime_atoi(v_str);
            } else if (runtime_strcasecmp(k_str, "z") == 0 || runtime_strcasecmp(k_str, "pos") == 0) {
                is_rel_z = has_sign || (v_str[0] == '+' || v_str[0] == '-');
                z = is_num ? (int)num : runtime_atoi(v_str);
            } else if (runtime_strcasecmp(k_str, "unit") == 0 || runtime_strcasecmp(k_str, "u") == 0) {
                if (runtime_strcasestr(v_str, "pixel") || runtime_strcasestr(v_str, "px")) is_pixel = true;
            } else if (runtime_strcasecmp(k_str, "base") == 0) {
                base = is_num ? (int)num : runtime_atoi(v_str);
            } else if (runtime_strcasecmp(k_str, "width") == 0 || runtime_strcasecmp(k_str, "w") == 0) {
                width = is_num ? (int)num : runtime_atoi(v_str);
                if (width <= 0) width = 80;
            } else if (runtime_strcasecmp(k_str, "fg") == 0 || runtime_strcasecmp(k_str, "color") == 0 || runtime_strcasecmp(k_str, "c") == 0) {
                if (is_num) {
                    fg = (int)num;
                } else if (v_str[0] == '#') {
                    runtime_snprintf(hex_fg, sizeof(hex_fg), "%s", v_str);
                } else {
                    fg = parse_color_name(v_str);
                }
            } else if (runtime_strcasecmp(k_str, "bg") == 0 || runtime_strcasecmp(k_str, "b") == 0) {
                if (is_num) {
                    bg = (int)num;
                } else if (v_str[0] == '#') {
                    runtime_snprintf(hex_bg, sizeof(hex_bg), "%s", v_str);
                } else {
                    bg = parse_color_name(v_str);
                }
            } else if (runtime_strcasecmp(k_str, "style") == 0) {
                if (runtime_strcasestr(v_str, "bold")) style = 1;
                else if (runtime_strcasestr(v_str, "italic")) style = 2;
                else if (runtime_strcasestr(v_str, "underline")) style = 3;
                else if (runtime_strcasestr(v_str, "blink")) style = 4;
                else if (runtime_strcasestr(v_str, "inverse") || runtime_strcasestr(v_str, "reverse")) style = 5;
                else if (runtime_strcasestr(v_str, "normal") || runtime_strcasestr(v_str, "reset")) style = 0;
                else if (is_num) style = (int)num;
            } else if (runtime_strcasecmp(k_str, "font") == 0 || runtime_strcasecmp(k_str, "f") == 0) {
                if (runtime_strcasestr(v_str, "triplex")) font_id = 1;
                else if (runtime_strcasestr(v_str, "small")) font_id = 2;
                else if (runtime_strcasestr(v_str, "sans")) font_id = 3;
                else if (runtime_strcasestr(v_str, "gothic")) font_id = 4;
                else if (is_num) font_id = (int)num;
                else font_id = 0;
            } else if (runtime_strcasecmp(k_str, "size") == 0 || runtime_strcasecmp(k_str, "s") == 0) {
                font_size = is_num ? (int)num : runtime_atoi(v_str);
            } else if (runtime_strcasecmp(k_str, "direction") == 0 || runtime_strcasecmp(k_str, "dir") == 0) {
                font_dir = is_num ? (int)num : runtime_atoi(v_str);
            }
        }

        if (z >= 0 || is_rel_z) {
            int target_z = is_rel_z ? (cur_r * width + cur_c + z) : z;
            if (target_z < 0) target_z = 0;
            y = target_z / width;
            x = target_z % width;
            is_rel_x = false;
            is_rel_y = false;
        }

        int target_x = cur_c;
        int target_y = cur_r;
        if (x >= 0 || is_rel_x) {
            target_x = is_rel_x ? (cur_c + x) : (x - base);
        }
        if (y >= 0 || is_rel_y) {
            target_y = is_rel_y ? (cur_r + y) : (y - base);
        }
        if (target_x < 0) target_x = 0;
        if (target_y < 0) target_y = 0;

        if (is_pixel) {
            BGI_Context *bgi_ctx = BGI_get_global_context();
            if (bgi_ctx && bgi_ctx->initialized && bgi_ctx->framebuffer) {
                BGI_moveto(bgi_ctx, target_x, target_y);
                if (fg >= 0) bgi_ctx->fg_color = fg;
                if (bg >= 0) bgi_ctx->bg_color = bg;
                if (font_id >= 0 || font_size >= 1 || font_dir >= 0) {
                    int fid = (font_id >= 0) ? font_id : 0;
                    int fdir = (font_dir >= 0) ? font_dir : 0;
                    int fsz = (font_size >= 1) ? font_size : 1;
                    BGI_settextstyle(bgi_ctx, fid, fdir, fsz);
                }
            } else {
                int cell_x = target_x / 8;
                int cell_y = target_y / 16;
                if (vcon) vcon_locate(vcon, 0, cell_y + 1, cell_x + 1);
                if (vdev) vdev_printf(vdev, "\033[%d;%dH", cell_y + 1, cell_x + 1);
                *col = (size_t)cell_x;
            }
        } else {
            if (vcon) vcon_locate(vcon, 0, target_y + 1, target_x + 1);
            if (vdev) vdev_printf(vdev, "\033[%d;%dH", target_y + 1, target_x + 1);
            *col = (size_t)target_x;
        }

        if (fg >= 0 || bg >= 0 || hex_fg[0] != '\0' || hex_bg[0] != '\0' || style >= 0) {
            apply_print_colors(vcon, vdev, fg, bg, hex_fg[0] ? hex_fg : NULL, hex_bg[0] ? hex_bg : NULL, style);
        }

        BppToken aft = lex_peek(lex);
        if (aft.type == TOK_SEMICOLON) {
            lex_next(lex);
            *last_was_sep = true;
        } else if (aft.type == TOK_COMMA) {
            lex_next(lex);
            *last_was_sep = true;
        } else if (aft.type == TOK_EOL || aft.type == TOK_EOF || aft.type == TOK_BACKSLASH ||
                   (aft.type == TOK_KEYWORD && aft.as.keyword == KW_ELSE)) {
            *last_was_sep = true;
        }
        return err;

    } else {
        // Parenthesized `(...)`, Bracketed `[...]`, or Bare `x, y`
        BppTokenType closing_type = TOK_EOF;
        bool is_pixel = false;
        if (open_tok.type == TOK_LPAREN) {
            lex_next(lex);
            closing_type = TOK_RPAREN;
        } else if (open_tok.type == TOK_LBRACKET) {
            lex_next(lex);
            closing_type = TOK_RBRACKET;
            is_pixel = true;
        }

        bool is_rel_x = (lex_peek(lex).type == TOK_PLUS || lex_peek(lex).type == TOK_MINUS);
        BValue vx = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        int x = (int)vx.as.number;

        if (lex_peek(lex).type != TOK_COMMA) {
            err.code = 2;
            err.message = "Expected ',' after X coordinate in AT";
            return err;
        }
        lex_next(lex); // Consume ','

        bool is_rel_y = (lex_peek(lex).type == TOK_PLUS || lex_peek(lex).type == TOK_MINUS);
        BValue vy = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        int y = (int)vy.as.number;

        int fg = -1, bg = -1;

        if (closing_type != TOK_EOF) {
            if (lex_peek(lex).type == TOK_COMMA) {
                lex_next(lex); // Consume ','
                if (lex_peek(lex).type != closing_type) {
                    BValue vfg = eval_expression(vm, lex, &err);
                    if (err.code != 0) return err;
                    if (vfg.type == VAL_NUMBER) fg = (int)vfg.as.number;

                    if (lex_peek(lex).type == TOK_COMMA) {
                        lex_next(lex); // Consume ','
                        if (lex_peek(lex).type != closing_type) {
                            BValue vbg = eval_expression(vm, lex, &err);
                            if (err.code != 0) return err;
                            if (vbg.type == VAL_NUMBER) bg = (int)vbg.as.number;
                        }
                    }
                }
            }

            if (lex_peek(lex).type == closing_type) {
                lex_next(lex);
            } else {
                err.code = 2;
                err.message = (closing_type == TOK_RPAREN) ? "Expected ')' after AT arguments" : "Expected ']' after AT arguments";
                return err;
            }
        } else {
            // Bare AT x, y [, fg [, bg]]
            if (lex_peek(lex).type == TOK_COMMA) {
                const char *src = lex_get_pos(lex);
                if (src && *src == ',') {
                    src++;
                    while (*src == ' ' || *src == '\t') src++;
                    if ((*src >= '0' && *src <= '9') || *src == '+' || *src == '-') {
                        lex_next(lex); // Consume ','
                        BValue vfg = eval_expression(vm, lex, &err);
                        if (err.code != 0) { return err; }
                        fg = (int)vfg.as.number;
                        if (lex_peek(lex).type == TOK_COMMA) {
                            const char *src2 = lex_get_pos(lex);
                            if (src2 && *src2 == ',') {
                                src2++;
                                while (*src2 == ' ' || *src2 == '\t') src2++;
                                if ((*src2 >= '0' && *src2 <= '9') || *src2 == '+' || *src2 == '-') {
                                    lex_next(lex); // Consume ','
                                    BValue vbg = eval_expression(vm, lex, &err);
                                    if (err.code != 0) { return err; }
                                    bg = (int)vbg.as.number;
                                }
                            }
                        }
                    }
                }
            }
        }

        int target_x = is_rel_x ? (cur_c + x) : x;
        int target_y = is_rel_y ? (cur_r + y) : y;
        if (target_x < 0) target_x = 0;
        if (target_y < 0) target_y = 0;

        if (is_pixel) {
            BGI_Context *bgi_ctx = BGI_get_global_context();
            if (bgi_ctx && bgi_ctx->initialized && bgi_ctx->framebuffer) {
                BGI_moveto(bgi_ctx, target_x, target_y);
                if (fg >= 0) bgi_ctx->fg_color = fg;
                if (bg >= 0) bgi_ctx->bg_color = bg;
            } else {
                int cell_x = target_x / 8;
                int cell_y = target_y / 16;
                if (vcon) vcon_locate(vcon, 0, cell_y + 1, cell_x + 1);
                if (vdev) vdev_printf(vdev, "\033[%d;%dH", cell_y + 1, cell_x + 1);
                *col = (size_t)cell_x;
            }
        } else {
            if (vcon) vcon_locate(vcon, 0, target_y + 1, target_x + 1);
            if (vdev) vdev_printf(vdev, "\033[%d;%dH", target_y + 1, target_x + 1);
            *col = (size_t)target_x;
        }

        if (fg >= 0 || bg >= 0) {
            apply_print_colors(vcon, vdev, fg, bg, NULL, NULL, -1);
        }
    }

    BppToken aft = lex_peek(lex);
    if (aft.type == TOK_SEMICOLON) {
        lex_next(lex);
        *last_was_sep = true;
    } else if (aft.type == TOK_COMMA) {
        lex_next(lex);
        *last_was_sep = true;
    } else if (aft.type == TOK_EOL || aft.type == TOK_EOF || aft.type == TOK_BACKSLASH ||
               (aft.type == TOK_KEYWORD && aft.as.keyword == KW_ELSE)) {
        *last_was_sep = true;
    }

    return err;
}

BppError parse_print_at_sign_modifier(VMContext *vm, LexerContext *lex, size_t *col, bool *last_was_sep) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    VConContext *vcon = vm_get_vcon(vm);
    VDevContext *vdev = vm_get_vdev(vm);
    int cur_r = 0, cur_c = 0;
    if (vcon) {
        vcon_get_cursor(vcon, 0, &cur_r, &cur_c);
    } else {
        cur_c = (int)*col;
    }

    BppToken open_tok = lex_peek(lex);

    if (open_tok.type == TOK_LBRACE || open_tok.type == TOK_RPN_LITERAL) {
        return parse_print_at_modifier(vm, lex, col, last_was_sep);
    }

    BppTokenType closing_type = TOK_EOF;
    bool is_vram = false;
    if (open_tok.type == TOK_LPAREN) {
        lex_next(lex);
        closing_type = TOK_RPAREN;
    } else if (open_tok.type == TOK_LBRACKET) {
        lex_next(lex);
        closing_type = TOK_RBRACKET;
        is_vram = true;
    }

    bool is_rel_z = (lex_peek(lex).type == TOK_PLUS || lex_peek(lex).type == TOK_MINUS);
    BValue vz = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    int z = (int)vz.as.number;

    // Check for Business BASIC / Apple III screen formatting mnemonics:
    // @(-1) = Clear screen (CLS)
    // @(-2) = Home cursor (HOME)
    // @(-3) = Clear to end of line (EOL)
    // @(-4) = Clear to end of screen (EOS)
    if (closing_type != TOK_EOF && z < 0 && lex_peek(lex).type != TOK_COMMA) {
        if (lex_peek(lex).type == closing_type) {
            lex_next(lex);
        } else {
            err.code = 2;
            err.message = (closing_type == TOK_RPAREN) ? "Expected ')' after @ argument" : "Expected ']' after @ argument";
            return err;
        }

        if (z == -1) {
            // CLS
            if (vcon) {
                vcon_clear_screen(vcon, 0, -3);
                vcon_locate(vcon, 0, 1, 1);
            }
            if (vdev) vdev_printf(vdev, "\033[2J\033[H");
            *col = 0;
        } else if (z == -2) {
            // HOME
            if (vcon) vcon_locate(vcon, 0, 1, 1);
            if (vdev) vdev_printf(vdev, "\033[H");
            *col = 0;
        } else if (z == -3) {
            // Clear to End of Line
            if (vdev) vdev_printf(vdev, "\033[K");
        } else if (z == -4) {
            // Clear to End of Screen
            if (vdev) vdev_printf(vdev, "\033[J");
        }

        BppToken aft = lex_peek(lex);
        if (aft.type == TOK_SEMICOLON || aft.type == TOK_COMMA) {
            lex_next(lex);
            *last_was_sep = true;
        } else if (aft.type == TOK_EOL || aft.type == TOK_EOF || aft.type == TOK_BACKSLASH ||
                   (aft.type == TOK_KEYWORD && aft.as.keyword == KW_ELSE)) {
            *last_was_sep = true;
        }
        return err;
    }

    // Check for Business BASIC / Apple III 2D coordinates: @(col, row [, fg]) or @[col, row]
    bool is_2d_coord = false;
    int target_row = 0;
    int fg = -1;

    if (closing_type != TOK_EOF) {
        if (lex_peek(lex).type == TOK_COMMA) {
            lex_next(lex);
            if (lex_peek(lex).type != closing_type) {
                BValue vc = eval_expression(vm, lex, &err);
                if (err.code != 0) return err;
                if (vc.type == VAL_NUMBER || vc.type == VAL_INTEGER) {
                    target_row = (int)vc.as.number;
                    is_2d_coord = true;
                }
            }
            if (lex_peek(lex).type == TOK_COMMA) {
                lex_next(lex);
                if (lex_peek(lex).type != closing_type) {
                    BValue vfg = eval_expression(vm, lex, &err);
                    if (err.code != 0) return err;
                    if (vfg.type == VAL_NUMBER || vfg.type == VAL_INTEGER) {
                        fg = (int)vfg.as.number;
                    }
                }
            }
        }
        if (lex_peek(lex).type == closing_type) {
            lex_next(lex);
        } else {
            err.code = 2;
            err.message = (closing_type == TOK_RPAREN) ? "Expected ')' after @ argument" : "Expected ']' after @ argument";
            return err;
        }
    } else {
        // Bare @z [, color]
        if (lex_peek(lex).type == TOK_COMMA) {
            const char *src = lex_get_pos(lex);
            if (src && *src == ',') {
                src++;
                while (*src == ' ' || *src == '\t') src++;
                if ((*src >= '0' && *src <= '9') || *src == '+' || *src == '-') {
                    lex_next(lex); // Consume ','
                    BValue vc = eval_expression(vm, lex, &err);
                    if (err.code != 0) { return err; }
                    fg = (int)vc.as.number;
                }
            }
        }
    }

    int target_x = 0, target_y = 0;
    if (is_2d_coord) {
        target_x = z;
        target_y = target_row;
    } else {
        int W = 80;
        if (is_vram) {
            if (z >= 15360 && z < 16384) {
                z -= 15360;
                W = 64;
            } else if (z >= 1024 && z < 1536) {
                z -= 1024;
                W = 32;
            } else if (z >= 0xB8000 && z < 0xBC000) {
                z = (z - 0xB8000) / 2;
                W = 80;
            } else if (z >= 0xB0000 && z < 0xB4000) {
                z = (z - 0xB0000) / 2;
                W = 80;
            }
        }

        int target_z = is_rel_z ? (cur_r * W + cur_c + z) : z;
        if (target_z < 0) target_z = 0;

        target_y = target_z / W;
        target_x = target_z % W;
    }

    if (vcon) vcon_locate(vcon, 0, target_y + 1, target_x + 1);
    if (vdev) vdev_printf(vdev, "\033[%d;%dH", target_y + 1, target_x + 1);
    *col = (size_t)target_x;

    if (fg >= 0) {
        apply_print_colors(vcon, vdev, fg, -1, NULL, NULL, -1);
    }

    BppToken aft = lex_peek(lex);
    if (aft.type == TOK_SEMICOLON) {
        lex_next(lex);
        *last_was_sep = true;
    } else if (aft.type == TOK_COMMA) {
        lex_next(lex);
        *last_was_sep = true;
    } else if (aft.type == TOK_EOL || aft.type == TOK_EOF || aft.type == TOK_BACKSLASH ||
               (aft.type == TOK_KEYWORD && aft.as.keyword == KW_ELSE)) {
        *last_was_sep = true;
    }

    return err;
}

void stmt_print_at_register(void) {
    lang_desc_register(&g_print_at_desc);
}
