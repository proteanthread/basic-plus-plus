// FILENAME: bppc_stmt.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (bppc_transpile.c)
// NEEDS: libcore (bppc_internal.h), libplatform
// C statement generator for bppc native transpiler.
//
// ---- Includes ----

#include "tools/bppc_internal.h"
#include "tools/bppc_freestanding.h"
#include "platform/platform.h"
#include "runtime/format/snprintf.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"
#include "runtime/ctype/ctype.h"
#include "runtime/memory/alloc.h"
#include "runtime/conv/num_parse.h"

#define fprintf(f, ...) platform_file_printf((void *)(f), __VA_ARGS__)

static char *bppc_find_unquoted_char(const char *s, char target) {
    if (!s) return NULL;
    bool in_quote = false;
    for (size_t i = 0; s[i]; i++) {
        if (s[i] == '"') {
            in_quote = !in_quote;
        } else if (s[i] == target && !in_quote) {
            return (char *)(s + i);
        }
    }
    return NULL;
}

void bppc_emit_print(void *out, const char *args) {
    if (bppc_is_freestanding_or_uefi()) {
        bppc_emit_freestanding_print(out, args, g_target_dialect == TARGET_UEFI);
        return;
    }
    char cur[256];
    int cur_len = 0;
    bool in_str = false;

    // Check for PRINT #channel, ...
    const char *p = bppc_trim((char *)args);
    int channel = 0;
    if (*p == '#') {
        p++;
        channel = (int)runtime_strtol(p, (char **)&p, 10);
        while (*p && (*p == ',' || runtime_isspace((unsigned char)*p))) p++;
    }

    for (int i = 0; ; i++) {
        char c = p[i];
        if (c == '"') {
            in_str = !in_str;
            if (cur_len < (int)sizeof(cur) - 2) cur[cur_len++] = c;
        } else if (!in_str && (c == ';' || c == ',' || c == '\0')) {
            cur[cur_len] = '\0';
            char *trimmed = bppc_trim(cur);
            if (*trimmed != '\0') {
                if (*trimmed == '"') {
                    char text[256] = {0};
                    size_t tlen = runtime_strlen(trimmed);
                    if (tlen >= 2 && trimmed[tlen - 1] == '"') {
                        runtime_memcpy(text, trimmed + 1, tlen - 2);
                        text[tlen - 2] = '\0';
                    } else {
                        runtime_strcpy(text, trimmed + 1);
                    }
                    if (channel > 0) {
                        fprintf(out, "    if (bpp_files[%d]) fprintf(bpp_files[%d], \"%%s\", \"%s\");\n", channel, channel, text);
                    } else {
                        fprintf(out, "    printf(\"%%s\", \"%s\");\n", text);
                    }
                } else {
                    char c_expr[256] = {0};
                    bppc_convert_expression_to_c(trimmed, c_expr, sizeof(c_expr));
                    if (runtime_strstr(trimmed, "$")) {
                        if (channel > 0) {
                            fprintf(out, "    if (bpp_files[%d]) fprintf(bpp_files[%d], \"%%s\", %s);\n", channel, channel, c_expr);
                        } else {
                            fprintf(out, "    printf(\"%%s\", %s);\n", c_expr);
                        }
                    } else {
                        if (channel > 0) {
                            fprintf(out, "    if (bpp_files[%d]) fprintf(bpp_files[%d], \"%%g\", (double)(%s));\n", channel, channel, c_expr);
                        } else {
                            fprintf(out, "    printf(\"%%g\", (double)(%s));\n", c_expr);
                        }
                    }
                }
            }
            if (c == ',') {
                if (channel > 0) {
                    fprintf(out, "    if (bpp_files[%d]) fprintf(bpp_files[%d], \"\\t\");\n", channel, channel);
                } else {
                    fprintf(out, "    printf(\"\\t\");\n");
                }
            }
            cur_len = 0;
            if (c == '\0') {
                if (channel > 0) {
                    fprintf(out, "    if (bpp_files[%d]) fprintf(bpp_files[%d], \"\\n\");\n", channel, channel);
                } else {
                    fprintf(out, "    printf(\"\\n\");\n");
                }
                break;
            }
        } else {
            if (cur_len < (int)sizeof(cur) - 2) {
                cur[cur_len++] = c;
            }
        }
    }
}

static bool bppc_emit_systems_stmt(void *out, const char *s) {
    if (runtime_strcasecmp(s, "CLS") == 0) {
        if (bppc_is_freestanding_or_uefi()) {
            if (g_target_dialect == TARGET_UEFI) {
                fprintf(out, "    bpp_uefi_cls();\n");
            } else {
                fprintf(out, "    // CLS (bare metal)\n");
            }
        } else {
            fprintf(out, "    bpp_rt_cls();\n");
        }
        return true;
    }
    if (runtime_strncasecmp(s, "POKE ", 5) == 0) {
        char *comma = runtime_strchr((char *)s + 5, ',');
        if (comma) {
            *comma = '\0';
            char a_c[128] = {0}, v_c[128] = {0};
            bppc_convert_expression_to_c(bppc_trim((char *)s + 5), a_c, sizeof(a_c));
            bppc_convert_expression_to_c(bppc_trim(comma + 1), v_c, sizeof(v_c));
            if (bppc_is_freestanding_or_uefi()) {
                fprintf(out, "    BPP_MEM8(%s) = (uint8_t)(%s);\n", a_c, v_c);
            } else {
                fprintf(out, "    bpp_rt_mem[(int)(%s) & 0xFFFF] = (unsigned char)(%s);\n", a_c, v_c);
            }
        }
        return true;
    }
    if (runtime_strncasecmp(s, "OUT ", 4) == 0) {
        char *comma = runtime_strchr((char *)s + 4, ',');
        if (comma) {
            *comma = '\0';
            char p_c[128] = {0}, v_c[128] = {0};
            bppc_convert_expression_to_c(bppc_trim((char *)s + 4), p_c, sizeof(p_c));
            bppc_convert_expression_to_c(bppc_trim(comma + 1), v_c, sizeof(v_c));
            if (bppc_is_freestanding_or_uefi()) {
                fprintf(out, "    BPP_PORT8(%s) = (uint8_t)(%s);\n", p_c, v_c);
            } else {
                fprintf(out, "    // OUT %s, %s\n", p_c, v_c);
            }
        }
        return true;
    }
    if (runtime_strncasecmp(s, "MEM[", 4) == 0) {
        const char *rb = runtime_strchr(s, ']');
        if (rb) {
            char addr[128] = {0};
            runtime_memcpy(addr, s + 4, rb - (s + 4));
            const char *eq = runtime_strchr(rb, '=');
            if (eq) {
                char a_c[128] = {0}, v_c[128] = {0};
                bppc_convert_expression_to_c(bppc_trim(addr), a_c, sizeof(a_c));
                bppc_convert_expression_to_c(bppc_trim((char *)eq + 1), v_c, sizeof(v_c));
                fprintf(out, "    BPP_MEM8(%s) = (uint8_t)(%s);\n", a_c, v_c);
            }
        }
        return true;
    }
    if (runtime_strncasecmp(s, "MEMW[", 5) == 0) {
        const char *rb = runtime_strchr(s, ']');
        if (rb) {
            char addr[128] = {0};
            runtime_memcpy(addr, s + 5, rb - (s + 5));
            const char *eq = runtime_strchr(rb, '=');
            if (eq) {
                char a_c[128] = {0}, v_c[128] = {0};
                bppc_convert_expression_to_c(bppc_trim(addr), a_c, sizeof(a_c));
                bppc_convert_expression_to_c(bppc_trim((char *)eq + 1), v_c, sizeof(v_c));
                fprintf(out, "    BPP_MEM16(%s) = (uint16_t)(%s);\n", a_c, v_c);
            }
        }
        return true;
    }
    if (runtime_strncasecmp(s, "MEML[", 5) == 0) {
        const char *rb = runtime_strchr(s, ']');
        if (rb) {
            char addr[128] = {0};
            runtime_memcpy(addr, s + 5, rb - (s + 5));
            const char *eq = runtime_strchr(rb, '=');
            if (eq) {
                char a_c[128] = {0}, v_c[128] = {0};
                bppc_convert_expression_to_c(bppc_trim(addr), a_c, sizeof(a_c));
                bppc_convert_expression_to_c(bppc_trim((char *)eq + 1), v_c, sizeof(v_c));
                fprintf(out, "    BPP_MEM32(%s) = (uint32_t)(%s);\n", a_c, v_c);
            }
        }
        return true;
    }
    if (runtime_strncasecmp(s, "PORT[", 5) == 0) {
        const char *rb = runtime_strchr(s, ']');
        if (rb) {
            char port[128] = {0};
            runtime_memcpy(port, s + 5, rb - (s + 5));
            const char *eq = runtime_strchr(rb, '=');
            if (eq) {
                char p_c[128] = {0}, v_c[128] = {0};
                bppc_convert_expression_to_c(bppc_trim(port), p_c, sizeof(p_c));
                bppc_convert_expression_to_c(bppc_trim((char *)eq + 1), v_c, sizeof(v_c));
                fprintf(out, "    BPP_PORT8(%s) = (uint8_t)(%s);\n", p_c, v_c);
            }
        }
        return true;
    }
    if (runtime_strncasecmp(s, "ASM ", 4) == 0) {
        const char *code = bppc_trim((char *)s + 4);
        fprintf(out, "    __asm__ volatile (%s);\n", code);
        return true;
    }
    return false;
}

void bppc_emit_statement(void *out, const char *s, bool debug) {
    (void)debug;
    if (runtime_strncasecmp(s, "LET ", 4) == 0) {
        s = bppc_trim((char *)s + 4);
    }
    if (bppc_emit_systems_stmt(out, s)) return;

    if (runtime_strncasecmp(s, "DIM ", 4) == 0) {
        // Handled in Pass 1
    } else if (runtime_strncasecmp(s, "OPEN ", 5) == 0) {
        char open_buf[256] = {0};
        runtime_strncpy(open_buf, s + 5, sizeof(open_buf) - 1);
        char *for_p = runtime_strstr(open_buf, " FOR ");
        if (!for_p) for_p = runtime_strstr(open_buf, " for ");
        char *as_p = runtime_strstr(open_buf, " AS ");
        if (!as_p) as_p = runtime_strstr(open_buf, " as ");

        if (for_p && as_p) {
            *for_p = '\0';
            char fn_c[256] = {0};
            bppc_convert_expression_to_c(bppc_trim(open_buf), fn_c, sizeof(fn_c));
            
            char mode_str[32] = {0};
            size_t mlen = as_p - (for_p + 5);
            runtime_memcpy(mode_str, for_p + 5, mlen);
            mode_str[mlen] = '\0';
            
            char *hash_p = runtime_strchr(as_p, '#');
            int chan = hash_p ? runtime_atoi(hash_p + 1) : 1;
            bool is_out = (runtime_strncasecmp(bppc_trim(mode_str), "OUTPUT", 6) == 0);
            bool is_app = (runtime_strncasecmp(bppc_trim(mode_str), "APPEND", 6) == 0);
            bool is_rnd = (runtime_strncasecmp(bppc_trim(mode_str), "RANDOM", 6) == 0 || runtime_strncasecmp(bppc_trim(mode_str), "BINARY", 6) == 0);
            const char *fmode = is_out ? "wb" : (is_app ? "ab" : (is_rnd ? "r+b" : "rb"));
            fprintf(out, "    bpp_files[%d] = fopen(%s, \"%s\");\n", chan, fn_c, fmode);
        } else {
            // Short-form OPEN: OPEN "mode", [#]chan, "filename" [, reclen]
            char parts[4][128] = {{0}};
            int part_idx = 0;
            int cur_len = 0;
            bool in_quotes = false;
            for (int ci = 0; open_buf[ci] && part_idx < 4; ci++) {
                char c = open_buf[ci];
                if (c == '"') in_quotes = !in_quotes;
                if (c == ',' && !in_quotes) {
                    parts[part_idx][cur_len] = '\0';
                    part_idx++;
                    cur_len = 0;
                } else {
                    if (cur_len < (int)sizeof(parts[0]) - 2) {
                        parts[part_idx][cur_len++] = c;
                    }
                }
            }
            if (part_idx < 4) {
                parts[part_idx][cur_len] = '\0';
                part_idx++;
            }

            if (part_idx >= 3) {
                const char *fmode = "rb";
                if (runtime_strchr(parts[0], 'O') || runtime_strchr(parts[0], 'o')) fmode = "wb";
                else if (runtime_strchr(parts[0], 'A') || runtime_strchr(parts[0], 'a')) fmode = "ab";
                else if (runtime_strchr(parts[0], 'R') || runtime_strchr(parts[0], 'r') ||
                         runtime_strchr(parts[0], 'B') || runtime_strchr(parts[0], 'b')) fmode = "r+b";
                else if (runtime_strchr(parts[0], 'I') || runtime_strchr(parts[0], 'i')) fmode = "rb";

                char *ch_str = bppc_trim(parts[1]);
                if (*ch_str == '#') ch_str = bppc_trim(ch_str + 1);
                int chan = runtime_atoi(ch_str);
                if (chan <= 0) chan = 1;

                char fn_c[256] = {0};
                bppc_convert_expression_to_c(bppc_trim(parts[2]), fn_c, sizeof(fn_c));
                fprintf(out, "    bpp_files[%d] = fopen(%s, \"%s\");\n", chan, fn_c, fmode);
            }
        }
    } else if (runtime_strncasecmp(s, "CLOSE", 5) == 0) {
        char *hash_p = runtime_strchr((char *)s, '#');
        int chan = hash_p ? runtime_atoi(hash_p + 1) : 1;
        fprintf(out, "    if (bpp_files[%d]) { fclose(bpp_files[%d]); bpp_files[%d] = NULL; }\n", chan, chan, chan);
    } else if (runtime_strncasecmp(s, "KILL ", 5) == 0) {
        char fn_c[256] = {0};
        bppc_convert_expression_to_c(bppc_trim((char *)s + 5), fn_c, sizeof(fn_c));
        fprintf(out, "    remove(%s);\n", fn_c);
    } else if (runtime_strncasecmp(s, "LINE INPUT ", 11) == 0) {
        char *hash_p = runtime_strchr((char *)s, '#');
        if (hash_p) {
            int chan = runtime_atoi(hash_p + 1);
            char *comma = runtime_strchr(hash_p, ',');
            if (comma) {
                char vname[64] = {0};
                bppc_sanitize_ident(bppc_trim(comma + 1), vname, sizeof(vname));
                fprintf(out, "    if (bpp_files[%d]) { if (fgets(var_%s, sizeof(var_%s), bpp_files[%d])) { size_t __l = runtime_strlen(var_%s); if (__l > 0 && (var_%s[__l-1] == '\\n' || var_%s[__l-1] == '\\r')) var_%s[__l-1] = '\\0'; } }\n",
                        chan, vname, vname, chan, vname, vname, vname, vname);
            }
        }
    } else if (runtime_strncmp(s, "FOR ", 4) == 0 || runtime_strncmp(s, "for ", 4) == 0) {
        char var_name[64] = {0};
        char start_val[64] = {0};
        char end_val[64] = {0};
        char step_val[64] = "1";
        
        char *eq = runtime_strchr((char *)s, '=');
        char *to_p = runtime_strstr((char *)s, " TO ");
        if (!to_p) to_p = runtime_strstr((char *)s, " to ");
        char *step_p = runtime_strstr((char *)s, " STEP ");
        if (!step_p) step_p = runtime_strstr((char *)s, " step ");
        
        if (eq && to_p) {
            size_t nlen = eq - (s + 4);
            if (nlen < sizeof(var_name)) {
                runtime_memcpy(var_name, s + 4, nlen);
                var_name[nlen] = '\0';
            }
            char sanitized_var[64] = {0};
            bppc_sanitize_ident(bppc_trim(var_name), sanitized_var, sizeof(sanitized_var));
            
            size_t slen = to_p - (eq + 1);
            if (slen < sizeof(start_val)) {
                runtime_memcpy(start_val, eq + 1, slen);
                start_val[slen] = '\0';
            }
            
            if (step_p) {
                size_t elen = step_p - (to_p + 4);
                if (elen < sizeof(end_val)) {
                    runtime_memcpy(end_val, to_p + 4, elen);
                    end_val[elen] = '\0';
                }
                runtime_strncpy(step_val, step_p + 6, sizeof(step_val) - 1);
            } else {
                runtime_strncpy(end_val, to_p + 4, sizeof(end_val) - 1);
            }
            char c_start[128] = {0}, c_end[128] = {0}, c_step[128] = {0};
            bppc_convert_expression_to_c(bppc_trim(start_val), c_start, sizeof(c_start));
            bppc_convert_expression_to_c(bppc_trim(end_val), c_end, sizeof(c_end));
            bppc_convert_expression_to_c(bppc_trim(step_val), c_step, sizeof(c_step));

            char *trimmed_step = bppc_trim(step_val);
            if (runtime_strcmp(trimmed_step, "1") == 0) {
                fprintf(out, "    for (var_%s = (%s); var_%s <= (%s); var_%s++) {\n",
                        sanitized_var, c_start, sanitized_var, c_end, sanitized_var);
            } else {
                char *endptr = NULL;
                long step_lit = runtime_strtol(trimmed_step, &endptr, 10);
                if (endptr && *endptr == '\0') {
                    if (step_lit > 0) {
                        fprintf(out, "    for (var_%s = (%s); var_%s <= (%s); var_%s += %ld) {\n",
                                sanitized_var, c_start, sanitized_var, c_end, sanitized_var, step_lit);
                    } else if (step_lit < 0) {
                        fprintf(out, "    for (var_%s = (%s); var_%s >= (%s); var_%s += (%ld)) {\n",
                                sanitized_var, c_start, sanitized_var, c_end, sanitized_var, step_lit);
                    } else {
                        fprintf(out, "    for (var_%s = (%s); ((%s) >= 0 ? var_%s <= (%s) : var_%s >= (%s)); var_%s += (%s)) {\n",
                                sanitized_var, c_start, c_step, sanitized_var, c_end, sanitized_var, c_end, sanitized_var, c_step);
                    }
                } else {
                    fprintf(out, "    for (var_%s = (%s); ((%s) >= 0 ? var_%s <= (%s) : var_%s >= (%s)); var_%s += (%s)) {\n",
                            sanitized_var, c_start, c_step, sanitized_var, c_end, sanitized_var, c_end, sanitized_var, c_step);
                }
            }
        }
    } else if (runtime_strncmp(s, "NEXT", 4) == 0 || runtime_strncmp(s, "next", 4) == 0) {
        fprintf(out, "    }\n");
    } else if (runtime_strncmp(s, "WHILE ", 6) == 0 || runtime_strncmp(s, "while ", 6) == 0) {
        char c_cond[256] = {0};
        bppc_convert_expression_to_c(s + 6, c_cond, sizeof(c_cond));
        fprintf(out, "    while (%s) {\n", c_cond);
    } else if (runtime_strcmp(s, "WEND") == 0 || runtime_strcmp(s, "wend") == 0) {
        fprintf(out, "    }\n");
    } else if (runtime_strncmp(s, "DO WHILE ", 9) == 0 || runtime_strncmp(s, "do while ", 9) == 0) {
        char c_cond[256] = {0};
        bppc_convert_expression_to_c(s + 9, c_cond, sizeof(c_cond));
        fprintf(out, "    while (%s) {\n", c_cond);
    } else if (runtime_strcmp(s, "DO") == 0 || runtime_strcmp(s, "do") == 0) {
        fprintf(out, "    do {\n");
    } else if (runtime_strncmp(s, "LOOP WHILE ", 11) == 0 || runtime_strncmp(s, "loop while ", 11) == 0) {
        char c_cond[256] = {0};
        bppc_convert_expression_to_c(s + 11, c_cond, sizeof(c_cond));
        fprintf(out, "    } while (%s);\n", c_cond);
    } else if (runtime_strcmp(s, "LOOP") == 0 || runtime_strcmp(s, "loop") == 0) {
        fprintf(out, "    } while (1);\n");
    } else if (runtime_strncmp(s, "SWAP ", 5) == 0 || runtime_strncmp(s, "swap ", 5) == 0) {
        char *args = bppc_trim((char *)s + 5);
        char *comma = runtime_strchr(args, ',');
        if (comma) {
            char v1[64] = {0}, v2[64] = {0};
            size_t v1_len = comma - args;
            if (v1_len < sizeof(v1)) {
                runtime_memcpy(v1, args, v1_len);
                v1[v1_len] = '\0';
            }
            runtime_strncpy(v2, comma + 1, sizeof(v2) - 1);
            char san1[64] = {0}, san2[64] = {0};
            bppc_sanitize_ident(bppc_trim(v1), san1, sizeof(san1));
            bppc_sanitize_ident(bppc_trim(v2), san2, sizeof(san2));
            fprintf(out, "    { double __swap_tmp = var_%s; var_%s = var_%s; var_%s = __swap_tmp; }\n", san1, san1, san2, san2);
        }
    } else if (runtime_strncmp(s, "SLEEP ", 6) == 0 || runtime_strncmp(s, "sleep ", 6) == 0 ||
               runtime_strncmp(s, "PAUSE ", 6) == 0 || runtime_strncmp(s, "pause ", 6) == 0) {
        char *expr = bppc_trim((char *)s + 6);
        char c_expr[256] = {0};
        bppc_convert_expression_to_c(expr, c_expr, sizeof(c_expr));
        fprintf(out, "    bpp_rt_sleep((double)(%s));\n", c_expr);
    } else if (runtime_strcasecmp(s, "BEEP") == 0) {
        fprintf(out, "    bpp_rt_beep();\n");
    } else if (runtime_strcasecmp(s, "END IF") == 0 || runtime_strcasecmp(s, "ENDIF") == 0) {
        fprintf(out, "    }\n");
    } else if (runtime_strcasecmp(s, "ELSE") == 0) {
        fprintf(out, "    } else {\n");
    } else if (runtime_strncasecmp(s, "ELSEIF ", 7) == 0) {
        char *then_p = runtime_strstr((char *)s, " THEN");
        if (!then_p) then_p = runtime_strstr((char *)s, " then");
        char cond[256] = {0};
        size_t clen = then_p ? (size_t)(then_p - (s + 7)) : runtime_strlen(s + 7);
        if (clen < sizeof(cond)) {
            runtime_memcpy(cond, s + 7, clen);
            cond[clen] = '\0';
        }
        char c_cond[256] = {0};
        bppc_convert_expression_to_c(bppc_trim(cond), c_cond, sizeof(c_cond));
        fprintf(out, "    } else if (%s) {\n", c_cond);
    } else if (runtime_strncmp(s, "IF ", 3) == 0 || runtime_strncmp(s, "if ", 3) == 0) {
        char *then_p = runtime_strstr((char *)s, " THEN");
        if (!then_p) then_p = runtime_strstr((char *)s, " then");
        if (then_p) {
            char cond[256] = {0};
            size_t clen = then_p - (s + 3);
            if (clen < sizeof(cond)) {
                runtime_memcpy(cond, s + 3, clen);
                cond[clen] = '\0';
            }
            char c_cond[256] = {0};
            bppc_convert_expression_to_c(bppc_trim(cond), c_cond, sizeof(c_cond));
            
            char *then_body = bppc_trim(then_p + 5);
            char *else_p = runtime_strstr(then_body, " ELSE ");
            if (!else_p) else_p = runtime_strstr(then_body, " else ");
            char else_target[64] = {0};
            if (else_p) {
                *else_p = '\0';
                char *after_else = bppc_trim(else_p + 6);
                if (runtime_strncasecmp(after_else, "GOTO ", 5) == 0) {
                    after_else = bppc_trim(after_else + 5);
                }
                runtime_strncpy(else_target, after_else, sizeof(else_target) - 1);
            }
            then_body = bppc_trim(then_body);

            if (*then_body == '\0') {
                fprintf(out, "    if (%s) {\n", c_cond);
            } else {
                if (runtime_strncasecmp(then_body, "LET ", 4) == 0) {
                    then_body = bppc_trim(then_body + 4);
                }
                if (runtime_isdigit((unsigned char)*then_body)) {
                    if (else_target[0] != '\0') {
                        fprintf(out, "    if (%s) goto line_%s; else goto line_%s;\n", c_cond, then_body, else_target);
                    } else {
                        fprintf(out, "    if (%s) goto line_%s;\n", c_cond, then_body);
                    }
                } else if (runtime_strncasecmp(then_body, "GOTO ", 5) == 0) {
                    if (else_target[0] != '\0') {
                        fprintf(out, "    if (%s) goto line_%s; else goto line_%s;\n", c_cond, bppc_trim(then_body + 5), else_target);
                    } else {
                        fprintf(out, "    if (%s) goto line_%s;\n", c_cond, bppc_trim(then_body + 5));
                    }
                } else {
                    char *sub_eq = bppc_find_unquoted_char(then_body, '=');
                    bool is_keyword_stmt = (runtime_strncasecmp(then_body, "PRINT", 5) == 0 ||
                                           runtime_strncasecmp(then_body, "INPUT", 5) == 0 ||
                                           runtime_strncasecmp(then_body, "CLS", 3) == 0 ||
                                           runtime_strncasecmp(then_body, "BEEP", 4) == 0 ||
                                           runtime_strncasecmp(then_body, "END", 3) == 0 ||
                                           runtime_strncasecmp(then_body, "STOP", 4) == 0);
                    if (sub_eq != NULL && !is_keyword_stmt) {
                        char sub_var[64] = {0};
                        size_t svlen = sub_eq - then_body;
                        if (svlen < sizeof(sub_var)) {
                            runtime_memcpy(sub_var, then_body, svlen);
                            sub_var[svlen] = '\0';
                        }
                        char *sub_rhs = bppc_trim(sub_eq + 1);
                        char sub_rhs_c[512] = {0};
                        bppc_convert_expression_to_c(sub_rhs, sub_rhs_c, sizeof(sub_rhs_c));
                        
                        char *lpar = runtime_strchr(sub_var, '(');
                        if (lpar) {
                            char *rpar = runtime_strchr(lpar, ')');
                            if (rpar) {
                                char arr_name[64] = {0};
                                size_t arr_len = lpar - sub_var;
                                runtime_memcpy(arr_name, sub_var, arr_len);
                                char san_arr[64] = {0};
                                bppc_sanitize_ident(bppc_trim(arr_name), san_arr, sizeof(san_arr));
                                char indices[64] = {0};
                                size_t idx_len = rpar - (lpar + 1);
                                runtime_memcpy(indices, lpar + 1, idx_len);
                                char *comma = runtime_strchr(indices, ',');
                                if (comma) {
                                    *comma = '\0';
                                    char idx1_c[64] = {0}, idx2_c[64] = {0};
                                    bppc_convert_expression_to_c(bppc_trim(indices), idx1_c, sizeof(idx1_c));
                                    bppc_convert_expression_to_c(bppc_trim(comma + 1), idx2_c, sizeof(idx2_c));
                                    fprintf(out, "    if (%s) { var_%s[(int)(%s)][(int)(%s)] = %s; }\n", c_cond, san_arr, idx1_c, idx2_c, sub_rhs_c);
                                } else {
                                    char idx1_c[64] = {0};
                                    bppc_convert_expression_to_c(bppc_trim(indices), idx1_c, sizeof(idx1_c));
                                    fprintf(out, "    if (%s) { var_%s[(int)(%s)] = %s; }\n", c_cond, san_arr, idx1_c, sub_rhs_c);
                                }
                            }
                        } else {
                            char san_sub_var[64] = {0};
                            bppc_sanitize_ident(bppc_trim(sub_var), san_sub_var, sizeof(san_sub_var));
                            fprintf(out, "    if (%s) { var_%s = %s; }\n", c_cond, san_sub_var, sub_rhs_c);
                        }
                    } else {
                        fprintf(out, "    if (%s) {\n", c_cond);
                        char sub_stmt_buf[512];
                        runtime_strncpy(sub_stmt_buf, then_body, sizeof(sub_stmt_buf) - 1);
                        sub_stmt_buf[sizeof(sub_stmt_buf) - 1] = '\0';
                        char *sub_cur = sub_stmt_buf;
                        while (sub_cur && *sub_cur) {
                            char *col = bppc_find_unquoted_char(sub_cur, ':');
                            if (col) *col = '\0';
                            char *trimmed_sub = bppc_trim(sub_cur);
                            if (*trimmed_sub) {
                                bppc_emit_statement(out, trimmed_sub, false);
                            }
                            if (col) sub_cur = col + 1;
                            else break;
                        }
                        fprintf(out, "    }\n");
                    }
                }
            }
        }
    } else if (runtime_strncmp(s, "GOTO ", 5) == 0 || runtime_strncmp(s, "goto ", 5) == 0) {
        char *target = bppc_trim((char *)s + 5);
        fprintf(out, "    goto line_%s;\n", target);
    } else if (runtime_strncmp(s, "GOSUB ", 6) == 0 || runtime_strncmp(s, "gosub ", 6) == 0) {
        char *target = bppc_trim((char *)s + 6);
        fprintf(out, "    // GOSUB line_%s\n", target);
    } else if (runtime_strcmp(s, "RETURN") == 0 || runtime_strcmp(s, "return") == 0) {
        fprintf(out, "    // RETURN\n");
    } else if (runtime_strcmp(s, "CLS") == 0 || runtime_strcmp(s, "cls") == 0) {
        fprintf(out, "    bpp_rt_cls();\n");
    } else if (runtime_strcasecmp(s, "TRON") == 0) {
        fprintf(out, "    bpp_rt_tron(1);\n");
    } else if (runtime_strcasecmp(s, "TROFF") == 0) {
        fprintf(out, "    bpp_rt_tron(0);\n");
    } else if (runtime_strncasecmp(s, "DEF FN", 6) == 0 || runtime_strncasecmp(s, "DEF  FN", 7) == 0) {
        fprintf(out, "    // %s\n", s);
    } else if (runtime_strcmp(s, "END") == 0 || runtime_strcmp(s, "end") == 0 || runtime_strcmp(s, "STOP") == 0 || runtime_strcmp(s, "stop") == 0) {
        fprintf(out, "    return 0;\n");
    } else if (runtime_strncmp(s, "BSAVE ", 6) == 0 || runtime_strncmp(s, "bsave ", 6) == 0) {
        char args[256];
        runtime_strncpy(args, s + 6, sizeof(args) - 1);
        args[sizeof(args) - 1] = '\0';
        char *comma1 = runtime_strchr(args, ',');
        if (comma1) {
            *comma1 = '\0';
            char *comma2 = runtime_strchr(comma1 + 1, ',');
            if (comma2) {
                *comma2 = '\0';
                fprintf(out, "    bpp_rt_bsave(%s, (int)(%s), (int)(%s));\n", bppc_trim(args), bppc_trim(comma1 + 1), bppc_trim(comma2 + 1));
            }
        }
    } else if (runtime_strncmp(s, "BLOAD ", 6) == 0 || runtime_strncmp(s, "bload ", 6) == 0) {
        char args[256];
        runtime_strncpy(args, s + 6, sizeof(args) - 1);
        args[sizeof(args) - 1] = '\0';
        char *comma = runtime_strchr(args, ',');
        if (comma) {
            *comma = '\0';
            fprintf(out, "    bpp_rt_bload(%s, (int)(%s));\n", bppc_trim(args), bppc_trim(comma + 1));
        } else {
            fprintf(out, "    bpp_rt_bload(%s, 0);\n", bppc_trim(args));
        }
    } else if (runtime_strncmp(s, "QSAVE ", 6) == 0 || runtime_strncmp(s, "qsave ", 6) == 0) {
        char args[256];
        runtime_strncpy(args, s + 6, sizeof(args) - 1);
        args[sizeof(args) - 1] = '\0';
        char *comma1 = runtime_strchr(args, ',');
        if (comma1) {
            *comma1 = '\0';
            char *comma2 = runtime_strchr(comma1 + 1, ',');
            if (comma2) {
                *comma2 = '\0';
                fprintf(out, "    bpp_rt_qsave(%s, (int)(%s), (int)(%s));\n", bppc_trim(args), bppc_trim(comma1 + 1), bppc_trim(comma2 + 1));
            }
        }
    } else if (runtime_strncmp(s, "QLOAD ", 6) == 0 || runtime_strncmp(s, "qload ", 6) == 0) {
        char args[256];
        runtime_strncpy(args, s + 6, sizeof(args) - 1);
        args[sizeof(args) - 1] = '\0';
        char *comma = runtime_strchr(args, ',');
        if (comma) {
            *comma = '\0';
            fprintf(out, "    bpp_rt_qload(%s, (int)(%s));\n", bppc_trim(args), bppc_trim(comma + 1));
        } else {
            fprintf(out, "    bpp_rt_qload(%s, 0);\n", bppc_trim(args));
        }
    } else if (runtime_strncmp(s, "CLEAR", 5) == 0 || runtime_strncmp(s, "clear", 5) == 0) {
        fprintf(out, "    bpp_rt_clear();\n");
    } else if (runtime_strncasecmp(s, "ON ERROR GOTO ", 14) == 0) {
        char *target = bppc_trim((char *)s + 14);
        int lnum = runtime_atoi(target);
        if (lnum > 0) {
            fprintf(out, "    bpp_err_target = %d;\n", lnum);
        } else {
            fprintf(out, "    bpp_err_target = 0;\n");
        }
    } else if (runtime_strcasecmp(s, "RESUME NEXT") == 0) {
        fprintf(out, "    // RESUME NEXT\n");
    } else if (runtime_strncasecmp(s, "RESUME ", 7) == 0) {
        char *target = bppc_trim((char *)s + 7);
        fprintf(out, "    goto line_%s;\n", target);
    } else if (runtime_strcasecmp(s, "RESUME") == 0) {
        fprintf(out, "    // RESUME\n");
    } else if (runtime_strncmp(s, "PRINT ", 6) == 0 || runtime_strncmp(s, "print ", 6) == 0 || runtime_strcmp(s, "PRINT") == 0 || runtime_strcmp(s, "print") == 0) {
        char *expr = (runtime_strlen(s) > 6) ? (char *)s + 6 : "";
        bppc_emit_print(out, expr);
    } else if (runtime_strchr(s, '=') != NULL) {
        char *eq = runtime_strchr((char *)s, '=');
        char var_name[64] = {0};
        size_t nlen = eq - s;
        if (nlen < sizeof(var_name)) {
            runtime_memcpy(var_name, s, nlen);
            var_name[nlen] = '\0';
        }

        char *rhs = bppc_trim(eq + 1);
        char rhs_c[512] = {0};
        bppc_convert_expression_to_c(rhs, rhs_c, sizeof(rhs_c));

        char *lpar = runtime_strchr(var_name, '(');
        if (lpar) {
            char *rpar = runtime_strchr(lpar, ')');
            if (rpar) {
                char arr_name[64] = {0};
                size_t arr_len = lpar - var_name;
                runtime_memcpy(arr_name, var_name, arr_len);
                char san_arr[64] = {0};
                bppc_sanitize_ident(bppc_trim(arr_name), san_arr, sizeof(san_arr));

                char indices[64] = {0};
                size_t idx_len = rpar - (lpar + 1);
                runtime_memcpy(indices, lpar + 1, idx_len);

                char *comma = runtime_strchr(indices, ',');
                if (comma) {
                    *comma = '\0';
                    char idx1_c[64] = {0}, idx2_c[64] = {0};
                    bppc_convert_expression_to_c(bppc_trim(indices), idx1_c, sizeof(idx1_c));
                    bppc_convert_expression_to_c(bppc_trim(comma + 1), idx2_c, sizeof(idx2_c));
                    fprintf(out, "    var_%s[(int)(%s)][(int)(%s)] = %s;\n", san_arr, idx1_c, idx2_c, rhs_c);
                } else {
                    char idx1_c[64] = {0};
                    bppc_convert_expression_to_c(bppc_trim(indices), idx1_c, sizeof(idx1_c));
                    fprintf(out, "    var_%s[(int)(%s)] = %s;\n", san_arr, idx1_c, rhs_c);
                }
            }
        } else {
            char sanitized_var[64] = {0};
            bppc_sanitize_ident(bppc_trim(var_name), sanitized_var, sizeof(sanitized_var));

            if (runtime_strstr(var_name, "$")) {
                char fmt_buf[128] = "";
                char args_buf[512] = "";
                char cur_item[256] = "";
                int cur_item_len = 0;
                bool in_item_str = false;
                int item_paren = 0;
                int item_count = 0;

                for (size_t ri = 0; ; ri++) {
                    char rc = rhs[ri];
                    if (rc == '"') in_item_str = !in_item_str;
                    if (!in_item_str && rc == '(') item_paren++;
                    if (!in_item_str && rc == ')') item_paren--;

                    if ((!in_item_str && item_paren == 0 && (rc == '+' || rc == '\0')) || rc == '\0') {
                        cur_item[cur_item_len] = '\0';
                        char *trim_item = bppc_trim(cur_item);
                        if (*trim_item) {
                            char item_c[256] = {0};
                            bppc_convert_expression_to_c(trim_item, item_c, sizeof(item_c));
                            runtime_strncat(fmt_buf, "%s", sizeof(fmt_buf) - runtime_strlen(fmt_buf) - 1);
                            if (item_count > 0) runtime_strncat(args_buf, ", ", sizeof(args_buf) - runtime_strlen(args_buf) - 1);
                            runtime_strncat(args_buf, item_c, sizeof(args_buf) - runtime_strlen(args_buf) - 1);
                            item_count++;
                        }
                        cur_item_len = 0;
                        if (rc == '\0') break;
                    } else {
                        if (cur_item_len < (int)sizeof(cur_item) - 2) cur_item[cur_item_len++] = rc;
                    }
                }

                if (item_count > 0) {
                    fprintf(out, "    runtime_snprintf(var_%s, sizeof(var_%s), \"%s\", %s);\n", sanitized_var, sanitized_var, fmt_buf, args_buf);
                } else {
                    fprintf(out, "    runtime_snprintf(var_%s, sizeof(var_%s), \"%%s\", %s);\n", sanitized_var, sanitized_var, rhs_c);
                }
            } else {
                fprintf(out, "    var_%s = %s;\n", sanitized_var, rhs_c);
            }
        }
    }
}
