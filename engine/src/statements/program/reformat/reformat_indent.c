// FILENAME: reformat_indent.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libengine (reformat_internal.h)
// Provides runtime implementation for the REFORMAT_INDENT statement in BASIC++.
//
// ---- Includes ----

#include "statements/program/reformat_internal.h"

//
// ---- Pass 3 Indentation and Case Conversion ----

void reformat_plan_init(ReformatPlan *plan, int spaces) {
    if (!plan) return;
    memset(plan, 0, sizeof(ReformatPlan));
    plan->spaces_per_indent = (spaces > 0) ? spaces : 2;
}

BppError reformat_pass3_indent(VMContext *vm, const ReformatPlan *plan, ReformatModifier mod) {
    BppError err;
    memset(&err, 0, sizeof(err));

    if (!vm || !plan) return err;
    MemoryContext *mem = vm_get_mem(vm);
    if (!mem) return err;

    size_t line_count = 0;
    BppProgramLine *lines = mem_program_get_all(mem, &line_count);
    if (line_count == 0 || !lines) return err;

    int current_indent = 0;
    int spaces_per = plan->spaces_per_indent > 0 ? plan->spaces_per_indent : 2;

    int changed = 0;
    int unchanged = 0;

    for (size_t i = 0; i < line_count; i++) {
        double line_num = lines[i].line_number;
        if (plan->target_start > 0.0 || plan->target_end > 0.0) {
            if (line_num < plan->target_start || line_num > plan->target_end) {
                continue;
            }
        }

        const char *orig_text = lines[i].text ? lines[i].text : "";

        FastToken tokens[MAX_LINE_TOKENS];
        int tok_cnt = tokenize_line_fast(orig_text, tokens, MAX_LINE_TOKENS);

        int pre_adjust = 0;
        int post_adjust = 0;
        bool is_bounce = false;

        if (tok_cnt > 0) {
            BppKeywordId kw = tokens[0].kw;

            if (kw == KW_FOR || kw == KW_WHILE || kw == KW_DO || kw == KW_SELECT ||
                kw == KW_SUB || kw == KW_FUNCTION || kw == KW_TRY) {
                if (kw != KW_IF || is_block_if(tokens, tok_cnt)) {
                    post_adjust = 1;
                }
            } else if (kw == KW_NEXT || kw == KW_WEND || kw == KW_LOOP) {
                pre_adjust = -1;
            } else if (kw == KW_END) {
                ReformatBlockType end_t = get_end_block_type(tokens, tok_cnt, 0);
                if (end_t != BLOCK_NONE) {
                    pre_adjust = -1;
                }
            } else if (kw == KW_ELSE || kw == KW_CASE || kw == KW_CATCH) {
                is_bounce = true;
            }
        }

        current_indent += pre_adjust;
        if (current_indent < 0) current_indent = 0;

        int line_indent = is_bounce ? (current_indent > 0 ? current_indent - 1 : 0) : current_indent;

        if (tok_cnt > 0) {
            if (is_label_line(tokens, tok_cnt) || ((tokens[0].kw == KW_REM || tokens[0].type == TOK_DOCSTRING) && current_indent == 0)) {
                line_indent = 0;
            }
        }

        const char *body_text = skip_leading_ws(orig_text);

        char body_buf[1024];
        snprintf(body_buf, sizeof(body_buf), "%s", body_text);

        if (mod == MOD_UPPER || mod == MOD_LOWER) {
            LexerContext *lex = lex_init(NULL, body_buf);
            if (lex) {
                while (1) {
                    BppToken tok = lex_next(lex);
                    if (tok.type == TOK_EOF) break;
                    if (tok.type == TOK_KEYWORD && tok.start && tok.length > 0) {
                        char *p = (char*)tok.start;
                        for (size_t k = 0; k < tok.length; k++) {
                            if (mod == MOD_UPPER) p[k] = (char)toupper((unsigned char)p[k]);
                            else if (mod == MOD_LOWER) p[k] = (char)tolower((unsigned char)p[k]);
                        }
                    }
                }
                lex_shutdown(lex);
            }
        }

        char new_buf[2048];
        int indent_spaces = line_indent * spaces_per;
        if (indent_spaces < 0) indent_spaces = 0;
        if (indent_spaces > 128) indent_spaces = 128;
        snprintf(new_buf, sizeof(new_buf), "%*s%.1024s", indent_spaces, "", body_buf);

        if (strcmp(orig_text, new_buf) != 0) {
            mem_program_insert(mem, line_num, new_buf);
            changed++;
        } else {
            unchanged++;
        }

        current_indent += post_adjust;
        if (current_indent < 0) current_indent = 0;
    }

    ((ReformatPlan*)plan)->changed_lines = changed;
    ((ReformatPlan*)plan)->unchanged_lines = unchanged;
    return err;
}

//
// ---- Procedure Lookup and Spacing Management ----

bool reformat_find_sub_range(VMContext *vm, const char *sub_name, double *out_start, double *out_end) {
    if (!vm || !sub_name || !out_start || !out_end) return false;
    MemoryContext *mem = vm_get_mem(vm);
    if (!mem) return false;

    size_t line_count = 0;
    BppProgramLine *lines = mem_program_get_all(mem, &line_count);
    if (line_count == 0 || !lines) return false;

    bool found = false;
    double start_line = 0.0;
    double end_line = 0.0;

    for (size_t i = 0; i < line_count; i++) {
        const char *text = lines[i].text ? lines[i].text : "";
        FastToken tokens[MAX_LINE_TOKENS];
        int tok_cnt = tokenize_line_fast(text, tokens, MAX_LINE_TOKENS);
        if (tok_cnt >= 2 && (tokens[0].kw == KW_SUB || tokens[0].kw == KW_FUNCTION)) {
            if (strcasecmp(tokens[1].text, sub_name) == 0) {
                found = true;
                start_line = lines[i].line_number;
                for (size_t j = i + 1; j < line_count; j++) {
                    const char *end_text = lines[j].text ? lines[j].text : "";
                    FastToken end_toks[MAX_LINE_TOKENS];
                    int end_cnt = tokenize_line_fast(end_text, end_toks, MAX_LINE_TOKENS);
                    if (end_cnt >= 2 && end_toks[0].kw == KW_END) {
                        if (end_toks[1].kw == KW_SUB || end_toks[1].kw == KW_FUNCTION) {
                            end_line = lines[j].line_number;
                            break;
                        }
                    }
                }
                break;
            }
        }
    }

    if (found) {
        *out_start = start_line;
        *out_end = (end_line > start_line) ? end_line : start_line;
        return true;
    }
    return false;
}

void reformat_pass2_blank_lines(VMContext *vm, ReformatPlan *plan) {
    (void)plan;
    if (!vm) return;
    MemoryContext *mem = vm_get_mem(vm);
    if (!mem) return;

    size_t line_count = 0;
    BppProgramLine *lines = mem_program_get_all(mem, &line_count);
    if (line_count <= 1 || !lines) return;

    for (size_t i = 1; i < line_count; i++) {
        double curr_line = lines[i].line_number;
        double prev_line = lines[i - 1].line_number;
        const char *text = lines[i].text ? lines[i].text : "";

        FastToken tokens[MAX_LINE_TOKENS];
        int tok_cnt = tokenize_line_fast(text, tokens, MAX_LINE_TOKENS);
        if (tok_cnt == 0) continue;

        BppKeywordId kw = tokens[0].kw;
        if (kw == KW_SUB || kw == KW_FUNCTION) {
            const char *prev_text = lines[i - 1].text ? lines[i - 1].text : "";
            FastToken prev_tokens[MAX_LINE_TOKENS];
            int prev_tok_cnt = tokenize_line_fast(prev_text, prev_tokens, MAX_LINE_TOKENS);
            if (prev_tok_cnt > 0) {
                if (prev_tokens[0].kw == KW_REM || prev_tokens[0].type == TOK_DOCSTRING || is_label_line(prev_tokens, prev_tok_cnt)) {
                    continue;
                }
            }

            if (curr_line - prev_line >= 2.0) {
                double new_line_num = (double)((long long)(prev_line + (curr_line - prev_line) / 2.0));
                if (new_line_num <= prev_line) new_line_num = prev_line + 1.0;
                if (new_line_num < curr_line) {
                    mem_program_insert(mem, new_line_num, "REM");
                    lines = mem_program_get_all(mem, &line_count);
                }
            }
        }
    }
}

static bool is_protected_split_pattern(const FastToken *tokens, int count) {
    if (count <= 0) return false;
    BppKeywordId kw0 = tokens[0].kw;
    if (kw0 == KW_DATA || kw0 == KW_DEF || kw0 == KW_DIM || kw0 == KW_OPEN || kw0 == KW_ON) {
        return true;
    }
    return false;
}

static bool has_split_trigger_keyword(const FastToken *tokens, int count) {
    for (int i = 0; i < count; i++) {
        BppKeywordId kw = tokens[i].kw;
        if (kw == KW_FOR || kw == KW_NEXT || kw == KW_IF || kw == KW_THEN ||
            kw == KW_ELSE || kw == KW_WHILE || kw == KW_WEND ||
            kw == KW_DO || kw == KW_LOOP || kw == KW_SELECT || kw == KW_CASE ||
            kw == KW_SUB || kw == KW_FUNCTION || kw == KW_TRY || kw == KW_CATCH ||
            kw == KW_GOSUB || kw == KW_RETURN) {
            return true;
        }
    }
    return false;
}

void reformat_pass_split(VMContext *vm, ReformatPlan *plan) {
    if (!vm || !plan) return;
    MemoryContext *mem = vm_get_mem(vm);
    if (!mem) return;

    size_t line_count = 0;
    BppProgramLine *lines = mem_program_get_all(mem, &line_count);
    if (line_count == 0 || !lines) return;

    for (size_t i = 0; i < line_count; i++) {
        double line_num = lines[i].line_number;
        const char *text = lines[i].text ? lines[i].text : "";

        FastToken tokens[MAX_LINE_TOKENS];
        int tok_cnt = tokenize_line_fast(text, tokens, MAX_LINE_TOKENS);
        if (tok_cnt == 0) continue;

        int colon_count = 0;
        bool in_str = false;
        const char *p = text;
        while (*p) {
            if (*p == '"') in_str = !in_str;
            else if (*p == ':' && !in_str) colon_count++;
            p++;
        }

        if (colon_count > 0) {
            if (is_protected_split_pattern(tokens, tok_cnt)) {
                continue;
            }

            if (has_split_trigger_keyword(tokens, tok_cnt)) {
                char text_copy[512];
                snprintf(text_copy, sizeof(text_copy), "%s", text);
                char *parts[16];
                int part_count = 0;
                char *curr = text_copy;
                in_str = false;
                parts[part_count++] = curr;

                for (char *c = text_copy; *c; c++) {
                    if (*c == '"') in_str = !in_str;
                    else if (*c == ':' && !in_str) {
                        *c = '\0';
                        if (part_count < 16) {
                            parts[part_count++] = c + 1;
                        }
                    }
                }

                if (part_count > 1) {
                    double next_line = (i + 1 < line_count) ? lines[i + 1].line_number : (line_num + 10.0);
                    double gap = (next_line - line_num) / (double)part_count;

                    mem_program_insert(mem, line_num, skip_leading_ws(parts[0]));

                    for (int k = 1; k < part_count; k++) {
                        double new_line_num = line_num + gap * (double)k;
                        mem_program_insert(mem, new_line_num, skip_leading_ws(parts[k]));
                    }
                    lines = mem_program_get_all(mem, &line_count);
                }
            } else if (colon_count >= 2) {
                add_suggestion(plan, line_num, text, "Contains 3+ statements; consider splitting if logically independent.");
            }
        }
    }
}
