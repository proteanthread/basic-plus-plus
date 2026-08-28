// FILENAME: category.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (conversion_fn.c)
// NEEDS: libcore (funcreg.h, funcreg.c, memory.h, memory.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, spec.h, spec.c)
// NEEDS: libcore (string.h, strings.h, strings.c)
// NEEDS: libengine (category.h, help_data.h, string.c)
// Provides runtime implementation for the CATEGORY built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/system/environment/category.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/strings.h"
#include "runtime/funcreg.h"
#include "runtime/spec.h"
typedef struct {
    const char *name;
    const char *category;
    const char *syntax;
    const char *desc;
} BuiltinHelpEntry;

#include "statements/dialect/meta/help_data.h"


#include "runtime/string.h"
#include "runtime/memory.h"
void func_category_register(void) {
    static const MicroLibMetadata meta = {
        .name = "CATEGORY$",
        .category = "Introspection",
        .syntax = "CATEGORY$(keyword$)",
        .help_text = "Returns the category name of the specified keyword, or empty string if not found.",
        .error_codes = "Error 13: Type Mismatch (expects one string argument)"
    };
    microlib_register(&meta);
}

BValue func_category_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (arg_count != 1 || args[0].type != VAL_STRING) {
        err->code = 13;
        err->message = "CATEGORY$ expects one string argument";
        return res;
    }

    BppStringRef sr = args[0].as.string;
    const char *target = str_data(sr);
    const char *cat = NULL;

    // 1. Try MicroLib Metadata
    const MicroLibMetadata *meta = microlib_find(target);
    if (meta && meta->category && meta->category[0] != '\0') {
        cat = meta->category;
    }

    // 2. Try Built-in Static Help
    if (!cat) {
        for (size_t i = 0; i < sizeof(g_builtin_help) / sizeof(g_builtin_help[0]); i++) {
            if (runtime_strcasecmp(target, g_builtin_help[i].name) == 0) {
                cat = g_builtin_help[i].category;
                break;
            }
        }
    }

    // 3. Try Dynamic Keyword Spec
    if (!cat) {
        SpecObject *spec = spec_find_by_name(target);
        if (spec) {
            cat = (spec->category == SPEC_CAT_STATEMENT) ? "Dynamic Statements" : "Dynamic Functions";
        }
    }

    // 4. Try Function Registry
    if (!cat) {
        const FunctionEntry *entry = funcreg_find_by_name(target);
        if (entry) {
            cat = "Registered Functions";
        }
    }

    res.type = VAL_STRING;
    if (cat) {
        res.as.string = str_create(vm_get_str(vm), cat, runtime_strlen(cat));
    } else {
        res.as.string = str_create(vm_get_str(vm), "", 0);
    }

    str_release(vm_get_str(vm), sr);
    return res;
}
