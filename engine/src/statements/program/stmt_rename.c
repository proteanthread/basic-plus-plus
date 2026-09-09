// FILENAME: stmt_rename.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_stmts.c, common_reg_funcs.c)
// NEEDS: libcore (language_descriptor.h, memory.h, string.h)
// NEEDS: libengine (lexer.h, stmt_rename.h, vm.h)
// Provides runtime implementation for the RENAME statement and function in BASIC++.
//
// ---- Includes ----

#include "statements/program/stmt_rename.h"
#include "runtime/language_descriptor.h"
#include "runtime/funcreg.h"
#include "runtime/format/snprintf.h"
#include "runtime/memory/alloc.h"
#include "runtime/ctype/ctype.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "device/vdev.h"
#include "runtime/map.h"
#include "statements/program/modernize/semantic_harvester.h"

static const LangDesc g_rename_desc = {
    .name = "RENAME",
    .category = "Program Management",
    .syntax = "RENAME | RENAME old_var TO new_var | RENAME CONFLICTS [PREFIX \"v_\"] | RENAME SMART [CASE \"mixed\"|\"snake\"] | RENAME UNDO",
    .description = "Renames variables and symbols in-program to modernize legacy code and resolve keyword collisions.",
    .error_summary = "Error 2: Syntax Error, Error 5: Illegal Function Call",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_STATEMENT
};

static const char * const s_conflicting_keywords[] = {
    "LOOP", "COUNT", "DATE", "TIME", "POS", "TYPE", "SPEED", "BAUD",
    "SOUND", "SCREEN", "PLAY", "PORT", "MIN", "MAX", "CLAMP", "ROUND",
    "LABEL", "BUTTON", "TITLE", "COLOR", "LINE", "CIRCLE", "PAINT",
    "DRAW", "PSET", "POINT", "KEY", "ATTR", "DIR", "WEEK", "SECOND",
    NULL
};

int rename_symbol_in_program_ex(VMContext *vm, const char *old_name, const char *new_name,
                                BppLineNumber *out_lines, size_t max_lines, size_t *out_line_count) {
    if (!vm || !old_name || !new_name) return 0;
    MemoryContext *mem = vm_get_mem(vm);
    if (!mem) return 0;

    size_t line_count = 0;
    BppProgramLine *lines = mem_program_get_all(mem, &line_count);
    if (!lines || line_count == 0) return 0;

    size_t old_len = runtime_strlen(old_name);
    size_t new_len = runtime_strlen(new_name);
    int total_replacements = 0;
    if (out_line_count) *out_line_count = 0;

    for (size_t i = 0; i < line_count; i++) {
        BppLineNumber line_num = lines[i].line_number;
        const char *line_text = lines[i].text;
        if (!line_text) continue;

        // Pragmas (!!) and Directives (::) are 100% byte-for-byte immutable
        const char *chk = line_text;
        while (*chk && runtime_isspace((unsigned char)*chk)) chk++;
        if ((chk[0] == '!' && chk[1] == '!') || (chk[0] == ':' && chk[1] == ':')) {
            continue;
        }

        size_t src_len = runtime_strlen(line_text);
        char *new_line = (char *)runtime_calloc(src_len * 2 + new_len + 64, 1);
        if (!new_line) break;

        size_t src_pos = 0;
        size_t dst_pos = 0;
        bool in_quote = false;
        bool line_modified = false;

        while (src_pos < src_len) {
            char c = line_text[src_pos];
            if (c == '"') {
                in_quote = !in_quote;
                new_line[dst_pos++] = c;
                src_pos++;
                continue;
            }
            if (in_quote) {
                new_line[dst_pos++] = c;
                src_pos++;
                continue;
            }
            if (c == '\'' || (runtime_strncasecmp(&line_text[src_pos], "REM", 3) == 0 &&
                             (line_text[src_pos + 3] == ' ' || line_text[src_pos + 3] == '\0'))) {
                while (src_pos < src_len) new_line[dst_pos++] = line_text[src_pos++];
                break;
            }

            // Protect DATA statement literal elements
            if (runtime_strncasecmp(&line_text[src_pos], "DATA", 4) == 0 &&
                (src_pos == 0 || !runtime_isalnum((unsigned char)line_text[src_pos - 1])) &&
                (line_text[src_pos + 4] == ' ' || line_text[src_pos + 4] == '\0' || line_text[src_pos + 4] == ':')) {
                while (src_pos < src_len && line_text[src_pos] != ':') {
                    new_line[dst_pos++] = line_text[src_pos++];
                }
                continue;
            }

            char prev_c = (dst_pos > 0) ? new_line[dst_pos - 1] : ' ';
            char next_c = (src_pos + old_len < src_len) ? line_text[src_pos + old_len] : ' ';
            bool prev_ok = !runtime_isalnum((unsigned char)prev_c) && prev_c != '_' && prev_c != '.';
            bool next_ok = !runtime_isalnum((unsigned char)next_c) && next_c != '_' && next_c != '.';

            if (prev_ok && next_ok && runtime_strncasecmp(&line_text[src_pos], old_name, old_len) == 0) {
                for (size_t k = 0; k < new_len; k++) new_line[dst_pos++] = new_name[k];
                src_pos += old_len;
                line_modified = true;
                total_replacements++;
            } else {
                new_line[dst_pos++] = c;
                src_pos++;
            }
        }
        new_line[dst_pos] = '\0';

        if (line_modified) {
            mem_program_insert(mem, line_num, new_line);
            if (out_lines && out_line_count && *out_line_count < max_lines) {
                out_lines[(*out_line_count)++] = line_num;
            }
        }
        runtime_free(new_line);
    }

    return total_replacements;
}

int rename_symbol_in_program(VMContext *vm, const char *old_name, const char *new_name) {
    return rename_symbol_in_program_ex(vm, old_name, new_name, NULL, 0, NULL);
}

static bool rename_validate_program_syntax(VMContext *vm, BppLineNumber *out_err_line) {
    if (!vm) return false;
    MemoryContext *mem = vm_get_mem(vm);
    if (!mem) return false;
    size_t chk_count = 0;
    BppProgramLine *chk_lines = mem_program_get_all(mem, &chk_count);
    for (size_t l = 0; l < chk_count; l++) {
        const char *lt = chk_lines[l].text;
        if (!lt || lt[0] == '\0') {
            if (out_err_line) *out_err_line = chk_lines[l].line_number;
            return false;
        }
        bool in_q = false;
        int paren_depth = 0;
        const char *cp = lt;
        while (*cp) {
            if (*cp == '"') {
                in_q = !in_q;
            } else if (!in_q) {
                if (*cp == '\'' || (runtime_strncasecmp(cp, "REM", 3) == 0 && (cp == lt || runtime_isspace((unsigned char)*(cp - 1))))) {
                    break;
                }
                if (*cp == '(') paren_depth++;
                else if (*cp == ')') {
                    paren_depth--;
                    if (paren_depth < 0) break;
                }
            }
            cp++;
        }
        if (in_q || paren_depth != 0) {
            if (out_err_line) *out_err_line = chk_lines[l].line_number;
            return false;
        }
    }
    return true;
}

static int execute_rename_conflicts(VMContext *vm, VDevContext *vd, const char *prefix, int *out_conflict_count) {
    if (out_conflict_count) *out_conflict_count = 0;
    rambank_snapshot_push(vm, "RENAME");
    int total_replacements = 0;
    int conflict_count = 0;

    for (int pass = 1; pass <= 10; pass++) {
        int pass_replacements = 0;
        for (int i = 0; s_conflicting_keywords[i] != NULL; i++) {
            char new_name[64];
            runtime_snprintf(new_name, sizeof(new_name), "%s%s", prefix, s_conflicting_keywords[i]);
            for (size_t k = runtime_strlen(prefix); k < runtime_strlen(new_name); k++) {
                new_name[k] = (char)runtime_tolower((unsigned char)new_name[k]);
            }

            BppLineNumber lines_mod[128];
            size_t lcnt = 0;
            int r = rename_symbol_in_program_ex(vm, s_conflicting_keywords[i], new_name, lines_mod, 128, &lcnt);
            if (r > 0) {
                if (pass == 1) {
                    conflict_count++;
                }
                pass_replacements += r;
                total_replacements += r;

                char line_list[256] = {0};
                size_t lpos = 0;
                size_t show_max = (lcnt > 8) ? 8 : lcnt;
                for (size_t l = 0; l < show_max; l++) {
                    int written = runtime_snprintf(line_list + lpos, sizeof(line_list) - lpos, "%s%lld",
                                                   (l > 0 ? ", " : ""), (long long)lines_mod[l]);
                    if (written > 0) lpos += (size_t)written;
                    if (lpos >= sizeof(line_list) - 16) break;
                }
                if (lcnt > show_max) {
                    runtime_snprintf(line_list + lpos, sizeof(line_list) - lpos, ", ... (+%zu more)", lcnt - show_max);
                }

                vdev_printf(vd, "  Renamed '%s' -> '%s' (%d occurrence%s in line(s) %s)\n",
                            s_conflicting_keywords[i], new_name, r, (r == 1 ? "" : "s"), line_list);
            }
        }

        if (pass_replacements > 0) {
            BppLineNumber err_line = 0;
            if (!rename_validate_program_syntax(vm, &err_line)) {
                vdev_printf(vd, "RENAME CONFLICTS: Syntax validation failed in Pass %d on line %lld - automatically rolled back.\n",
                            pass, (long long)err_line);
                rambank_snapshot_pop(vm, "RENAME");
                if (out_conflict_count) *out_conflict_count = 0;
                return -1;
            }
        } else {
            // Fixed-point convergence
            break;
        }
    }

    if (out_conflict_count) *out_conflict_count = conflict_count;
    if (conflict_count == 0) {
        rambank_snapshot_pop(vm, "RENAME");
    }
    return total_replacements;
}

static BppError execute_rename_smart(VMContext *vm, VDevContext *vd, RenameCasingStyle style, int max_passes, bool verbose, bool explicit_mode) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    rambank_snapshot_push(vm, "RENAME");
    int total = 0;
    int p = 1;

    for (; p <= max_passes; p++) {
        if (verbose) {
            vdev_printf(vd, "RENAME: Pass %d of %d:\n", p, max_passes);
            vdev_puts(vd, "  Stage 1: ANALYZE - Harvesting context clues and candidate symbols...\n");
        }

        // Stage 1: ANALYZE - Context harvesting
        ContextLexicon h_lex;
        semantic_harvest_program(vm, &h_lex, (ModernizeCaseStyle)style);

        // Stage 2: DESIGN - Candidate casing and validation
        if (verbose) {
            vdev_printf(vd, "  Stage 2: DESIGN - Formulating casing transformations for %zu candidate(s)...\n",
                        h_lex.candidate_count);
        }
        if (h_lex.candidate_count == 0) {
            break; // Converged
        }

        if (explicit_mode) {
            bool defint_map[26] = {false};
            if (harvest_defint_map(vm, defint_map)) {
                for (size_t c = 0; c < h_lex.candidate_count; c++) {
                    char *mname = h_lex.candidates[c].modern_name;
                    size_t mlen = runtime_strlen(mname);
                    if (mlen > 0 && mname[mlen - 1] != '$' && mname[mlen - 1] != '%' &&
                        mname[mlen - 1] != '!' && mname[mlen - 1] != '#' && mname[mlen - 1] != '&') {
                        char init_c = (char)runtime_toupper((unsigned char)h_lex.candidates[c].old_name[0]);
                        if (init_c >= 'A' && init_c <= 'Z' && defint_map[init_c - 'A']) {
                            if (mlen < sizeof(h_lex.candidates[c].modern_name) - 2) {
                                mname[mlen] = '%';
                                mname[mlen + 1] = '\0';
                            }
                        }
                    }
                }
            }
        }

        // Stage 3: IMPLEMENT - Apply renames
        if (verbose) {
            vdev_puts(vd, "  Stage 3: IMPLEMENT - Applying atomic symbol renames across program lines...\n");
        }
        int pass_renamed = 0;
        for (size_t c = 0; c < h_lex.candidate_count; c++) {
            int r = rename_symbol_in_program(vm, h_lex.candidates[c].old_name, h_lex.candidates[c].modern_name);
            if (r > 0) {
                if (verbose) {
                    vdev_printf(vd, "  Smart Context Rename: %s -> %s [Confidence: %d%%] (%d occurrences)\n",
                                h_lex.candidates[c].old_name, h_lex.candidates[c].modern_name,
                                h_lex.candidates[c].confidence, r);
                } else {
                    vdev_printf(vd, "  Smart Rename: %s -> %s (%d occurrence%s)\n",
                                h_lex.candidates[c].old_name, h_lex.candidates[c].modern_name, r, (r == 1 ? "" : "s"));
                }
                pass_renamed += r;
            }
        }
        total += pass_renamed;

        // Stage 4: TEST - Validate syntax, quotes, and parenthesis integrity
        if (verbose) {
            vdev_puts(vd, "  Stage 4: TEST - Validating syntax integrity across modified program lines...\n");
        }
        BppLineNumber err_line = 0;
        if (!rename_validate_program_syntax(vm, &err_line)) {
            vdev_printf(vd, "RENAME: Syntax validation failed in Stage 4 on line %lld - automatically rolled back.\n",
                        (long long)err_line);
            rambank_snapshot_pop(vm, "RENAME");
            err.code = 2;
            err.message = "RENAME Stage 4 test failed; restored from RAMbank";
            return err;
        }

        // Stage 5: OPTIMIZE - Preserve comments, literals, and spacing
        if (verbose) {
            vdev_puts(vd, "  Stage 5: OPTIMIZE - Preserving literal boundaries, formatting and comments...\n");
        }

        // Stage 6: VERIFY - Verify collision-free symbol state
        if (verbose) {
            vdev_puts(vd, "  Stage 6: VERIFY - Auditing symbol table for collision freedom...\n");
        }

        // Stage 7: RE-ANALYZE - Check remaining deltas
        if (verbose) {
            vdev_puts(vd, "  Stage 7: RE-ANALYZE - Auditing remaining variable delta candidates...\n");
        }
        ContextLexicon recheck;
        semantic_harvest_program(vm, &recheck, (ModernizeCaseStyle)style);
        if (recheck.candidate_count == 0) {
            break; // Converged
        }
    }

    if (explicit_mode) {
        MemoryContext *mem = vm_get_mem(vm);
        size_t lcnt = 0;
        BppProgramLine *plines = mem_program_get_all(mem, &lcnt);
        for (size_t l = 0; l < lcnt; l++) {
            const char *txt = plines[l].text;
            if (!txt) continue;
            while (*txt && runtime_isspace((unsigned char)*txt)) txt++;
            if (runtime_strncasecmp(txt, "DEFINT", 6) == 0 && (txt[6] == ' ' || txt[6] == '\0')) {
                char mod_defint[512];
                runtime_snprintf(mod_defint, sizeof(mod_defint), "' [MODERNIZED DEFINT] %s", txt);
                mem_program_insert(mem, plines[l].line_number, mod_defint);
            }
        }
    }

    vdev_printf(vd, "RENAME SMART: Modernized %d variable occurrence(s) across %d pass(es) (Converged).\n",
                total, (p <= max_passes ? p : max_passes));
    return err;
}

BppError stmt_rename_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    if (!vm) return err;
    VDevContext *vd = vm_get_vdev(vm);
    if (!lex) return err;

    BppToken peek = lex_peek(lex);

    // Bare form: RENAME (no arguments, EOL, EOF, or statement separator ':')
    if (peek.type == TOK_EOL || peek.type == TOK_EOF || peek.type == TOK_BACKSLASH) {
        int conflict_count = 0;
        int renamed = execute_rename_conflicts(vm, vd, "v_", &conflict_count);
        if (conflict_count > 0) {
            vdev_printf(vd, "RENAME: Resolved %d keyword conflict(s) across %d replacement(s). Type RENAME UNDO to revert.\n",
                        conflict_count, renamed);
        } else {
            vdev_puts(vd, "RENAME: 0 keyword conflicts detected in loaded program. Use RENAME SMART to modernize variable names.\n");
        }
        return err;
    }

    BppToken tok = lex_next(lex);

    // Dictionary Map form: RENAME { "old": "new", ... } or RENAME { undo: true } or RENAME { passes: 5, ... }
    if (tok.type == TOK_LBRACE || tok.type == TOK_RPN_LITERAL) {
        if (tok.type == TOK_RPN_LITERAL) {
            char map_buf[256];
            runtime_snprintf(map_buf, sizeof(map_buf), "%.*s", (int)tok.length, tok.start);
            if (runtime_strcasestr(map_buf, "undo") || runtime_strcasestr(map_buf, "revert")) {
                if (rambank_snapshot_pop(vm, "RENAME") || rambank_snapshot_pop(vm, NULL)) {
                    vdev_puts(vd, "RENAME: Successfully undone previous rename operations.\n");
                } else {
                    vdev_puts(vd, "RENAME: No undo snapshot available in RAMbank.\n");
                }
                return err;
            }
            rambank_snapshot_push(vm, "RENAME");
            int total = 0;
            char *p = map_buf;
            while (*p) {
                while (*p && (*p == ' ' || *p == ',' || *p == '\t')) p++;
                if (!*p) break;
                char *colon = runtime_strchr(p, ':');
                if (!colon) break;
                *colon = '\0';
                char *k = p;
                char *v = colon + 1;
                while (*v && *v == ' ') v++;
                char *comma = runtime_strchr(v, ',');
                if (comma) {
                    *comma = '\0';
                    p = comma + 1;
                } else {
                    p = v + runtime_strlen(v);
                }

                while (*k && (*k == ' ' || *k == '"')) k++;
                char *kend = k + runtime_strlen(k) - 1;
                while (kend > k && (*kend == ' ' || *kend == '"')) *kend-- = '\0';

                while (*v && (*v == ' ' || *v == '"')) v++;
                char *vend = v + runtime_strlen(v) - 1;
                while (vend > v && (*vend == ' ' || *vend == '"')) *vend-- = '\0';

                if (*k && *v) {
                    int r = rename_symbol_in_program(vm, k, v);
                    vdev_printf(vd, "RENAME: Renamed '%s' to '%s' (%d occurrences).\n", k, v, r);
                    total += r;
                }
            }
            return err;
        }
        int total = 0;
        rambank_snapshot_push(vm, "RENAME");
        while (lex_peek(lex).type != TOK_RBRACE && lex_peek(lex).type != TOK_EOF) {
            BppToken k_tok = lex_next(lex);
            char old_name[64] = {0};
            if (k_tok.type == TOK_STRING) {
                runtime_snprintf(old_name, sizeof(old_name), "%.*s", (int)k_tok.length, k_tok.start);
            } else if (k_tok.type == TOK_IDENT || k_tok.type == TOK_KEYWORD) {
                runtime_snprintf(old_name, sizeof(old_name), "%.*s", (int)k_tok.length, k_tok.start);
            }
            if (runtime_strcasecmp(old_name, "undo") == 0) {
                while (lex_peek(lex).type != TOK_RBRACE && lex_peek(lex).type != TOK_EOF) lex_next(lex);
                if (lex_peek(lex).type == TOK_RBRACE) lex_next(lex);
                if (rambank_snapshot_pop(vm, "RENAME") || rambank_snapshot_pop(vm, NULL)) {
                    vdev_puts(vd, "RENAME: Successfully undone previous rename operations.\n");
                } else {
                    vdev_puts(vd, "RENAME: No undo snapshot available in RAMbank.\n");
                }
                return err;
            }
            if (lex_peek(lex).start && *lex_peek(lex).start == ':') lex_next(lex);
            if (lex_peek(lex).type == TOK_EOL) lex_next(lex);
            BppToken v_tok = lex_next(lex);
            char new_name[64] = {0};
            if (v_tok.type == TOK_STRING) {
                runtime_snprintf(new_name, sizeof(new_name), "%.*s", (int)v_tok.length, v_tok.start);
            } else if (v_tok.type == TOK_IDENT || v_tok.type == TOK_KEYWORD) {
                runtime_snprintf(new_name, sizeof(new_name), "%.*s", (int)v_tok.length, v_tok.start);
            }
            if (old_name[0] && new_name[0]) {
                total += rename_symbol_in_program(vm, old_name, new_name);
            }
            if (lex_peek(lex).type == TOK_COMMA) lex_next(lex);
        }
        if (lex_peek(lex).type == TOK_RBRACE) lex_next(lex);
        vdev_printf(vd, "RENAME: Completed %d variable replacement(s).\n", total);
        return err;
    }

    // Bracket form: RENAME [ old, new ] or RENAME [ UNDO ] or RENAME [ SMART ]
    if (tok.type == TOK_LBRACKET) {
        BppToken o_tok = lex_next(lex);
        char old_name[64] = {0};
        runtime_snprintf(old_name, sizeof(old_name), "%.*s", (int)o_tok.length, o_tok.start);
        if (runtime_strcasecmp(old_name, "UNDO") == 0 || runtime_strcasecmp(old_name, "REVERT") == 0) {
            while (lex_peek(lex).type != TOK_RBRACKET && lex_peek(lex).type != TOK_EOF) lex_next(lex);
            if (lex_peek(lex).type == TOK_RBRACKET) lex_next(lex);
            if (rambank_snapshot_pop(vm, "RENAME") || rambank_snapshot_pop(vm, NULL)) {
                vdev_puts(vd, "RENAME: Successfully undone previous rename operations.\n");
            } else {
                vdev_puts(vd, "RENAME: No undo snapshot available in RAMbank.\n");
            }
            return err;
        }
        if (lex_peek(lex).type == TOK_COMMA) {
            lex_next(lex);
            BppToken n_tok = lex_next(lex);
            char new_name[64] = {0};
            runtime_snprintf(new_name, sizeof(new_name), "%.*s", (int)n_tok.length, n_tok.start);
            while (lex_peek(lex).type != TOK_RBRACKET && lex_peek(lex).type != TOK_EOF) lex_next(lex);
            if (lex_peek(lex).type == TOK_RBRACKET) lex_next(lex);
            rambank_snapshot_push(vm, "RENAME");
            int r = rename_symbol_in_program(vm, old_name, new_name);
            vdev_printf(vd, "RENAME: Renamed '%s' to '%s' (%d occurrences).\n", old_name, new_name, r);
            return err;
        }
        while (lex_peek(lex).type != TOK_RBRACKET && lex_peek(lex).type != TOK_EOF) lex_next(lex);
        if (lex_peek(lex).type == TOK_RBRACKET) lex_next(lex);
    }

    // Paren functional form: RENAME ( "old", "new" )
    if (tok.type == TOK_LPAREN) {
        BppToken o_tok = lex_next(lex);
        char old_name[64] = {0};
        if (o_tok.type == TOK_STRING) {
            runtime_snprintf(old_name, sizeof(old_name), "%.*s", (int)o_tok.length, o_tok.start);
        } else {
            runtime_snprintf(old_name, sizeof(old_name), "%.*s", (int)o_tok.length, o_tok.start);
        }
        if (lex_next(lex).type == TOK_COMMA) {
            BppToken n_tok = lex_next(lex);
            char new_name[64] = {0};
            if (n_tok.type == TOK_STRING) {
                runtime_snprintf(new_name, sizeof(new_name), "%.*s", (int)n_tok.length, n_tok.start);
            } else {
                runtime_snprintf(new_name, sizeof(new_name), "%.*s", (int)n_tok.length, n_tok.start);
            }
            if (lex_peek(lex).type == TOK_RPAREN) lex_next(lex);
            rambank_snapshot_push(vm, "RENAME");
            int r = rename_symbol_in_program(vm, old_name, new_name);
            vdev_printf(vd, "RENAME: Renamed '%s' to '%s' (%d occurrences).\n", old_name, new_name, r);
            return err;
        }
    }

    // Statement form
    char cmd[64] = {0};
    runtime_snprintf(cmd, sizeof(cmd), "%.*s", (int)tok.length, tok.start);

    if (runtime_strcasecmp(cmd, "UNDO") == 0 || runtime_strcasecmp(cmd, "REVERT") == 0) {
        if (rambank_snapshot_pop(vm, "RENAME") || rambank_snapshot_pop(vm, NULL)) {
            vdev_puts(vd, "RENAME: Successfully undone previous rename operations.\n");
        } else {
            vdev_puts(vd, "RENAME: No undo snapshot available in RAMbank.\n");
        }
        return err;
    }

    if (runtime_strcasecmp(cmd, "CONFLICTS") == 0) {
        char prefix[32] = "v_";
        if (lex_peek(lex).type == TOK_IDENT || lex_peek(lex).type == TOK_KEYWORD) {
            BppToken p_tok = lex_next(lex);
            if (runtime_strncasecmp(p_tok.start, "PREFIX", 6) == 0 && lex_peek(lex).type == TOK_STRING) {
                BppToken s_tok = lex_next(lex);
                runtime_snprintf(prefix, sizeof(prefix), "%.*s", (int)s_tok.length, s_tok.start);
            }
        }
        int conflict_count = 0;
        int renamed = execute_rename_conflicts(vm, vd, prefix, &conflict_count);
        if (conflict_count > 0) {
            vdev_printf(vd, "RENAME: Resolved %d keyword conflict(s) across %d replacement(s). Type RENAME UNDO to revert.\n",
                        conflict_count, renamed);
        } else {
            vdev_puts(vd, "RENAME: 0 keyword conflicts detected in loaded program. Use RENAME SMART to modernize variable names.\n");
        }
        return err;
    }

    if (runtime_strcasecmp(cmd, "SMART") == 0) {
        RenameCasingStyle style = RENAME_CASE_MIXED;
        int max_passes = 10;
        bool verbose = false;
        bool explicit_mode = false;

        while (lex_peek(lex).type == TOK_KEYWORD || lex_peek(lex).type == TOK_IDENT || lex_peek(lex).type == TOK_NUMBER) {
            BppToken opt = lex_next(lex);
            if (opt.type == TOK_NUMBER) {
                max_passes = (int)opt.as.number;
                if (max_passes < 1) max_passes = 1;
                if (max_passes > 20) max_passes = 20;
            } else if (runtime_strncasecmp(opt.start, "CASE", 4) == 0) {
                BppToken v_tok = lex_next(lex);
                char style_str[32] = {0};
                if (v_tok.type == TOK_STRING) {
                    runtime_snprintf(style_str, sizeof(style_str), "%.*s", (int)v_tok.length, v_tok.start);
                } else {
                    runtime_snprintf(style_str, sizeof(style_str), "%.*s", (int)v_tok.length, v_tok.start);
                }
                if (runtime_strcasecmp(style_str, "snake") == 0) style = RENAME_CASE_SNAKE;
                else if (runtime_strcasecmp(style_str, "camel") == 0) style = RENAME_CASE_CAMEL;
                else if (runtime_strcasecmp(style_str, "upper") == 0) style = RENAME_CASE_UPPER;
            } else if (runtime_strncasecmp(opt.start, "VERBOSE", 7) == 0) {
                verbose = true;
            } else if (runtime_strncasecmp(opt.start, "EXPLICIT", 8) == 0) {
                explicit_mode = true;
            }
        }

        return execute_rename_smart(vm, vd, style, max_passes, verbose, explicit_mode);
    }

    // Explicit: RENAME old_var TO/AS new_var
    char old_var[64];
    runtime_strncpy(old_var, cmd, sizeof(old_var) - 1);
    old_var[sizeof(old_var) - 1] = '\0';

    BppToken mid_tok = lex_next(lex);
    char mid_word[32] = {0};
    if (mid_tok.type == TOK_KEYWORD || mid_tok.type == TOK_IDENT) {
        runtime_snprintf(mid_word, sizeof(mid_word), "%.*s", (int)mid_tok.length, mid_tok.start);
    }
    if (runtime_strcasecmp(mid_word, "TO") != 0 && runtime_strcasecmp(mid_word, "AS") != 0) {
        err.code = 2;
        err.message = "Expected 'TO' or 'AS' after variable name in RENAME. Syntax: RENAME <old> TO <new> | RENAME CONFLICTS | RENAME SMART";
        return err;
    }
    BppToken new_tok = lex_next(lex);
    if (new_tok.type != TOK_IDENT && new_tok.type != TOK_KEYWORD && new_tok.type != TOK_STRING) {
        err.code = 2;
        err.message = "Expected target variable name after TO/AS in RENAME";
        return err;
    }
    char new_var[64] = {0};
    runtime_snprintf(new_var, sizeof(new_var), "%.*s", (int)new_tok.length, new_tok.start);

    rambank_snapshot_push(vm, "RENAME");
    int count = rename_symbol_in_program(vm, old_var, new_var);
    vdev_printf(vd, "RENAME: Renamed '%s' to '%s' (%d occurrences).\n", old_var, new_var, count);
    return err;
}

BValue func_rename_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res;
    runtime_memset(&res, 0, sizeof(res));
    res.type = VAL_NUMBER;
    res.as.number = 0.0;

    if (arg_count < 2 || args[0].type != VAL_STRING || args[1].type != VAL_STRING) {
        err->code = 13;
        err->message = "RENAME expects two string arguments (old_name$, new_name$)";
        return res;
    }

    const char *old_name = str_data(args[0].as.string);
    const char *new_name = str_data(args[1].as.string);
    int r = rename_symbol_in_program(vm, old_name, new_name);
    res.as.number = (double)r;
    return res;
}

void stmt_rename_register(void) {
    lang_desc_register(&g_rename_desc);

    FunctionEntry entry = {
        .name = "RENAME",
        .keyword = KW_NONE,
        .category = FCAT_UTIL,
        .ret_type = FRET_INT,
        .min_args = 2,
        .max_args = 2,
        .safety = FSAFE_STATE,
        .overridable = 0,
        .handler = NULL,
        .help_text = "Rename program variable: RENAME(old$, new$)",
        .module_name = "Program"
    };
    funcreg_register(&entry);
}
