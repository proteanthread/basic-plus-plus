/**
 * @file stmt_help.c
 * @brief HELP and CATALOG statement handler implementations.
 *
 * SECTION 1: WHAT IT DOES
 * - Implements the HELP and CATALOG statements. Shows command lists, module tables,
 *   global labels, and metadata blocks.
 *
 * SECTION 2: WHY IT EXISTS
 * - Provides dynamic REPL-level and runtime help/docstring and catalog introspection
 *   without depending on external text files at runtime.
 *
 * SECTION 3: WHY IT WORKS THIS WAY
 * - It references a compiled-in static database generated from source files (help_data.h)
 *   and queries BppMetadataRegistry, dynamic modules, and function systems at runtime.
 *
 * SECTION 4: WHAT CAN BE CHANGED
 * - Help layout, column alignment, text formatting, and categorization.
 *
 * SECTION 5: WHAT CANNOT BE CHANGED
 * - Introspection queries, registry traversal routines, or console routing.
 *
 * SECTION 6: WHAT TO EXPECT
 * - O(1) keyword help lookup and fast O(N) catalog generation.
 *
 * SECTION 7: WHAT TO DO IF SOMETHING BREAKS
 * - Verify the help_data.h was regenerated successfully from gen_help.bas.
 *
 * SECTION 8: ASSUMPTIONS
 * - The registry contexts are initialized and populated during boot phases.
 *
 * SECTION 9: PORTABILITY CONCERNS
 * - Core operations are platform-independent; uses vdev instead of stdout.
 *
 * SECTION 10: FUTURE EXPANSIONS
 * - Adding sub-topic help search, fuzzy keyword matching, or interactive pagination.
 *
 * SECTION 11: EXTERNAL EXTENSION HOOKS
 * - Dynamic modules can register help details via bpp_module_init entry points.
 */

#include "bpp_vm.h"
#include "bpp_lexer.h"
#include "bpp_vdev.h"
#include "bpp_metadata.h"
#include "bpp_module.h"
#include "bpp_funcreg.h"
#include "bpp_spec.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#ifdef _MSC_VER
#define strncasecmp _strnicmp
#endif

typedef struct {
    const char *name;
    const char *category;
    const char *syntax;
    const char *desc;
} BuiltinHelpEntry;

#include "help_data.h"

static void resolve_relative_path(VMContext *vm, const char *rel_path, char *out_path, size_t max_len) {
    const char *curr_file = vm_get_current_filename(vm);
    if (curr_file && curr_file[0] != '\0') {
        /* Find last slash or backslash */
        const char *last_slash = strrchr(curr_file, '/');
        const char *last_backslash = strrchr(curr_file, '\\');
        const char *slash = (last_slash > last_backslash) ? last_slash : last_backslash;
        if (slash) {
            size_t dir_len = slash - curr_file + 1;
            if (dir_len < max_len - 1) {
                memcpy(out_path, curr_file, dir_len);
                out_path[dir_len] = '\0';
                strncat(out_path, rel_path, max_len - dir_len - 1);
                return;
            }
        }
    }
    /* Fallback to current working directory */
    strncpy(out_path, rel_path, max_len - 1);
    out_path[max_len - 1] = '\0';
}

static bool extract_help_file(const char *body, char *out_file, size_t max_len) {
    if (!body || body[0] == '\0') return false;
    const char *p = body;
    while (*p) {
        /* Skip leading whitespace of the line */
        while (*p && (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')) {
            p++;
        }
        if (*p == '\0') break;

        /* Check for comment marker or HELPFILE / DOCFILE */
        bool found = false;
        int word_len = 0;
        if (strncasecmp(p, "HELPFILE", 8) == 0) {
            word_len = 8;
            found = true;
        } else if (strncasecmp(p, "DOCFILE", 7) == 0) {
            word_len = 7;
            found = true;
        }

        if (found) {
            p += word_len;
            /* Skip spaces */
            while (*p && (*p == ' ' || *p == '\t')) p++;
            /* Check for optional quote */
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

        /* Go to next line */
        while (*p && *p != '\n') p++;
    }
    return false;
}

BppError stmt_help_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    VDevContext *vdev = vm_get_vdev(vm);
    BppMetadataRegistry *reg = vm_get_metadata(vm);

    BppToken tok = lex_peek(lex);

    if (tok.type == TOK_EOL || tok.type == TOK_EOF) {
        /* General Help */
        vdev_printf(vdev, "BASIC++ Interactive Help System\n");
        vdev_printf(vdev, "Usage:\n");
        vdev_printf(vdev, "  HELP            - Show this general help text.\n");
        vdev_printf(vdev, "  HELP <target>   - Query docstring/block/function documentation.\n");
        vdev_printf(vdev, "  CATALOG         - List active modules, global labels, and custom blocks.\n\n");

        /* Show categorised built-in keywords */
        vdev_printf(vdev, "[Built-in Keywords by Category]\n");
        const char *prev_cat = "";
        int line_len = 0;
        for (size_t i = 0; i < sizeof(g_builtin_help) / sizeof(g_builtin_help[0]); i++) {
            if (strcmp(g_builtin_help[i].category, prev_cat) != 0) {
                if (line_len > 0) {
                    vdev_printf(vdev, "\n");
                    line_len = 0;
                }
                vdev_printf(vdev, "  %s:\n    ", g_builtin_help[i].category);
                prev_cat = g_builtin_help[i].category;
            }
            vdev_printf(vdev, "%-12s", g_builtin_help[i].name);
            line_len++;
            if (line_len % 6 == 0) {
                vdev_printf(vdev, "\n    ");
            }
        }
        if (line_len > 0) {
            vdev_printf(vdev, "\n");
        }
        vdev_printf(vdev, "\n");

        vdev_printf(vdev, "REPL Commands:\n");
        vdev_printf(vdev, "  LIST, RUN, NEW, LOAD, SAVE, MERGE, BLOAD, BSAVE, BRUN, SELFTEST, CATALOG\n");
        return err;
    }

    lex_next(lex); /* Consume the help target token */
    char target[256] = "";

    if (tok.type == TOK_STRING) {
        int len = (int)(tok.length < sizeof(target) - 1 ? tok.length : sizeof(target) - 1);
        memcpy(target, tok.as.string, len);
        target[len] = '\0';
    } else if (tok.type == TOK_IDENT || tok.type == TOK_KEYWORD) {
        int len = (int)(tok.length < sizeof(target) - 1 ? tok.length : sizeof(target) - 1);
        memcpy(target, tok.start, len);
        target[len] = '\0';
    } else if (tok.type == TOK_GLOBAL_LABEL) {
        snprintf(target, sizeof(target), "::%s", tok.as.string);
    } else if (tok.type == TOK_NAMESPACE_DECL) {
        strncpy(target, tok.as.string, sizeof(target) - 1);
        target[sizeof(target) - 1] = '\0';
    } else {
        err.code = 2; err.message = "Expected help target identifier or string";
        return err;
    }

    if (strcasecmp(target, "REGISTER") == 0) {
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
        memcpy(topic_buf, topic_tok.as.string, t_len);
        topic_buf[t_len] = '\0';

        char text_buf[1024];
        int tx_len = (int)(text_tok.length < sizeof(text_buf) - 1 ? text_tok.length : sizeof(text_buf) - 1);
        memcpy(text_buf, text_tok.as.string, tx_len);
        text_buf[tx_len] = '\0';

        if (!metadata_register_docstring(reg, topic_buf, text_buf)) {
            err.code = 14; err.message = "Failed to register help topic";
            return err;
        }
        return err;
    }

    /* 1. Try standard docstring */
    const char *doc = metadata_get_docstring(reg, target);
    if (doc && doc[0] != '\0') {
        vdev_printf(vdev, "Documentation for '%s':\n%s\n", target, doc);
        return err;
    }

    /* 2. Try metadata blocks (KEYWORD, SCOPE, ALIAS, OPTION) */
    const BppMetadataBlock *block = metadata_get_block(reg, "KEYWORD", target);
    if (!block) block = metadata_get_block(reg, "SCOPE", target);
    if (!block) block = metadata_get_block(reg, "ALIAS", target);
    if (!block) block = metadata_get_block(reg, "OPTION", target);

    if (block) {
        /* Check if the block body specifies an external help/doc file */
        char rel_file[256];
        if (extract_help_file(block->body, rel_file, sizeof(rel_file))) {
            char file_path[512];
            resolve_relative_path(vm, rel_file, file_path, sizeof(file_path));
            FILE *fp = fopen(file_path, "r");
            if (fp) {
                vdev_printf(vdev, "[%s BLOCK: %s (Loaded from %s)]\n", block->block_type, block->target_name, rel_file);
                char read_buf[512];
                while (fgets(read_buf, sizeof(read_buf), fp)) {
                    vdev_printf(vdev, "%s", read_buf);
                }
                vdev_printf(vdev, "\n");
                fclose(fp);
                return err;
            }
        }

        vdev_printf(vdev, "[%s BLOCK: %s]\nDocstring: %s\nBody:\n%s\n", 
                    block->block_type, block->target_name, 
                    block->docstring[0] ? block->docstring : "(None)", 
                    block->body[0] ? block->body : "(Empty)");
        return err;
    }

    /* 3. Try function registry */
    const FunctionEntry *entry = funcreg_find_by_name(target);
    if (entry) {
        vdev_printf(vdev, "Function: %s\nHelp:     %s\n", entry->name, entry->help_text);
        return err;
    }

    /* Try built-in static keywords */
    for (size_t i = 0; i < sizeof(g_builtin_help) / sizeof(g_builtin_help[0]); i++) {
        if (strcasecmp(target, g_builtin_help[i].name) == 0) {
            vdev_printf(vdev, "Category:  %s\n", g_builtin_help[i].category);
            vdev_printf(vdev, "Syntax:    %s\n", g_builtin_help[i].syntax);
            vdev_printf(vdev, "Help:      %s\n", g_builtin_help[i].desc);
            return err;
        }
    }



    /* 4. Try dynamic keyword specification */
    SpecObject *spec = spec_find_by_name(target);
    if (spec) {
        vdev_printf(vdev, "Dynamic Feature: %s\n", spec->name);
        vdev_printf(vdev, "Category:        %s\n", spec->category == SPEC_CAT_STATEMENT ? "STATEMENT" : "FUNCTION");
        vdev_printf(vdev, "Version:         %s\n", spec->version[0] ? spec->version : "1.0");
        vdev_printf(vdev, "Security Level:  %s\n", spec->required_level);
        vdev_printf(vdev, "Companion Lib:   %s\n", spec->lib_path[0] ? spec->lib_path : "None");
        return err;
    }

    vdev_printf(vdev, "No documentation found for target '%s'\n", target);
    return err;
}

BppError stmt_catalog_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    VDevContext *vdev = vm_get_vdev(vm);
    BppMetadataRegistry *reg = vm_get_metadata(vm);

    (void)lex; /* unused */

    vdev_printf(vdev, "=== BASIC++ ENVIRONMENT CATALOG ===\n\n");

    /* Show categorised built-in keywords */
    vdev_printf(vdev, "[Built-in Keywords by Category]\n");
    const char *prev_cat = "";
    int line_len = 0;
    for (size_t i = 0; i < sizeof(g_builtin_help) / sizeof(g_builtin_help[0]); i++) {
        if (strcmp(g_builtin_help[i].category, prev_cat) != 0) {
            if (line_len > 0) {
                vdev_printf(vdev, "\n");
                line_len = 0;
            }
            vdev_printf(vdev, "  %s:\n    ", g_builtin_help[i].category);
            prev_cat = g_builtin_help[i].category;
        }
        vdev_printf(vdev, "%-12s", g_builtin_help[i].name);
        line_len++;
        if (line_len % 6 == 0) {
            vdev_printf(vdev, "\n    ");
        }
    }
    if (line_len > 0) {
        vdev_printf(vdev, "\n");
    }
    vdev_printf(vdev, "\n");

    /* 1. Show Dynamic Modules & Extensions */
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
        /* 2. Show User-Defined Global Labels */
        vdev_printf(vdev, "[User-Defined Global Labels]\n");
        if (reg->global_label_count == 0) {
            vdev_printf(vdev, "  (None registered)\n");
        } else {
            for (int i = 0; i < reg->global_label_count; i++) {
                vdev_printf(vdev, "  ::%-20s -> %s (Line %g)\n", 
                            reg->global_labels[i].label_name,
                            reg->global_labels[i].filename,
                            reg->global_labels[i].line_number);
            }
        }
        vdev_printf(vdev, "\n");

        /* 3. Show Registered Metadata Blocks (KEYWORD, SCOPE, ALIAS, OPTION) */
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

    /* 4. Show Registered Dynamic Specifications */
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

    vdev_printf(vdev, "Use HELP <keyword> or HELP <block_target> for more detailed information.\n");

    return err;
}
