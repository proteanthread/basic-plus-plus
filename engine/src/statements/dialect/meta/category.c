// FILENAME: category.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (conversion_fn.c)
// NEEDS: libcore (funcreg.h, funcreg.c, memops.h, memops.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, spec.h, spec.c)
// NEEDS: libcore (strops.h, strops.c)
// NEEDS: libengine (category.h, eval_internal.h, help_data.h)
// NEEDS: libkernel (vdev.h, vdev.c)
// Provides runtime implementation for the CATEGORY statement in BASIC++.
//
// ---- Includes ----

#include "statements/dialect/meta/category.h"
#include "device/vdev.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/funcreg.h"
#include "runtime/spec.h"
#include "eval/eval_internal.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"

typedef struct {
    const char *name;
    const char *category;
    const char *syntax;
    const char *desc;
} BuiltinHelpEntry;

#include "statements/dialect/meta/help_data.h"

static bool category_list_contains(char cats[256][64], int count, const char *cat) {
    for (int i = 0; i < count; i++) {
        if (runtime_strcasecmp(cats[i], cat) == 0) return true;
    }
    return false;
}

static const char *lookup_keyword_category(const char *target) {
    // 1. Try MicroLib Metadata
    const MicroLibMetadata *meta = microlib_find(target);
    if (meta && meta->category && meta->category[0] != '\0') {
        return meta->category;
    }

    // 2. Try Built-in Static Help
    for (size_t i = 0; i < sizeof(g_builtin_help) / sizeof(g_builtin_help[0]); i++) {
        if (runtime_strcasecmp(target, g_builtin_help[i].name) == 0) {
            return g_builtin_help[i].category;
        }
    }

    // 3. Try Dynamic Keyword Spec
    SpecObject *spec = spec_find_by_name(target);
    if (spec) {
        return (spec->category == SPEC_CAT_STATEMENT) ? "Dynamic Statements" : "Dynamic Functions";
    }

    // 4. Try Function Registry
    const FunctionEntry *entry = funcreg_find_by_name(target);
    if (entry) {
        return "Registered Functions";
    }

    return NULL;
}

BppError stmt_category_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    VDevContext *vdev = vm_get_vdev(vm);

    BppToken tok = lex_peek(lex);

    // Case 1: No arguments -> List all distinct categories
    if (tok.type == TOK_EOF || tok.type == TOK_EOL) {

        char categories[256][64];
        int cat_count = 0;

        for (size_t i = 0; i < sizeof(g_builtin_help) / sizeof(g_builtin_help[0]); i++) {
            const char *c = g_builtin_help[i].category;
            if (c && c[0] != '\0' && !category_list_contains(categories, cat_count, c) && cat_count < 256) {
                runtime_strncpy(categories[cat_count], c, sizeof(categories[0]) - 1);
                categories[cat_count][sizeof(categories[0]) - 1] = '\0';
                cat_count++;
            }
        }

        // Sort categories alphabetically
        for (int i = 0; i < cat_count - 1; i++) {
            for (int j = i + 1; j < cat_count; j++) {
                if (runtime_strcasecmp(categories[i], categories[j]) > 0) {
                    char tmp[64];
                    runtime_strcpy(tmp, categories[i]);
                    runtime_strcpy(categories[i], categories[j]);
                    runtime_strcpy(categories[j], tmp);
                }
            }
        }

        vdev_printf(vdev, "=== AVAILABLE KEYWORD CATEGORIES ===\n\n");
        for (int i = 0; i < cat_count; i++) {
            vdev_printf(vdev, "  - %s\n", categories[i]);
        }
        vdev_printf(vdev, "\nUse CATEGORY <keyword> to see a keyword's category, or CATEGORY <category_name> to list keywords.\n");
        return err;
    }

    // Consume argument token
    tok = lex_next(lex);
    char target[128] = "";
    if (tok.type == TOK_STRING || tok.type == TOK_IDENT || tok.type == TOK_KEYWORD) {
        size_t len = (tok.length < sizeof(target) - 1) ? tok.length : sizeof(target) - 1;
        if (tok.start && len > 0) {
            runtime_memcpy(target, tok.start, len);
        }
        target[len] = '\0';
    } else {
        err.code = 2;
        err.message = "Expected keyword identifier or category string";
        return err;
    }


    // Check if target matches a specific keyword
    const char *cat = lookup_keyword_category(target);
    if (cat) {
        vdev_printf(vdev, "Keyword:  %s\nCategory: %s\n", target, cat);
        return err;
    }

    // Check if target matches a category name (or substring)
    char matching_cat[64] = "";
    for (size_t i = 0; i < sizeof(g_builtin_help) / sizeof(g_builtin_help[0]); i++) {
        if (runtime_strcasecmp(target, g_builtin_help[i].category) == 0) {
            runtime_strcpy(matching_cat, g_builtin_help[i].category);
            break;
        }
    }

    if (matching_cat[0] == '\0') {
        // Partial / substring match
        for (size_t i = 0; i < sizeof(g_builtin_help) / sizeof(g_builtin_help[0]); i++) {
            if (runtime_strstr(g_builtin_help[i].category, target) != NULL) {
                runtime_strcpy(matching_cat, g_builtin_help[i].category);
                break;
            }
        }
    }

    if (matching_cat[0] != '\0') {
        vdev_printf(vdev, "Category: %s\nKeywords:\n  ", matching_cat);
        int col = 0;
        for (size_t i = 0; i < sizeof(g_builtin_help) / sizeof(g_builtin_help[0]); i++) {
            if (runtime_strcasecmp(matching_cat, g_builtin_help[i].category) == 0) {
                vdev_printf(vdev, "%-14s", g_builtin_help[i].name);
                col++;
                if (col % 5 == 0) {
                    vdev_printf(vdev, "\n  ");
                }
            }
        }
        if (col % 5 != 0) {
            vdev_printf(vdev, "\n");
        }
        return err;
    }

    vdev_printf(vdev, "Unknown keyword or category '%s'\n", target);
    return err;
}

void stmt_category_register(void) {
    static const MicroLibMetadata meta = {
        .name = "CATEGORY",
        .category = "Introspection",
        .syntax = "CATEGORY [keyword | category_name]",
        .help_text = "Displays all categories, queries the category of a keyword, or lists all keywords in a category.",
        .error_codes = "Error 2: Syntax Error"
    };
    microlib_register(&meta);
}
