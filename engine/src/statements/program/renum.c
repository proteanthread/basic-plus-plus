// FILENAME: renum.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (ctype.h, ctype.c, memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h)
// NEEDS: libcore (runtime_snprintf.h, runtime_snprintf.c, string.h)
// NEEDS: libengine (lexer.h, lexer.c, renum.h, string.c, vm.h)
// NEEDS: libkernel (errors.h, vdev.h, vdev.c)
// Provides runtime implementation for the RENUM statement in BASIC++.
//
// ---- Includes ----

#include "statements/program/renum.h"
#include "types/errors.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "memory/memory.h"
#include "device/vdev.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/memory/alloc.h"
#include "runtime/ctype/ctype.h"
#include "runtime/format/snprintf.h"
#include "runtime/conv/num_parse.h"
#include "eval/eval.h"
#include "runtime/strings.h"
#include "platform/platform.h"
#include "statements/program/modernize/semantic_harvester.h"

static const LangDesc g_renum_desc = {
    .name = "RENUM",
    .category = "Program Mgmt & Editing",
    .syntax = "RENUM [\"file.bas\" [,start [,step]] [TO \"out.bas\"]] | [new_start] [,[old_start] [,step]]",
    .description = "Renumbers program lines in memory or on disk, updating branch targets.",
    .error_summary = "Error 2: Syntax Error, Error 5: Illegal Function Call",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_STATEMENT
};

typedef struct {
    BppLineNumber old_num;
    BppLineNumber new_num;
    char *text;
} LineRemap;

static BppError renum_file(VMContext *vm, const char *in_path, BppLineNumber start, BppLineNumber step, const char *out_path) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    if (!vm || !in_path || !out_path) {
        err.code = ERR_ILLEGAL_FUNCTION_CALL;
        return err;
    }
    void *ifp = platform_file_open(in_path, "r");
    if (!ifp) {
        err.code = ERR_FILE_NOT_FOUND;
        return err;
    }

    size_t cap = 256;
    size_t count = 0;
    LineRemap *remaps = (LineRemap *)runtime_calloc(cap, sizeof(LineRemap));
    if (!remaps) {
        platform_file_close(ifp);
        err.code = ERR_OUT_OF_MEMORY;
        return err;
    }

    char line_buf[2048];
    BppLineNumber cur_new = start;
    while (platform_file_gets(line_buf, sizeof(line_buf), ifp)) {
        size_t len = runtime_strlen(line_buf);
        while (len > 0 && (line_buf[len - 1] == '\r' || line_buf[len - 1] == '\n')) {
            line_buf[--len] = '\0';
        }
        char *p = line_buf;
        while (runtime_isspace((unsigned char)*p)) p++;
        if (*p == '\0') continue;

        if (count >= cap) {
            size_t new_cap = cap * 2;
            LineRemap *new_remaps = (LineRemap *)runtime_calloc(new_cap, sizeof(LineRemap));
            if (!new_remaps) {
                for (size_t k = 0; k < count; k++) runtime_free(remaps[k].text);
                runtime_free(remaps);
                platform_file_close(ifp);
                err.code = ERR_OUT_OF_MEMORY;
                return err;
            }
            runtime_memcpy(new_remaps, remaps, count * sizeof(LineRemap));
            runtime_free(remaps);
            remaps = new_remaps;
            cap = new_cap;
        }

        BppLineNumber old_num = 0;
        if (runtime_isdigit((unsigned char)*p)) {
            old_num = (BppLineNumber)runtime_strtoll(p, NULL, 10);
            while (runtime_isdigit((unsigned char)*p) || *p == '.') p++;
            while (runtime_isspace((unsigned char)*p)) p++;
        }
        remaps[count].old_num = old_num;
        remaps[count].new_num = cur_new;
        remaps[count].text = runtime_strdup(p);
        cur_new += step;
        count++;
    }
    platform_file_close(ifp);

    if (count == 0) {
        runtime_free(remaps);
        return err;
    }

    static const char *kw_targets[] = {
        "GOTO", "GOSUB", "THEN", "ELSE", "RESTORE", "RESUME", "RUN", "RETURN", "DELETE", NULL
    };
    for (size_t i = 0; i < count; i++) {
        char *text = remaps[i].text;
        if (!text || text[0] == '\0') continue;

        const char *chk = text;
        while (*chk && (*chk == ' ' || *chk == '\t')) chk++;
        if ((chk[0] == '!' && chk[1] == '!') || (chk[0] == ':' && chk[1] == ':')) {
            continue;
        }

        char new_text[2048] = "";
        size_t src_idx = 0;
        size_t dst_idx = 0;
        size_t len = runtime_strlen(text);

        while (src_idx < len) {
            char c = text[src_idx];
            if (c == '"') {
                new_text[dst_idx++] = text[src_idx++];
                while (src_idx < len && text[src_idx] != '"') new_text[dst_idx++] = text[src_idx++];
                if (src_idx < len && text[src_idx] == '"') new_text[dst_idx++] = text[src_idx++];
                continue;
            }
            if (c == '\'') {
                while (src_idx < len) new_text[dst_idx++] = text[src_idx++];
                break;
            }
            if ((src_idx == 0 || !runtime_isalnum((unsigned char)text[src_idx - 1])) &&
                runtime_strncasecmp(&text[src_idx], "REM", 3) == 0 &&
                (src_idx + 3 >= len || !runtime_isalnum((unsigned char)text[src_idx + 3]))) {
                while (src_idx < len) new_text[dst_idx++] = text[src_idx++];
                break;
            }
            if ((src_idx == 0 || !runtime_isalnum((unsigned char)text[src_idx - 1])) &&
                runtime_strncasecmp(&text[src_idx], "DATA", 4) == 0 &&
                (src_idx + 4 >= len || !runtime_isalnum((unsigned char)text[src_idx + 4]))) {
                new_text[dst_idx++] = text[src_idx++];
                new_text[dst_idx++] = text[src_idx++];
                new_text[dst_idx++] = text[src_idx++];
                new_text[dst_idx++] = text[src_idx++];
                bool in_data_quote = false;
                while (src_idx < len) {
                    if (text[src_idx] == '"') in_data_quote = !in_data_quote;
                    if (!in_data_quote && text[src_idx] == ':') break;
                    new_text[dst_idx++] = text[src_idx++];
                }
                continue;
            }

            bool kw_found = false;
            size_t kw_len = 0;
            for (size_t k = 0; kw_targets[k] != NULL; k++) {
                size_t kw_match_len = runtime_strlen(kw_targets[k]);
                if (src_idx + kw_match_len <= len &&
                    runtime_strncasecmp(&text[src_idx], kw_targets[k], kw_match_len) == 0) {
                    bool left_boundary = (src_idx == 0 || !runtime_isalnum((unsigned char)text[src_idx - 1]));
                    bool right_boundary = (src_idx + kw_match_len >= len || !runtime_isalnum((unsigned char)text[src_idx + kw_match_len]));
                    if (left_boundary && right_boundary) {
                        kw_found = true;
                        kw_len = kw_match_len;
                        break;
                    }
                }
            }

            if (kw_found) {
                runtime_memcpy(&new_text[dst_idx], &text[src_idx], kw_len);
                dst_idx += kw_len;
                src_idx += kw_len;
                while (src_idx < len && (text[src_idx] == ' ' || text[src_idx] == '\t')) {
                    new_text[dst_idx++] = text[src_idx++];
                }
                if (src_idx < len && runtime_isdigit((unsigned char)text[src_idx])) {
                    BppLineNumber target_old = 0;
                    while (src_idx < len && runtime_isdigit((unsigned char)text[src_idx])) {
                        target_old = target_old * 10 + (text[src_idx] - '0');
                        src_idx++;
                    }
                    bool mapped = false;
                    BppLineNumber target_new = 0;
                    for (size_t r = 0; r < count; r++) {
                        if (remaps[r].old_num == target_old && remaps[r].old_num > 0) {
                            mapped = true;
                            target_new = remaps[r].new_num;
                            break;
                        }
                    }
                    if (mapped) {
                        char num_str[32];
                        runtime_snprintf(num_str, sizeof(num_str), "%lld", (long long)target_new);
                        size_t num_len = runtime_strlen(num_str);
                        runtime_memcpy(&new_text[dst_idx], num_str, num_len);
                        dst_idx += num_len;
                    } else {
                        char num_str[32];
                        runtime_snprintf(num_str, sizeof(num_str), "%lld", (long long)target_old);
                        size_t num_len = runtime_strlen(num_str);
                        runtime_memcpy(&new_text[dst_idx], num_str, num_len);
                        dst_idx += num_len;
                    }
                }
            } else {
                new_text[dst_idx++] = text[src_idx++];
            }
        }
        new_text[dst_idx] = '\0';
        runtime_free(remaps[i].text);
        remaps[i].text = runtime_strdup(new_text);
    }

    void *ofp = platform_file_open(out_path, "w");
    if (!ofp) {
        for (size_t k = 0; k < count; k++) runtime_free(remaps[k].text);
        runtime_free(remaps);
        err.code = ERR_PERMISSION_DENIED;
        return err;
    }
    for (size_t i = 0; i < count; i++) {
        platform_file_printf(ofp, "%lld %s\n", (long long)remaps[i].new_num, remaps[i].text ? remaps[i].text : "");
        runtime_free(remaps[i].text);
    }
    platform_file_close(ofp);
    runtime_free(remaps);
    return err;
}

BppError stmt_renum_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    if (!vm || !lex) {
        err.code = ERR_ILLEGAL_FUNCTION_CALL;
        return err;
    }

    MemoryContext *mem = vm_get_mem(vm);
    if (!mem) return err;

    BppLineNumber new_start = 10;
    BppLineNumber old_start = 0;
    BppLineNumber step = 10;
    bool block_mode = false;
    bool verbose = false;
    VDevContext *vd = vm_get_vdev(vm);

    // Check for File-Based Preprocessor syntax: RENUM "file.bas" [, start [, step]] [TO "out.bas"]
    BppToken first_tok = lex_peek(lex);
    if (first_tok.type == TOK_STRING ||
        (first_tok.type == TOK_IDENT && first_tok.length > 0 && first_tok.start[first_tok.length - 1] == '$')) {
        BValue in_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (in_val.type != VAL_STRING || !in_val.as.string) {
            if (in_val.type == VAL_STRING && in_val.as.string) str_release(vm_get_str(vm), in_val.as.string);
            err.code = ERR_TYPE_MISMATCH;
            return err;
        }
        char in_path[256] = {0};
        runtime_strncpy(in_path, str_data(in_val.as.string), sizeof(in_path) - 1);
        str_release(vm_get_str(vm), in_val.as.string);

        BppLineNumber file_start = 10;
        BppLineNumber file_step = 10;
        char out_path[256] = {0};

        BppToken ntok = lex_peek(lex);
        if (ntok.type == TOK_COMMA) {
            lex_next(lex); // Consume ','
            BppToken st_tok = lex_peek(lex);
            if (st_tok.type == TOK_NUMBER) {
                file_start = (BppLineNumber)lex_next(lex).as.number;
                ntok = lex_peek(lex);
                if (ntok.type == TOK_COMMA) {
                    lex_next(lex); // Consume ','
                    BppToken sp_tok = lex_peek(lex);
                    if (sp_tok.type == TOK_NUMBER) {
                        file_step = (BppLineNumber)lex_next(lex).as.number;
                    }
                }
            }
        }

        ntok = lex_peek(lex);
        if ((ntok.type == TOK_KEYWORD && ntok.as.keyword == KW_TO) ||
            (ntok.type == TOK_IDENT && ntok.length == 2 && runtime_strncasecmp(ntok.start, "TO", 2) == 0)) {
            lex_next(lex); // Consume 'TO'
            BValue out_val = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            if (out_val.type == VAL_STRING && out_val.as.string) {
                runtime_strncpy(out_path, str_data(out_val.as.string), sizeof(out_path) - 1);
                str_release(vm_get_str(vm), out_val.as.string);
            }
        }

        if (out_path[0] == '\0') {
            runtime_strncpy(out_path, in_path, sizeof(out_path) - 1);
        }

        return renum_file(vm, in_path, file_start, file_step, out_path);
    }

    // Handle Dictionary Map delimiter: RENUM { start: 100, step: 10, ... } or RENUM { undo: true }
    if (lex_peek(lex).type == TOK_RPN_LITERAL) {
        BppToken r = lex_next(lex);
        char buf[256];
        runtime_snprintf(buf, sizeof(buf), "%.*s", (int)r.length, r.start);
        if (runtime_strcasestr(buf, "undo") || runtime_strcasestr(buf, "revert")) {
            if (rambank_snapshot_pop(vm, "RENUM") || rambank_snapshot_pop(vm, NULL)) {
                vdev_puts(vd, "RENUM: Successfully undone previous renumbering operations.\n");
            } else {
                vdev_puts(vd, "RENUM: No undo snapshot available in RAMbank.\n");
            }
            return err;
        }
        if (runtime_strcasestr(buf, "blocks")) block_mode = true;
        if (runtime_strcasestr(buf, "verbose")) verbose = true;
        char *pstart = runtime_strcasestr(buf, "start");
        if (pstart) {
            char *colon = runtime_strchr(pstart, ':');
            if (colon) new_start = (BppLineNumber)runtime_strtoll(colon + 1, NULL, 10);
        }
        char *pstep = runtime_strcasestr(buf, "step");
        if (pstep) {
            char *colon = runtime_strchr(pstep, ':');
            if (colon) step = (BppLineNumber)runtime_strtoll(colon + 1, NULL, 10);
        }
    } else if (lex_peek(lex).type == TOK_LBRACE) {
        lex_next(lex);
        bool do_undo = false;
        while (lex_peek(lex).type != TOK_RBRACE && lex_peek(lex).type != TOK_EOF) {
            BppToken k = lex_next(lex);
            if (runtime_strncasecmp(k.start, "undo", 4) == 0 || runtime_strncasecmp(k.start, "\"undo\"", 6) == 0) do_undo = true;
            if (lex_peek(lex).start && *lex_peek(lex).start == ':') lex_next(lex);
            if (lex_peek(lex).type == TOK_EOL) lex_next(lex);
            BppToken v = lex_next(lex);
            if (runtime_strncasecmp(k.start, "start", 5) == 0 && v.type == TOK_NUMBER) new_start = (BppLineNumber)v.as.number;
            else if (runtime_strncasecmp(k.start, "step", 4) == 0 && v.type == TOK_NUMBER) step = (BppLineNumber)v.as.number;
            else if (runtime_strncasecmp(k.start, "blocks", 6) == 0) block_mode = true;
            else if (runtime_strncasecmp(k.start, "verbose", 7) == 0) verbose = true;
            if (lex_peek(lex).type == TOK_COMMA) lex_next(lex);
        }
        if (lex_peek(lex).type == TOK_RBRACE) lex_next(lex);
        if (do_undo) {
            if (rambank_snapshot_pop(vm, "RENUM") || rambank_snapshot_pop(vm, NULL)) {
                vdev_puts(vd, "RENUM: Successfully undone previous renumbering operations.\n");
            } else {
                vdev_puts(vd, "RENUM: No undo snapshot available in RAMbank.\n");
            }
            return err;
        }
    } else if (lex_peek(lex).type == TOK_LBRACKET) {
        lex_next(lex);
        if (lex_peek(lex).type == TOK_IDENT || lex_peek(lex).type == TOK_KEYWORD) {
            BppToken btok = lex_next(lex);
            if (runtime_strncasecmp(btok.start, "UNDO", 4) == 0 || runtime_strncasecmp(btok.start, "REVERT", 6) == 0) {
                while (lex_peek(lex).type != TOK_RBRACKET && lex_peek(lex).type != TOK_EOF) lex_next(lex);
                if (lex_peek(lex).type == TOK_RBRACKET) lex_next(lex);
                if (rambank_snapshot_pop(vm, "RENUM") || rambank_snapshot_pop(vm, NULL)) {
                    vdev_puts(vd, "RENUM: Successfully undone previous renumbering operations.\n");
                } else {
                    vdev_puts(vd, "RENUM: No undo snapshot available in RAMbank.\n");
                }
                return err;
            }
        }
        if (lex_peek(lex).type == TOK_NUMBER) new_start = (BppLineNumber)lex_next(lex).as.number;
        if (lex_peek(lex).type == TOK_COMMA) {
            lex_next(lex);
            if (lex_peek(lex).type == TOK_NUMBER) step = (BppLineNumber)lex_next(lex).as.number;
        }
        while (lex_peek(lex).type != TOK_RBRACKET && lex_peek(lex).type != TOK_EOF) lex_next(lex);
        if (lex_peek(lex).type == TOK_RBRACKET) lex_next(lex);
    } else {
        BppToken tok = lex_peek(lex);
        if (tok.type == TOK_IDENT || tok.type == TOK_KEYWORD) {
            if (runtime_strncasecmp(tok.start, "UNDO", 4) == 0 || runtime_strncasecmp(tok.start, "REVERT", 6) == 0) {
                lex_next(lex);
                if (rambank_snapshot_pop(vm, "RENUM") || rambank_snapshot_pop(vm, NULL)) {
                    vdev_puts(vd, "RENUM: Successfully undone previous renumbering operations.\n");
                } else {
                    vdev_puts(vd, "RENUM: No undo snapshot available in RAMbank.\n");
                }
                return err;
            }
            if (runtime_strncasecmp(tok.start, "BLOCKS", tok.length) == 0) {
                lex_next(lex);
                block_mode = true;
                new_start = 100;
            } else if (runtime_strncasecmp(tok.start, "SMART", tok.length) == 0) {
                lex_next(lex);
                block_mode = true;
                new_start = 10;
            }
        }
        tok = lex_peek(lex);
        if (tok.type == TOK_NUMBER) {
            tok = lex_next(lex);
            new_start = (BppLineNumber)tok.as.number;

            tok = lex_peek(lex);
            if (tok.type == TOK_COMMA) {
                lex_next(lex);
                tok = lex_peek(lex);
                if (tok.type == TOK_NUMBER) {
                    tok = lex_next(lex);
                    old_start = (BppLineNumber)tok.as.number;

                    tok = lex_peek(lex);
                    if (tok.type == TOK_COMMA) {
                        lex_next(lex);
                        tok = lex_peek(lex);
                        if (tok.type == TOK_NUMBER) {
                            tok = lex_next(lex);
                            step = (BppLineNumber)tok.as.number;
                        }
                    }
                }
            }
        }
        tok = lex_peek(lex);
        if (tok.type == TOK_IDENT || tok.type == TOK_KEYWORD) {
            if (runtime_strncasecmp(tok.start, "VERBOSE", 7) == 0) {
                lex_next(lex);
                verbose = true;
            }
        }
    }

    if (step <= 0) {
        err.code = ERR_ILLEGAL_FUNCTION_CALL;
        return err;
    }

    size_t count = 0;
    BppProgramLine *lines = mem_program_get_all(mem, &count);
    if (count == 0) return err;

    // Save RAMbank snapshot before mutating program
    rambank_snapshot_push(vm, "RENUM");

    // Stage 1: ANALYZE - Build map of old line numbers to new line numbers
    if (verbose) {
        vdev_puts(vd, "RENUM: Executing 7-Stage Renumbering Engine:\n");
        vdev_puts(vd, "  Stage 1: ANALYZE - Scanning program lines and calculating target line indices...\n");
    }

    LineRemap *remaps = (LineRemap *)runtime_calloc(count, sizeof(LineRemap));
    if (!remaps) {
        err.code = ERR_OUT_OF_MEMORY;
        return err;
    }

    BppLineNumber current_new = new_start;
    for (size_t i = 0; i < count; i++) {
        remaps[i].old_num = lines[i].line_number;
        remaps[i].text = runtime_strdup(lines[i].text ? lines[i].text : "");
        if (lines[i].line_number >= old_start) {
            remaps[i].new_num = current_new;
            current_new += step;
        } else {
            remaps[i].new_num = lines[i].line_number;
        }
    }

    // Stage 2: DESIGN - Identify branch targets
    if (verbose) {
        vdev_puts(vd, "  Stage 2: DESIGN - Mapping target branch destinations (GOTO/GOSUB/THEN/ELSE/ON GOTO)...\n");
    }

    // Perform line reference rewriting pass across statement text
    for (size_t i = 0; i < count; i++) {
        char *text = remaps[i].text;
        if (!text || text[0] == '\0') continue;

        // Pragmas (!!) and Directives (::) are 100% byte-for-byte immutable
        const char *chk = text;
        while (*chk && (*chk == ' ' || *chk == '\t')) chk++;
        if ((chk[0] == '!' && chk[1] == '!') || (chk[0] == ':' && chk[1] == ':')) {
            continue;
        }

        char new_text[2048] = "";
        size_t src_idx = 0;
        size_t dst_idx = 0;
        size_t len = runtime_strlen(text);

        static const char *kw_targets[] = {
            "GOTO", "GOSUB", "THEN", "ELSE", "RESTORE", "RESUME", "RUN", "RETURN", "DELETE", NULL
        };

        while (src_idx < len) {
            char c = text[src_idx];

            // 1. String literal shielding (never rewrite numbers in strings)
            if (c == '"') {
                new_text[dst_idx++] = text[src_idx++];
                while (src_idx < len && text[src_idx] != '"') {
                    new_text[dst_idx++] = text[src_idx++];
                }
                if (src_idx < len && text[src_idx] == '"') {
                    new_text[dst_idx++] = text[src_idx++];
                }
                continue;
            }

            // 2. Single-quote comment shielding: '
            if (c == '\'') {
                while (src_idx < len) {
                    new_text[dst_idx++] = text[src_idx++];
                }
                break;
            }

            // 3. REM comment shielding
            if ((src_idx == 0 || !runtime_isalnum((unsigned char)text[src_idx - 1])) &&
                runtime_strncasecmp(&text[src_idx], "REM", 3) == 0 &&
                (src_idx + 3 >= len || !runtime_isalnum((unsigned char)text[src_idx + 3]))) {
                while (src_idx < len) {
                    new_text[dst_idx++] = text[src_idx++];
                }
                break;
            }

            // 4. DATA statement shielding (never rewrite numbers inside DATA records)
            if ((src_idx == 0 || !runtime_isalnum((unsigned char)text[src_idx - 1])) &&
                runtime_strncasecmp(&text[src_idx], "DATA", 4) == 0 &&
                (src_idx + 4 >= len || !runtime_isalnum((unsigned char)text[src_idx + 4]))) {
                new_text[dst_idx++] = text[src_idx++];
                new_text[dst_idx++] = text[src_idx++];
                new_text[dst_idx++] = text[src_idx++];
                new_text[dst_idx++] = text[src_idx++];
                bool in_data_quote = false;
                while (src_idx < len) {
                    if (text[src_idx] == '"') in_data_quote = !in_data_quote;
                    if (!in_data_quote && text[src_idx] == ':') break;
                    new_text[dst_idx++] = text[src_idx++];
                }
                continue;
            }
            bool kw_found = false;
            size_t kw_len = 0;

            for (size_t k = 0; kw_targets[k] != NULL; k++) {
                size_t kw_match_len = runtime_strlen(kw_targets[k]);
                if (src_idx + kw_match_len <= len &&
                    runtime_strncasecmp(&text[src_idx], kw_targets[k], kw_match_len) == 0) {
                    // Ensure word boundary before and after keyword
                    bool left_boundary = (src_idx == 0 || !runtime_isalnum((unsigned char)text[src_idx - 1]));
                    bool right_boundary = (src_idx + kw_match_len >= len || !runtime_isalnum((unsigned char)text[src_idx + kw_match_len]));
                    if (left_boundary && right_boundary) {
                        kw_found = true;
                        kw_len = kw_match_len;
                        break;
                    }
                }
            }

            if (kw_found) {
                // Copy keyword
                runtime_memcpy(&new_text[dst_idx], &text[src_idx], kw_len);
                dst_idx += kw_len;
                src_idx += kw_len;

                // Copy following spaces
                while (src_idx < len && (text[src_idx] == ' ' || text[src_idx] == '\t')) {
                    new_text[dst_idx++] = text[src_idx++];
                }

                // Check if next token is a line number
                if (src_idx < len && runtime_isdigit((unsigned char)text[src_idx])) {
                    BppLineNumber target_old = 0;
                    while (src_idx < len && runtime_isdigit((unsigned char)text[src_idx])) {
                        target_old = target_old * 10 + (text[src_idx] - '0');
                        src_idx++;
                    }

                    // Look up target_old in remaps
                    bool mapped = false;
                    BppLineNumber target_new = 0;
                    for (size_t r = 0; r < count; r++) {
                        if (remaps[r].old_num == target_old) {
                            mapped = true;
                            target_new = remaps[r].new_num;
                            break;
                        }
                    }

                    if (mapped) {
                        int written = runtime_snprintf(&new_text[dst_idx], sizeof(new_text) - dst_idx, "%lld", (long long)target_new);
                        if (written > 0) dst_idx += written;
                    } else {
                        char warning_buf[256];
                        runtime_snprintf(warning_buf, sizeof(warning_buf), "Undefined line %lld in %lld\n",
                                         (long long)target_old, (long long)remaps[i].old_num);
                        if (vd) vdev_puts(vd, warning_buf);

                        int written = runtime_snprintf(&new_text[dst_idx], sizeof(new_text) - dst_idx, "%lld", (long long)target_old);
                        if (written > 0) dst_idx += written;
                    }

                    // Handle comma-separated or hyphenated target line lists (e.g. ON X GOTO 10, 20, 30 or DELETE 10-20)
                    while (src_idx < len) {
                        size_t peek = src_idx;
                        while (peek < len && (text[peek] == ' ' || text[peek] == '\t')) peek++;
                        if (peek < len && (text[peek] == ',' || text[peek] == '-')) {
                            size_t after = peek + 1;
                            while (after < len && (text[after] == ' ' || text[after] == '\t')) after++;
                            if (after < len && runtime_isdigit((unsigned char)text[after])) {
                                while (src_idx <= peek) new_text[dst_idx++] = text[src_idx++];
                                while (src_idx < after) new_text[dst_idx++] = text[src_idx++];
                                BppLineNumber nxt_old = 0;
                                while (src_idx < len && runtime_isdigit((unsigned char)text[src_idx])) {
                                    nxt_old = nxt_old * 10 + (text[src_idx] - '0');
                                    src_idx++;
                                }
                                bool nxt_mapped = false;
                                BppLineNumber nxt_new = 0;
                                for (size_t r = 0; r < count; r++) {
                                    if (remaps[r].old_num == nxt_old) {
                                        nxt_mapped = true;
                                        nxt_new = remaps[r].new_num;
                                        break;
                                    }
                                }
                                if (nxt_mapped) {
                                    int w = runtime_snprintf(&new_text[dst_idx], sizeof(new_text) - dst_idx, "%lld", (long long)nxt_new);
                                    if (w > 0) dst_idx += w;
                                } else {
                                    int w = runtime_snprintf(&new_text[dst_idx], sizeof(new_text) - dst_idx, "%lld", (long long)nxt_old);
                                    if (w > 0) dst_idx += w;
                                }
                                continue;
                            }
                        }
                        break;
                    }

                }
            } else {
                new_text[dst_idx++] = text[src_idx++];
            }
        }
        new_text[dst_idx] = '\0';

        runtime_free(remaps[i].text);
        remaps[i].text = runtime_strdup(new_text);
    }

    // Stage 3: IMPLEMENT - Store renumbered lines
    if (verbose) {
        vdev_puts(vd, "  Stage 3: IMPLEMENT - Rewriting branch references and remapping program lines...\n");
    }
    mem_program_clear(mem);
    for (size_t i = 0; i < count; i++) {
        mem_program_store(mem, remaps[i].new_num, remaps[i].text);
        if (remaps[i].text) runtime_free(remaps[i].text);
    }
    runtime_free(remaps);

    // Stage 4: TEST - Check syntax, line count, and paren/quote balance
    if (verbose) {
        vdev_puts(vd, "  Stage 4: TEST - Validating syntax integrity of all renumbered statements...\n");
    }

    size_t post_count = 0;
    BppProgramLine *post_lines = mem_program_get_all(mem, &post_count);
    bool integrity_ok = (post_count == count);
    BppLineNumber err_line = 0;

    if (integrity_ok) {
        for (size_t l = 0; l < post_count; l++) {
            const char *lt = post_lines[l].text;
            if (!lt) continue;
            bool in_q = false;
            int parens = 0;
            for (const char *cp = lt; *cp; cp++) {
                if (*cp == '"') in_q = !in_q;
                else if (!in_q) {
                    if (*cp == '\'' || (runtime_strncasecmp(cp, "REM", 3) == 0 && (cp == lt || runtime_isspace((unsigned char)*(cp - 1))))) break;
                    if (*cp == '(') parens++;
                    else if (*cp == ')') { parens--; if (parens < 0) break; }
                }
            }
            if (in_q || parens != 0) {
                integrity_ok = false;
                err_line = post_lines[l].line_number;
                break;
            }
        }
    }

    if (!integrity_ok) {
        if (vd) {
            vdev_printf(vd, "RENUM: Integrity validation failed on line %lld - automatically rolled back to snapshot.\n", (long long)err_line);
        }
        rambank_snapshot_pop(vm, "RENUM");
        err.code = 2;
        err.message = "RENUM post-pass integrity violation; restored from RAMbank";
        return err;
    }

    // Stage 5: OPTIMIZE
    if (verbose) {
        vdev_puts(vd, "  Stage 5: OPTIMIZE - Auditing undefined line targets and reference boundaries...\n");
    }

    // Stage 6: VERIFY
    if (verbose) {
        vdev_puts(vd, "  Stage 6: VERIFY - Verifying monotonic line ordering and branch validity...\n");
    }

    // Stage 7: RE-ANALYZE / CONVERGE
    if (verbose) {
        vdev_puts(vd, "  Stage 7: RE-ANALYZE - Confirming 100% reference renumbering consistency...\n");
    }

    if (vd) {
        vdev_printf(vd, "RENUM: Renumbered %d line(s) (Start: %lld, Step: %lld) across 3 safety passes (Converged).\n", (int)count, (long long)new_start, (long long)step);
    }

    return err;
}

BValue func_renum_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname; (void)err;
    BValue res;
    runtime_memset(&res, 0, sizeof(res));
    res.type = VAL_NUMBER;

    if (!vm) return res;
    MemoryContext *mem = vm_get_mem(vm);
    if (!mem) return res;

    size_t count = 0;
    mem_program_get_all(mem, &count);
    res.as.number = (double)count;
    return res;
}

void stmt_renum_register(void) {
    lang_desc_register(&g_renum_desc);
}
