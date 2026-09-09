// FILENAME: help.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (eval.h, help.h, spec.h)
// Provides runtime implementation for the HELP and CATALOG statements in BASIC++.

#include "statements/introspection/help.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "device/vdev.h"
#include "runtime/metadata.h"
#include "module/module.h"
#include "runtime/funcreg.h"
#include "runtime/spec.h"
#include "runtime/language_descriptor.h"
#include "eval/eval_internal.h"
#include "hal/hal.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"
#include "runtime/format/snprintf.h"

static const LangDesc g_help_desc = {
    .name = "HELP",
    .category = "Introspection",
    .syntax = "HELP [keyword | command | block_target]",
    .description = "Displays interactive help documentation for statements, functions, syntax, and system components.",
    .error_summary = "Error 2: Syntax Error",
    .subsystem = SUBSYSTEM_CORE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_STATEMENT
};

static const LangDesc g_catalog_desc = {
    .name = "CATALOG",
    .category = "Introspection",
    .syntax = "CATALOG [category_name | keyword1 [, keyword2...]]",
    .description = "Displays interactive catalog listing of keyword categories, dynamic modules, global labels, custom blocks, and specifications.",
    .error_summary = "Error 2: Syntax Error",
    .subsystem = SUBSYSTEM_CORE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_STATEMENT
};

void stmt_help_register(void) {
    lang_desc_register(&g_help_desc);
    lang_desc_register(&g_catalog_desc);
}

static void resolve_relative_path(VMContext *vm, const char *rel_path, char *out_path, size_t max_len) {
    const char *curr_file = vm_get_current_filename(vm);
    if (curr_file && curr_file[0] != '\0') {
        const char *last_slash = runtime_strrchr(curr_file, '/');
        const char *last_backslash = runtime_strrchr(curr_file, '\\');
        const char *slash = (last_slash > last_backslash) ? last_slash : last_backslash;
        if (slash) {
            size_t dir_len = (size_t)(slash - curr_file + 1);
            if (dir_len < max_len - 1) {
                runtime_memcpy(out_path, curr_file, dir_len);
                out_path[dir_len] = '\0';
                runtime_strncat(out_path, rel_path, max_len - dir_len - 1);
                return;
            }
        }
    }
    runtime_strncpy(out_path, rel_path, max_len - 1);
    out_path[max_len - 1] = '\0';
}

static bool extract_help_file(const char *body, char *out_file, size_t max_len) {
    if (!body || body[0] == '\0') return false;
    const char *p = body;
    while (*p) {
        while (*p && (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')) {
            p++;
        }
        if (*p == '\0') break;

        bool found = false;
        int word_len = 0;
        if (runtime_strncasecmp(p, "HELPFILE", 8) == 0) {
            word_len = 8;
            found = true;
        } else if (runtime_strncasecmp(p, "DOCFILE", 7) == 0) {
            word_len = 7;
            found = true;
        }

        if (found) {
            p += word_len;
            while (*p && (*p == ' ' || *p == '\t')) p++;
            char quote = '\0';
            if (*p == '"' || *p == '\'') {
                quote = *p;
                p++;
            }
            size_t idx = 0;
            while (*p && idx < max_len - 1) {
                if (quote) {
                    if (*p == quote) {
                        p++;
                        break;
                    }
                } else {
                    if (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') {
                        break;
                    }
                }
                out_file[idx++] = *p++;
            }
            out_file[idx] = '\0';
            return (idx > 0);
        }

        while (*p && *p != '\n') p++;
    }
    return false;
}

static void display_baud_speed_table(VDevContext *vdev) {
    vdev_printf(vdev, "\nSupported Transmission Rates (BAUD / SPEED&):\n");
    vdev_printf(vdev, "  45.45   Baudot 60 WPM / 5-bit teletype standard\n");
    vdev_printf(vdev, "  50.0    Baudot 66 WPM European standard\n");
    vdev_printf(vdev, "  56.88   Baudot 75 WPM\n");
    vdev_printf(vdev, "  74.2    Vintage RTTY (45.45-74.2 baud)\n");
    vdev_printf(vdev, "  75.0    Standard RTTY 100 WPM\n");
    vdev_printf(vdev, "  110.0   Bell 101 / Teletype Model 33 ASR\n");
    vdev_printf(vdev, "  134.5   IBM 2741 Selectric Terminal\n");
    vdev_printf(vdev, "  150, 300, 600, 1200, 2400, 4800, 9600, 14400, 19200,\n");
    vdev_printf(vdev, "  28800, 33600, 38400, 57600, 115200 bps up to broadband\n");
    vdev_printf(vdev, "  0       Unthrottled maximum host execution speed\n\n");
    vdev_printf(vdev, "SPEED%% vs SPEED&:\n");
    vdev_printf(vdev, "  SPEED%% (0..255) : Apple II relative host delay byte (varies with host speed)\n");
    vdev_printf(vdev, "  SPEED& (bps)    : Absolute baud transmission rate matching BAUD(0)\n");
}

static void display_help_for_target(VMContext *vm, VDevContext *vdev, BppMetadataRegistry *reg, const char *target) {
    if (!target || target[0] == '\0') return;

    const char *doc = metadata_get_docstring(reg, target);
    if (doc && doc[0] != '\0') {
        vdev_printf(vdev, "Documentation for '%s':\n%s\n", target, doc);
        return;
    }

    const BppMetadataBlock *block = metadata_get_block(reg, "KEYWORD", target);
    if (!block) block = metadata_get_block(reg, "SCOPE", target);
    if (!block) block = metadata_get_block(reg, "ALIAS", target);
    if (!block) block = metadata_get_block(reg, "OPTION", target);

    if (block) {
        char rel_file[256];
        if (extract_help_file(block->body, rel_file, sizeof(rel_file))) {
            char file_path[512];
            resolve_relative_path(vm, rel_file, file_path, sizeof(file_path));
            HalContext *hal = hal_get();
            if (hal && hal->io.file_open && hal->io.file_close) {
                IoHandle fp = hal->io.file_open(file_path, "rb");
                if (fp != IO_HANDLE_INVALID) {
                    vdev_printf(vdev, "[%s BLOCK: %s (Loaded from %s)]\n", block->block_type, block->target_name, rel_file);
                    char read_buf[512];
                    size_t read_bytes = 0;
                    while ((read_bytes = hal->io.file_read(fp, read_buf, 1, sizeof(read_buf) - 1)) > 0) {
                        read_buf[read_bytes] = '\0';
                        vdev_printf(vdev, "%s", read_buf);
                    }
                    vdev_printf(vdev, "\n");
                    hal->io.file_close(fp);
                    return;
                }
            }
        }

        vdev_printf(vdev, "[%s BLOCK: %s]\nDocstring: %s\nBody:\n%s\n", 
                    block->block_type, block->target_name, 
                    block->docstring[0] ? block->docstring : "(None)", 
                    block->body[0] ? block->body : "(Empty)");
        return;
    }

    const LanguageDescriptor *desc = lang_desc_find(target);
    if (desc) {
        vdev_printf(vdev, "Category:    %s\n", desc->category ? desc->category : "");
        vdev_printf(vdev, "Syntax:      %s\n", desc->syntax ? desc->syntax : "");
        vdev_printf(vdev, "Help:        %s\n", desc->description ? desc->description : "");
        if (desc->error_summary && desc->error_summary[0] != '\0') {
            vdev_printf(vdev, "Errors:      %s\n", desc->error_summary);
        }
        if (desc->examples && desc->examples[0] != '\0') {
            vdev_printf(vdev, "Example:     %s\n", desc->examples);
        }
        if (runtime_strncasecmp(target, "BAUD", 4) == 0 || runtime_strncasecmp(target, "SPEED", 5) == 0) {
            display_baud_speed_table(vdev);
        }
        return;
    }

    const FunctionEntry *entry = funcreg_find_by_name(target);
    if (entry) {
        vdev_printf(vdev, "Function: %s\nHelp:     %s\n", entry->name, entry->help_text);
        return;
    }

    SpecObject *spec = spec_find_by_name(target);
    if (spec) {
        vdev_printf(vdev, "Dynamic Feature: %s\n", spec->name);
        vdev_printf(vdev, "Category:        %s\n", spec->category == SPEC_CAT_STATEMENT ? "STATEMENT" : "FUNCTION");
        vdev_printf(vdev, "Version:         %s\n", spec->version[0] ? spec->version : "1.0");
        vdev_printf(vdev, "Security Level:  %s\n", spec->required_level);
        vdev_printf(vdev, "Companion Lib:   %s\n", spec->lib_path[0] ? spec->lib_path : "None");
        return;
    }

    const LanguageDescriptor *results[512];
    int match_count = lang_desc_query_category(target, results, 512);
    if (match_count > 0) {
        vdev_printf(vdev, "Category: %s\nKeywords:\n  ", target);
        for (int i = 0; i < match_count; i++) {
            vdev_printf(vdev, "%-14s", results[i]->name ? results[i]->name : "");
            if ((i + 1) % 5 == 0 && i + 1 < match_count) {
                vdev_printf(vdev, "\n  ");
            }
        }
        vdev_printf(vdev, "\n");
        return;
    }

    vdev_printf(vdev, "No documentation or category found for target '%s'\n", target);
}

BppError stmt_help_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    VDevContext *vdev = vm_get_vdev(vm);
    BppMetadataRegistry *reg = vm_get_metadata(vm);

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_EOL || tok.type == TOK_EOF) {
        vdev_printf(vdev, "BASIC++ Interactive Help System\n");
        vdev_printf(vdev, "Usage:\n");
        vdev_printf(vdev, "  HELP            - Show this general help text.\n");
        vdev_printf(vdev, "  HELP <target>   - Query docstring/block/function documentation.\n");
        vdev_printf(vdev, "  CATALOG         - List active modules, global labels, and custom blocks.\n\n");

        vdev_printf(vdev, "[Built-in Keywords by Category]\n");
        char categories[64][64];
        int cat_count = lang_desc_get_categories(categories, 64);

        for (int c = 0; c < cat_count; c++) {
            vdev_printf(vdev, "  %s:\n    ", categories[c]);
            const LanguageDescriptor *results[512];
            int kw_count = lang_desc_query_category(categories[c], results, 512);
            int line_len = 0;
            for (int i = 0; i < kw_count; i++) {
                vdev_printf(vdev, "%-12s", results[i]->name ? results[i]->name : "");
                line_len++;
                if (line_len % 6 == 0 && i + 1 < kw_count) {
                    vdev_printf(vdev, "\n    ");
                }
            }
            vdev_printf(vdev, "\n");
        }
        vdev_printf(vdev, "\n");

        vdev_printf(vdev, "REPL Commands:\n");
        vdev_printf(vdev, "  LIST, RUN, NEW, LOAD, SAVE, MERGE, BLOAD, BSAVE, BRUN, SELFTEST, CATALOG\n");
        return err;
    }

    while (1) {
        tok = lex_peek(lex);
        if (tok.type == TOK_EOL || tok.type == TOK_EOF) {
            break;
        }
        if (tok.type == TOK_COMMA) {
            lex_next(lex);
            continue;
        }

        lex_next(lex);
        char target[256] = "";

        if (tok.type == TOK_STRING || tok.type == TOK_IDENT || tok.type == TOK_KEYWORD || tok.type == TOK_DIRECTIVE ||
            eval_is_operator(tok.type) || (tok.start != NULL && tok.length > 0)) {
            int len = (int)(tok.length < sizeof(target) - 1 ? tok.length : sizeof(target) - 1);
            if (tok.start && len > 0) {
                runtime_memcpy(target, tok.start, len);
            }
            target[len] = '\0';
        } else if (tok.type == TOK_GLOBAL_LABEL) {
            int len = (int)(tok.length < sizeof(target) - 3 ? tok.length : sizeof(target) - 3);
            target[0] = ':';
            target[1] = ':';
            if (tok.start && len > 0) {
                runtime_memcpy(target + 2, tok.start, len);
            }
            target[len + 2] = '\0';
        } else if (tok.type == TOK_NAMESPACE_DECL) {
            int len = (int)(tok.length < sizeof(target) - 1 ? tok.length : sizeof(target) - 1);
            if (tok.start && len > 0) {
                runtime_memcpy(target, tok.start, len);
            }
            target[len] = '\0';
        } else {
            break;
        }

        if (runtime_strcasecmp(target, "REGISTER") == 0) {
            BppToken topic_tok = lex_next(lex);
            if (topic_tok.type != TOK_STRING) {
                err.code = 2; err.message = "Expected string for help topic";
                return err;
            }
            BppToken comma = lex_next(lex);
            if (comma.type != TOK_COMMA) {
                err.code = 2; err.message = "Expected ',' after help topic";
                return err;
            }
            BppToken text_tok = lex_next(lex);
            if (text_tok.type != TOK_STRING) {
                err.code = 2; err.message = "Expected string for help text";
                return err;
            }

            char topic_buf[256];
            int t_len = (int)(topic_tok.length < sizeof(topic_buf) - 1 ? topic_tok.length : sizeof(topic_buf) - 1);
            if (topic_tok.start && t_len > 0) {
                runtime_memcpy(topic_buf, topic_tok.start, t_len);
            }
            topic_buf[t_len] = '\0';

            char text_buf[1024];
            int tx_len = (int)(text_tok.length < sizeof(text_buf) - 1 ? text_tok.length : sizeof(text_buf) - 1);
            if (text_tok.start && tx_len > 0) {
                runtime_memcpy(text_buf, text_tok.start, tx_len);
            }
            text_buf[tx_len] = '\0';

            if (!metadata_register_docstring(reg, topic_buf, text_buf)) {
                err.code = 14; err.message = "Failed to register help topic";
                return err;
            }
            return err;
        }

        display_help_for_target(vm, vdev, reg, target);
    }

    return err;
}

BppError stmt_catalog_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    VDevContext *vdev = vm_get_vdev(vm);
    BppMetadataRegistry *reg = vm_get_metadata(vm);

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_EOL || tok.type == TOK_EOF) {
        vdev_printf(vdev, "=== BASIC++ ENVIRONMENT CATALOG ===\n\n");

        vdev_printf(vdev, "[Built-in Keywords by Category]\n");
        char categories[64][64];
        int cat_count = lang_desc_get_categories(categories, 64);

        for (int c = 0; c < cat_count; c++) {
            vdev_printf(vdev, "  %s:\n    ", categories[c]);
            const LanguageDescriptor *results[512];
            int kw_count = lang_desc_query_category(categories[c], results, 512);
            int line_len = 0;
            for (int i = 0; i < kw_count; i++) {
                vdev_printf(vdev, "%-12s", results[i]->name ? results[i]->name : "");
                line_len++;
                if (line_len % 6 == 0 && i + 1 < kw_count) {
                    vdev_printf(vdev, "\n    ");
                }
            }
            vdev_printf(vdev, "\n");
        }
        vdev_printf(vdev, "\n");

        vdev_printf(vdev, "[Active Modules & Libraries]\n");
        int mc = module_count();
        if (mc == 0) {
            vdev_printf(vdev, "  (None registered)\n");
        } else {
            vdev_printf(vdev, "  %-16s %-10s %-8s %-6s %s\n", "Module", "Class", "Version", "Caps", "Status");
            vdev_printf(vdev, "  %-16s %-10s %-8s %-6s %s\n", "------", "-----", "-------", "----", "------");
            for (int i = 0; i < mc; ++i) {
                const BppModuleInfo *m = module_get(i);
                if (!m) continue;
                char caps[12];
                module_caps_string(m->capabilities, caps, sizeof(caps));
                vdev_printf(vdev, "  %-16s %-10s %-8s %-6s %s\n",
                            m->name,
                            module_class_name(m->mod_class),
                            m->version,
                            caps,
                            module_is_loaded(i) ? "ACTIVE" : "INACTIVE");
            }
        }
        vdev_printf(vdev, "\n");

        if (reg) {
            vdev_printf(vdev, "[User-Defined Global Labels]\n");
            if (reg->global_label_count == 0) {
                vdev_printf(vdev, "  (None registered)\n");
            } else {
                for (int i = 0; i < reg->global_label_count; i++) {
                    vdev_printf(vdev, "  ::%-20s -> %s (Line %lld)\n", 
                                reg->global_labels[i].label_name,
                                reg->global_labels[i].filename,
                                (long long)reg->global_labels[i].line_number);
                }
            }
            vdev_printf(vdev, "\n");

            vdev_printf(vdev, "[Custom Metadata Blocks]\n");
            if (reg->metadata_block_count == 0) {
                vdev_printf(vdev, "  (None registered)\n");
            } else {
                for (int i = 0; i < reg->metadata_block_count; i++) {
                    const BppMetadataBlock *b = &reg->metadata_blocks[i];
                    vdev_printf(vdev, "  ::%-8s %-20s (Docstring: %s)\n",
                                b->block_type, b->target_name,
                                b->docstring[0] ? b->docstring : "none");
                }
            }
            vdev_printf(vdev, "\n");
        }

        vdev_printf(vdev, "[Dynamic Keyword Specifications]\n");
        int sc = spec_get_count();
        if (sc == 0) {
            vdev_printf(vdev, "  (None registered)\n");
        } else {
            vdev_printf(vdev, "  %-16s %-10s %-8s %-12s %s\n", "Keyword", "Category", "Version", "Security", "Companion Lib");
            vdev_printf(vdev, "  %-16s %-10s %-8s %-12s %s\n", "-------", "--------", "-------", "--------", "-------------");
            for (int i = 0; i < sc; ++i) {
                SpecObject *s = spec_get_by_index(i);
                if (s) {
                    vdev_printf(vdev, "  %-16s %-10s %-8s %-12s %s\n",
                                s->name,
                                s->category == SPEC_CAT_STATEMENT ? "STATEMENT" : "FUNCTION",
                                s->version[0] ? s->version : "1.0",
                                s->required_level,
                                s->lib_path[0] ? s->lib_path : "None");
                }
            }
        }
        vdev_printf(vdev, "\n");
        return err;
    }

    while (1) {
        tok = lex_peek(lex);
        if (tok.type == TOK_EOL || tok.type == TOK_EOF) break;
        if (tok.type == TOK_COMMA) { lex_next(lex); continue; }

        lex_next(lex);
        char target[256] = "";
        if (tok.type == TOK_STRING || tok.type == TOK_IDENT || tok.type == TOK_KEYWORD || tok.type == TOK_DIRECTIVE ||
            eval_is_operator(tok.type) || (tok.start != NULL && tok.length > 0)) {
            int len = (int)(tok.length < sizeof(target) - 1 ? tok.length : sizeof(target) - 1);
            if (tok.start && len > 0) runtime_memcpy(target, tok.start, len);
            target[len] = '\0';
        } else {
            break;
        }

        const LanguageDescriptor *results[512];
        int match_count = lang_desc_query_category(target, results, 512);

        if (match_count > 0) {
            vdev_printf(vdev, "[Category: %s]\n  ", target);
            for (int i = 0; i < match_count; i++) {
                vdev_printf(vdev, "%-14s", results[i]->name ? results[i]->name : "");
                if ((i + 1) % 5 == 0 && i + 1 < match_count) {
                    vdev_printf(vdev, "\n  ");
                }
            }
            vdev_printf(vdev, "\n\n");
            continue;
        }

        const LanguageDescriptor *desc = lang_desc_find(target);
        if (desc && desc->category && desc->category[0] != '\0') {
            vdev_printf(vdev, "%-16s -> %s\n", target, desc->category);
        } else {
            vdev_printf(vdev, "%-16s -> (Not Found in Catalog)\n", target);
        }
    }

    return err;
}
