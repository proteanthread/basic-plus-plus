// FILENAME: category.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore, libengine, libflex
// NEEDS: libcore (funcreg.h, language_descriptor.h, memops.h, spec.h, strops.h)
// NEEDS: libkernel (vdev.h)
// Provides runtime implementation for the CATEGORY statement in BASIC++.

#include "statements/introspection/category.h"
#include "device/vdev.h"
#include "runtime/language_descriptor.h"
#include "runtime/funcreg.h"
#include "runtime/spec.h"
#include "eval/eval_internal.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"

static const LangDesc g_category_stmt_desc = {
    .name = "CATEGORY",
    .category = "Introspection",
    .syntax = "CATEGORY [keyword | category_name]",
    .description = "Displays all categories, queries the category of a keyword, or lists all keywords in a category.",
    .error_summary = "Error 2: Syntax Error",
    .subsystem = SUBSYSTEM_CORE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_STATEMENT
};

void stmt_category_register(void) {
    lang_desc_register(&g_category_stmt_desc);
}

static const char *lookup_keyword_category(const char *target) {
    if (!target || target[0] == '\0') return NULL;

    const LanguageDescriptor *desc = lang_desc_find(target);
    if (desc && desc->category && desc->category[0] != '\0') {
        return desc->category;
    }

    SpecObject *spec = spec_find_by_name(target);
    if (spec) {
        return (spec->category == SPEC_CAT_STATEMENT) ? "Dynamic Statements" : "Dynamic Functions";
    }

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

    if (tok.type == TOK_EOF || tok.type == TOK_EOL) {
        char categories[256][64];
        int cat_count = lang_desc_get_categories(categories, 256);

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

    const char *cat = lookup_keyword_category(target);
    if (cat) {
        vdev_printf(vdev, "Keyword:  %s\nCategory: %s\n", target, cat);
        return err;
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
        return err;
    }

    vdev_printf(vdev, "Unknown keyword or category '%s'\n", target);
    return err;
}
