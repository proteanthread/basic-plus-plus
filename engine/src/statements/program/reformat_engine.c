/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file reformat_engine.c
 * @brief Formatting engine for REFORMAT statement, supporting multi-pass analysis and indentation.
 *
 * 1. WHAT IT DOES:
 * Implements reformat_plan_init(), reformat_pass1_analyze(), reformat_pass3_indent(), and
 * reformat_render_check_report() for static program analysis and source code indentation.
 *
 * 2. WHY IT EXISTS:
 * Encapsulates the multi-pass REFORMAT pipeline into a modular engine to support interactive
 * formatting, STRICT validation, and CHECK mode reporting without code duplication.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Pass 1 uses a push/pop stack state machine to analyze control flow blocks (FOR/NEXT, WHILE/WEND,
 * DO/LOOP, IF/THEN/ELSE, SELECT/CASE, SUB/FUNCTION, TRY/CATCH, TYPE, CLASS, ATOMIC, WITH) and detect
 * 4 ERROR conditions and 3 WARNING conditions. Pass 3 strips leading whitespace and re-indents.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'stmt_reformat'. Includes "statements/program/reformat.h",
 * "vm/vm.h", "memory/memory.h", "lexer/lexer.h", "device/vdev.h", "platform/platform.h",
 * <stdio.h>, <stdlib.h>, <string.h>, <ctype.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Included in desktop ('baspp') and REPL ('bpp') editions.
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Extend diagnostic checks in pass 1 or add formatting modifiers to pass 3.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Line number formatting invariant: line numbers are doubles internally, MUST format using %lld
 * cast from (long long) per Rule #10.
 *
 * 8. WHAT TO EXPECT:
 * Analyzes program structure, builds diagnostic reports, and formats memory lines in-place.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify stack push/pop balance logic in pass 1 and whitespace stripping logic in pass 3.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid initialized VMContext and MemoryContext with program lines stored in order.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Thread-safe, bounded string operations using memchr/snprintf per Rule #1.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/memory/mem_system.c
 * - engine/src/lexer/lexer.c
 * - engine/src/device/vdev.c
 * Prerequisite Header Files:
 * - engine/include/statements/program/reformat.h
 * - engine/include/vm/vm.h
 * - engine/include/memory/memory.h
 * - engine/include/lexer/lexer.h
 * - engine/include/device/vdev.h
 */

#include "statements/program/reformat.h"
#include "vm/vm.h"
#include "memory/memory.h"
#include "lexer/lexer.h"
#include "device/vdev.h"
#include "platform/platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef _MSC_VER
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#endif

typedef struct {
    ReformatBlockType type;
    double            start_line;
    double            end_line;
} BlockRange;

#define MAX_BLOCK_RANGES 128

static const char *get_block_name(ReformatBlockType type) {
    switch (type) {
        case BLOCK_FOR:      return "FOR";
        case BLOCK_WHILE:    return "WHILE";
        case BLOCK_DO:       return "DO";
        case BLOCK_IF:       return "IF";
        case BLOCK_SELECT:   return "SELECT";
        case BLOCK_SUB:      return "SUB";
        case BLOCK_FUNCTION: return "FUNCTION";
        case BLOCK_TRY:      return "TRY";
        case BLOCK_TYPE:     return "TYPE";
        case BLOCK_CLASS:    return "CLASS";
        case BLOCK_ATOMIC:   return "ATOMIC";
        case BLOCK_WITH:     return "WITH";
        default:             return "BLOCK";
    }
}

static const char *get_expected_closer(ReformatBlockType type) {
    switch (type) {
        case BLOCK_FOR:      return "NEXT";
        case BLOCK_WHILE:    return "WEND";
        case BLOCK_DO:       return "LOOP";
        case BLOCK_IF:       return "END IF";
        case BLOCK_SELECT:   return "END SELECT";
        case BLOCK_SUB:      return "END SUB";
        case BLOCK_FUNCTION: return "END FUNCTION";
        case BLOCK_TRY:      return "END TRY";
        case BLOCK_TYPE:     return "END TYPE";
        case BLOCK_CLASS:    return "END CLASS";
        case BLOCK_ATOMIC:   return "END ATOMIC";
        case BLOCK_WITH:     return "END WITH";
        default:             return "END";
    }
}

void reformat_plan_init(ReformatPlan *plan, int spaces) {
    if (!plan) return;
    memset(plan, 0, sizeof(ReformatPlan));
    plan->spaces_per_indent = (spaces > 0) ? spaces : 2;
}

static void add_diagnostic(ReformatPlan *plan, DiagSeverity sev, double line,
                           const char *what, const char *why, const char *how) {
    if (!plan || plan->diag_count >= REFORMAT_MAX_DIAGNOSTICS) return;
    ReformatDiagnostic *d = &plan->diagnostics[plan->diag_count++];
    d->severity = sev;
    d->line = line;
    snprintf(d->what, sizeof(d->what), "%s", what ? what : "");
    snprintf(d->why,  sizeof(d->why),  "%s", why  ? why  : "");
    snprintf(d->how,  sizeof(d->how),  "%s", how  ? how  : "");

    if (sev == DIAG_ERROR) {
        plan->error_count++;
    } else {
        plan->warning_count++;
    }
}

static const char *skip_leading_ws(const char *str) {
    if (!str) return "";
    while (*str && isspace((unsigned char)*str)) str++;
    return str;
}

typedef struct {
    BppKeywordId kw;
    BppTokenType type;
    char text[64];
} FastToken;

#define MAX_LINE_TOKENS 32

static int tokenize_line_fast(const char *text, FastToken *tokens, int max_tokens) {
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

static bool is_label_line(const FastToken *tokens, int count) {
    if (count <= 0) return false;
    if (tokens[0].type == TOK_IDENT) {
        size_t len = strlen(tokens[0].text);
        if (len > 1 && tokens[0].text[len - 1] == ':') {
            return true;
        }
    }
    return false;
}

static bool is_block_if(const FastToken *tokens, int count) {
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

static ReformatBlockType get_end_block_type(const FastToken *tokens, int count, int end_idx) {
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
            snprintf(what, sizeof(what), "Unreferenced label '%s' at line %lld",
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

        char new_buf[1024];
        int indent_spaces = line_indent * spaces_per;
        snprintf(new_buf, sizeof(new_buf), "%*s%s", indent_spaces, "", body_buf);

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

void reformat_render_check_report(VMContext *vm, const ReformatPlan *plan, bool save_requested) {
    (void)save_requested;
    if (!vm || !plan) return;

    VDevContext *vd = vm_get_vdev(vm);
    vdev_puts(vd, "REFORMAT CHECK: Analyzing program...\n\n");

    vdev_puts(vd, "=== DIFF PREVIEW ===\n");
    MemoryContext *mem = vm_get_mem(vm);
    size_t line_count = 0;
    BppProgramLine *lines = mem_program_get_all(mem, &line_count);

    int current_indent = 0;
    int spaces_per = plan->spaces_per_indent > 0 ? plan->spaces_per_indent : 2;
    int diff_shown = 0;

    for (size_t i = 0; i < line_count; i++) {
        double line_num = lines[i].line_number;
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
                if (kw != KW_IF || is_block_if(tokens, tok_cnt)) post_adjust = 1;
            } else if (kw == KW_NEXT || kw == KW_WEND || kw == KW_LOOP) {
                pre_adjust = -1;
            } else if (kw == KW_END) {
                ReformatBlockType end_t = get_end_block_type(tokens, tok_cnt, 0);
                if (end_t != BLOCK_NONE) pre_adjust = -1;
            } else if (kw == KW_ELSE || kw == KW_CASE || kw == KW_CATCH) {
                is_bounce = true;
            }
        }

        current_indent += pre_adjust;
        if (current_indent < 0) current_indent = 0;

        int line_indent = is_bounce ? (current_indent > 0 ? current_indent - 1 : 0) : current_indent;
        if (tok_cnt > 0 && (is_label_line(tokens, tok_cnt) || ((tokens[0].kw == KW_REM || tokens[0].type == TOK_DOCSTRING) && current_indent == 0))) {
            line_indent = 0;
        }

        const char *body_text = skip_leading_ws(orig_text);
        char new_buf[1024];
        snprintf(new_buf, sizeof(new_buf), "%*s%s", line_indent * spaces_per, "", body_text);

        if (strcmp(orig_text, new_buf) != 0) {
            char buf[512];
            snprintf(buf, sizeof(buf), "  Line %lld: [BEFORE] %lld %s\n           [AFTER ] %lld %s\n",
                     (long long)line_num, (long long)line_num, orig_text, (long long)line_num, new_buf);
            vdev_puts(vd, buf);
            diff_shown++;
        }

        current_indent += post_adjust;
        if (current_indent < 0) current_indent = 0;
    }

    if (diff_shown == 0) {
        vdev_puts(vd, "  (No formatting changes required)\n");
    }
    vdev_puts(vd, "\n");

    vdev_puts(vd, "=== DIAGNOSTICS ===\n");
    if (plan->diag_count == 0) {
        vdev_puts(vd, "  No structural errors or warnings found.\n");
    } else {
        for (int d = 0; d < plan->diag_count; d++) {
            const ReformatDiagnostic *diag = &plan->diagnostics[d];
            const char *sev_str = (diag->severity == DIAG_ERROR) ? "ERROR" : "WARNING";
            char header[256];
            snprintf(header, sizeof(header), "  [%-7s] Line %lld: %s\n", sev_str, (long long)diag->line, diag->what);
            vdev_puts(vd, header);

            char detail[512];
            snprintf(detail, sizeof(detail), "            WHAT: %s\n            WHY:  %s\n            HOW:  %s\n\n",
                     diag->what, diag->why, diag->how);
            vdev_puts(vd, detail);
        }
        char count_buf[128];
        snprintf(count_buf, sizeof(count_buf), "  %d ERROR(s), %d WARNING(s)\n", plan->error_count, plan->warning_count);
        vdev_puts(vd, count_buf);
    }
    vdev_puts(vd, "\n");

    vdev_puts(vd, "=== SUMMARY ===\n");
    char sum_buf[512];
    snprintf(sum_buf, sizeof(sum_buf),
             "  Total lines:              %d\n"
             "  Lines to change:           %d\n"
             "  Lines unchanged:           %d\n"
             "  Indent width:               %d spaces\n\n"
             "  Diagnostics:\n"
             "    Errors:     %d\n"
             "    Warnings:   %d\n\n",
             plan->total_lines, diff_shown, plan->total_lines - diff_shown,
             plan->spaces_per_indent, plan->error_count, plan->warning_count);
    vdev_puts(vd, sum_buf);

    if (plan->error_count == 0 && plan->warning_count == 0) {
        vdev_puts(vd, "  STRICT mode ready:  YES\n  Recommend: REFORMAT ");
        char rec[32];
        snprintf(rec, sizeof(rec), "%d\n\n", plan->spaces_per_indent);
        vdev_puts(vd, rec);
    } else {
        char strict_buf[256];
        snprintf(strict_buf, sizeof(strict_buf),
                 "  STRICT mode ready:  NO\n"
                 "    %d error(s) and %d warning(s) must be resolved before STRICT will proceed.\n"
                 "    Tip: Fix the issues listed in DIAGNOSTICS, then run REFORMAT STRICT.\n\n",
                 plan->error_count, plan->warning_count);
        vdev_puts(vd, strict_buf);
    }

    if (plan->has_filename && strlen(plan->filename) > 0) {
        char chk_name[260];
        snprintf(chk_name, sizeof(chk_name), "%s", plan->filename);
        char *dot = strrchr(chk_name, '.');
        if (dot) *dot = '\0';
        strncat(chk_name, ".CHK", sizeof(chk_name) - strlen(chk_name) - 1);

        FILE *f = fopen(chk_name, "w");
        if (f) {
            fprintf(f, "REFORMAT CHECK Report for %s\n\n", plan->filename);
            fprintf(f, "Total Lines: %d, Diagnostics: %d Errors, %d Warnings\n",
                    plan->total_lines, plan->error_count, plan->warning_count);
            fprintf(f, "STRICT Ready: %s\n\n", (plan->error_count == 0 && plan->warning_count == 0) ? "YES" : "NO");
            for (int d = 0; d < plan->diag_count; d++) {
                fprintf(f, "[%s] Line %lld: %s\n  WHAT: %s\n  WHY:  %s\n  HOW:  %s\n\n",
                        plan->diagnostics[d].severity == DIAG_ERROR ? "ERROR" : "WARNING",
                        (long long)plan->diagnostics[d].line,
                        plan->diagnostics[d].what, plan->diagnostics[d].what,
                        plan->diagnostics[d].why, plan->diagnostics[d].how);
            }
            fclose(f);
            char report_msg[300];
            snprintf(report_msg, sizeof(report_msg), "  Report saved to: %s\n", chk_name);
            vdev_puts(vd, report_msg);
        } else {
            vdev_puts(vd, "  Report NOT saved (failed to open .CHK file for writing).\n");
        }
    } else {
        vdev_puts(vd, "  Report NOT saved (program has no filename — use SAVE first).\n");
    }
}

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
    if (!vm || !plan) return;
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

static void add_suggestion(ReformatPlan *plan, double line, const char *text, const char *reason) {
    if (!plan || plan->suggestion_count >= REFORMAT_MAX_SUGGESTIONS) return;
    ReformatSuggestion *s = &plan->suggestions[plan->suggestion_count++];
    s->line = line;
    snprintf(s->text, sizeof(s->text), "%s", text ? text : "");
    snprintf(s->reason, sizeof(s->reason), "%s", reason ? reason : "");
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

void reformat_render_suggestions_summary(VMContext *vm, const ReformatPlan *plan) {
    if (!vm || !plan) return;
    VDevContext *vd = vm_get_vdev(vm);
    if (plan->suggestion_count == 0) return;

    vdev_puts(vd, "=== SUGGESTIONS ===\n");
    for (int s = 0; s < plan->suggestion_count; s++) {
        const ReformatSuggestion *sug = &plan->suggestions[s];
        char buf[512];
        snprintf(buf, sizeof(buf), "  %d. Line %lld: %s\n     -> %s\n",
                 s + 1, (long long)sug->line, sug->text, sug->reason);
        vdev_puts(vd, buf);
    }
    char count_buf[128];
    snprintf(count_buf, sizeof(count_buf), "\n  %d suggestion(s) for manual review.\n\n", plan->suggestion_count);
    vdev_puts(vd, count_buf);
}
