// FILENAME: reformat_analyze.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libengine (reformat_internal.h)
// Provides runtime implementation for the REFORMAT_ANALYZE statement in BASIC++.
//
// ---- Includes ----

#include "statements/program/reformat_internal.h"

//
// ---- Fast Tokenizer and Block Checkers ----

int tokenize_line_fast(const char *text, FastToken *tokens, int max_tokens) {
    if (!text) return 0;
    LexerContext *lex = lex_init(NULL, text);
    if (!lex) return 0;

    int count = 0;
    while (count < max_tokens) {
        BppToken tok = lex_next(lex);
        if (tok.type == TOK_EOF) break;

        tokens[count].kw = (tok.type == TOK_KEYWORD) ? tok.as.keyword : KW_NONE;
        tokens[count].type = tok.type;
        size_t len = tok.length < 63 ? tok.length : 63;
        if (tok.start && len > 0) {
            memcpy(tokens[count].text, tok.start, len);
        }
        tokens[count].text[len] = '\0';
        count++;
    }
    lex_shutdown(lex);
    return count;
}

bool is_label_line(const FastToken *tokens, int count) {
    if (count <= 0) return false;
    if (tokens[0].type == TOK_IDENT) {
        size_t len = strlen(tokens[0].text);
        if (len > 1 && tokens[0].text[len - 1] == ':') {
            return true;
        }
    }
    return false;
}

bool is_block_if(const FastToken *tokens, int count) {
    int then_idx = -1;
    for (int i = 0; i < count; i++) {
        if (tokens[i].kw == KW_THEN) {
            then_idx = i;
            break;
        }
    }
    if (then_idx < 0) return false;
    for (int j = then_idx + 1; j < count; j++) {
        if (tokens[j].kw == KW_REM || tokens[j].type == TOK_DOCSTRING) continue;
        return false;
    }
    return true;
}

ReformatBlockType get_end_block_type(const FastToken *tokens, int count, int end_idx) {
    if (end_idx + 1 < count) {
        BppKeywordId next_kw = tokens[end_idx + 1].kw;
        if (next_kw == KW_IF)       return BLOCK_IF;
        if (next_kw == KW_SUB)      return BLOCK_SUB;
        if (next_kw == KW_FUNCTION) return BLOCK_FUNCTION;
        if (next_kw == KW_SELECT)   return BLOCK_SELECT;
        if (next_kw == KW_TRY)      return BLOCK_TRY;
        if (next_kw == KW_WITH)     return BLOCK_WITH;
    }
    return BLOCK_NONE;
}

//
// ---- Pass 1 Analysis Engine ----

void reformat_pass1_analyze(VMContext *vm, ReformatPlan *plan) {
    if (!vm || !plan) return;

    MemoryContext *mem = vm_get_mem(vm);
    if (!mem) return;

    size_t line_count = 0;
    BppProgramLine *lines = mem_program_get_all(mem, &line_count);
    plan->total_lines = (int)line_count;

    const char *curr_fn = vm_get_current_filename(vm);
    if (curr_fn && strlen(curr_fn) > 0 && strcmp(curr_fn, "untitled") != 0) {
        plan->has_filename = true;
        snprintf(plan->filename, sizeof(plan->filename), "%s", curr_fn);
    } else {
        plan->has_filename = false;
        plan->filename[0] = '\0';
    }

    ReformatBlockStack stack;
    memset(&stack, 0, sizeof(stack));

    BlockRange closed_blocks[MAX_BLOCK_RANGES];
    int closed_block_count = 0;

    typedef struct { char name[64]; double line; bool referenced; } LabelInfo;
    LabelInfo labels[128];
    int label_count = 0;

    typedef struct { char target[64]; double line; } GotoInfo;
    GotoInfo gotos[128];
    int goto_count = 0;

    double last_opener_line = -1.0;
    ReformatBlockType last_opener_type = BLOCK_NONE;

    for (size_t i = 0; i < line_count; i++) {
        double line_num = lines[i].line_number;
        const char *text = lines[i].text;
        if (!text) continue;

        FastToken tokens[MAX_LINE_TOKENS];
        int tok_cnt = tokenize_line_fast(text, tokens, MAX_LINE_TOKENS);
        if (tok_cnt == 0) continue;

        if (is_label_line(tokens, tok_cnt) && label_count < 128) {
            char lbl[64];
            snprintf(lbl, sizeof(lbl), "%s", tokens[0].text);
            lbl[strlen(lbl) - 1] = '\0';
            snprintf(labels[label_count].name, sizeof(labels[label_count].name), "%s", lbl);
            labels[label_count].line = line_num;
            labels[label_count].referenced = false;
            label_count++;
        }

        for (int k = 0; k < tok_cnt; k++) {
            if ((tokens[k].kw == KW_GOTO || tokens[k].kw == KW_GOSUB) && k + 1 < tok_cnt) {
                if (goto_count < 128) {
                    snprintf(gotos[goto_count].target, sizeof(gotos[goto_count].target), "%s", tokens[k + 1].text);
                    gotos[goto_count].line = line_num;
                    goto_count++;
                }
            }
        }

        BppKeywordId kw = tokens[0].kw;

        ReformatBlockType push_type = BLOCK_NONE;
        ReformatBlockType pop_type = BLOCK_NONE;

        if (kw == KW_FOR)      push_type = BLOCK_FOR;
        else if (kw == KW_WHILE) push_type = BLOCK_WHILE;
        else if (kw == KW_DO)    push_type = BLOCK_DO;
        else if (kw == KW_IF && is_block_if(tokens, tok_cnt)) push_type = BLOCK_IF;
        else if (kw == KW_SELECT) push_type = BLOCK_SELECT;
        else if (kw == KW_SUB)   push_type = BLOCK_SUB;
        else if (kw == KW_FUNCTION) push_type = BLOCK_FUNCTION;
        else if (kw == KW_TRY)   push_type = BLOCK_TRY;

        if (kw == KW_NEXT) pop_type = BLOCK_FOR;
        else if (kw == KW_WEND) pop_type = BLOCK_WHILE;
        else if (kw == KW_LOOP) pop_type = BLOCK_DO;
        else if (kw == KW_END) {
            ReformatBlockType end_t = get_end_block_type(tokens, tok_cnt, 0);
            if (end_t != BLOCK_NONE) {
                pop_type = end_t;
            }
        }

        if (pop_type != BLOCK_NONE && last_opener_line > 0 && last_opener_type == pop_type) {
            if (i > 0 && lines[i - 1].line_number == last_opener_line) {
                char what[256], why[256], how[256];
                snprintf(what, sizeof(what), "Empty block body between %s (line %lld) and %s (line %lld)",
                         get_block_name(pop_type), (long long)last_opener_line,
                         get_expected_closer(pop_type), (long long)line_num);
                snprintf(why, sizeof(why), "%s", "An empty block body usually indicates dead code or incomplete logic.");
                snprintf(how, sizeof(how), "%s", "Add statements inside the block or remove the empty block pair.");
                add_diagnostic(plan, DIAG_WARNING, line_num, what, why, how);
            }
        }

        if (push_type != BLOCK_NONE) {
            if (stack.depth < REFORMAT_MAX_NESTING) {
                stack.entries[stack.depth].type = push_type;
                stack.entries[stack.depth].line_opened = line_num;
                stack.depth++;
            }
            last_opener_line = line_num;
            last_opener_type = push_type;
        }

        if (pop_type != BLOCK_NONE) {
            if (stack.depth == 0) {
                char what[256], why[256], how[256];
                snprintf(what, sizeof(what), "Block closer '%s' at line %lld has no matching opener",
                         get_expected_closer(pop_type), (long long)line_num);
                snprintf(why, sizeof(why), "%s", "Unexpected block closer without a corresponding opener breaks program structure.");
                snprintf(how, sizeof(how), "%s", "Remove the orphaned closer statement or add the missing block opener.");
                add_diagnostic(plan, DIAG_ERROR, line_num, what, why, how);
            } else {
                ReformatBlockType top_type = stack.entries[stack.depth - 1].type;
                double open_line = stack.entries[stack.depth - 1].line_opened;
                stack.depth--;

                if (top_type != pop_type) {
                    char what[256], why[256], how[256];
                    snprintf(what, sizeof(what), "Block type mismatch — '%s' at line %lld expected '%s', found '%s' at line %lld",
                             get_block_name(top_type), (long long)open_line,
                             get_expected_closer(top_type), get_expected_closer(pop_type), (long long)line_num);
                    snprintf(why, sizeof(why), "%s", "Mismatched block closers indicate crossed nesting or invalid structure.");
                    snprintf(how, sizeof(how), "%s", "Ensure nested blocks are properly terminated in reverse order of opening.");
                    add_diagnostic(plan, DIAG_ERROR, line_num, what, why, how);
                } else if (closed_block_count < MAX_BLOCK_RANGES) {
                    closed_blocks[closed_block_count].type = top_type;
                    closed_blocks[closed_block_count].start_line = open_line;
                    closed_blocks[closed_block_count].end_line = line_num;
                    closed_block_count++;
                }
            }
            last_opener_line = -1.0;
            last_opener_type = BLOCK_NONE;
        }
    }

    while (stack.depth > 0) {
        ReformatBlockType open_type = stack.entries[stack.depth - 1].type;
        double open_line = stack.entries[stack.depth - 1].line_opened;
        stack.depth--;

        char what[256], why[256], how[256];
        if (open_type == BLOCK_SUB || open_type == BLOCK_FUNCTION) {
            snprintf(what, sizeof(what), "%s at line %lld has no matching END %s",
                     get_block_name(open_type), (long long)open_line, get_block_name(open_type));
            snprintf(why, sizeof(why), "%s", "Procedure definitions must be terminated with END SUB or END FUNCTION.");
            snprintf(how, sizeof(how), "Add 'END %s' at the end of the procedure definition.", get_block_name(open_type));
            add_diagnostic(plan, DIAG_ERROR, open_line, what, why, how);
        } else {
            snprintf(what, sizeof(what), "%s at line %lld has no matching %s",
                     get_block_name(open_type), (long long)open_line, get_expected_closer(open_type));
            snprintf(why, sizeof(why), "%s", "Every structured control block must be closed before end of file.");
            snprintf(how, sizeof(how), "Add '%s' before end of procedure/file.", get_expected_closer(open_type));
            add_diagnostic(plan, DIAG_ERROR, open_line, what, why, how);
        }
    }

    for (int l = 0; l < label_count; l++) {
        bool ref = false;
        for (int g = 0; g < goto_count; g++) {
            if (strcasecmp(labels[l].name, gotos[g].target) == 0) {
                ref = true;
                break;
            }
        }
        if (!ref) {
            char what[256], why[256], how[256];
            snprintf(what, sizeof(what), "Unreferenced label '%.128s' at line %lld",
                     labels[l].name, (long long)labels[l].line);
            snprintf(why, sizeof(why), "%s", "Unreferenced labels represent unused target points or dead code.");
            snprintf(how, sizeof(how), "%s", "Remove the label or update GOTO/GOSUB statements to reference it.");
            add_diagnostic(plan, DIAG_WARNING, labels[l].line, what, why, how);
        }
    }

    for (int g = 0; g < goto_count; g++) {
        char *endptr;
        double target_line = strtod(gotos[g].target, &endptr);
        if (*endptr == '\0' && target_line > 0) {
            for (int r = 0; r < closed_block_count; r++) {
                if (target_line > closed_blocks[r].start_line && target_line < closed_blocks[r].end_line) {
                    if (gotos[g].line < closed_blocks[r].start_line || gotos[g].line > closed_blocks[r].end_line) {
                        char what[256], why[256], how[256];
                        snprintf(what, sizeof(what), "GOTO %lld at line %lld jumps into the middle of a %s block (lines %lld-%lld)",
                                 (long long)target_line, (long long)gotos[g].line,
                                 get_block_name(closed_blocks[r].type),
                                 (long long)closed_blocks[r].start_line, (long long)closed_blocks[r].end_line);
                        snprintf(why, sizeof(why), "%s", "Jumping directly into a structured block bypasses initialization and breaks control flow.");
                        snprintf(how, sizeof(how), "%s", "Restructure the GOTO target to enter the block at its opener statement.");
                        add_diagnostic(plan, DIAG_WARNING, gotos[g].line, what, why, how);
                    }
                }
            }
        }
    }
}
