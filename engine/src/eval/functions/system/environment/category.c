// FILENAME: category.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (conversion_fn.c)
// NEEDS: libcore (funcreg.h, language_descriptor.h, spec.h, string.h, strings.h)
// Provides runtime implementation for the CATEGORY$ built-in function in BASIC++.

#include "eval/functions/system/environment/category.h"
#include "runtime/language_descriptor.h"
#include "runtime/strings.h"
#include "runtime/funcreg.h"
#include "runtime/spec.h"
#include "runtime/string.h"
#include "runtime/memory.h"
#include "runtime/string/strops.h"

static const LangDesc g_category_desc = {
    .name = "CATEGORY$",
    .category = "Introspection",
    .syntax = "CATEGORY$(keyword$)",
    .description = "Returns the category name of the specified keyword, or empty string if not found.",
    .error_summary = "Error 13: Type Mismatch (expects one string argument)",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};

void func_category_register(void) {
    lang_desc_register(&g_category_desc);
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

    // 1. Query LanguageDescriptor Registry
    const LanguageDescriptor *desc = lang_desc_find(target);
    if (desc && desc->category && desc->category[0] != '\0') {
        cat = desc->category;
    }

    // 2. Try Dynamic Keyword Spec
    if (!cat) {
        SpecObject *spec = spec_find_by_name(target);
        if (spec) {
            cat = (spec->category == SPEC_CAT_STATEMENT) ? "Dynamic Statements" : "Dynamic Functions";
        }
    }

    // 3. Try Function Registry
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
