// FILENAME: func_program.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, sys_fn.c, eval_ident_builtin.c)
// NEEDS: libcore (language_descriptor.h, memory.h, string.h)
// NEEDS: libengine (func_program.h)
// Provides runtime implementation for the PROGRAM$ program source introspection function.

#include "eval/functions/system/environment/func_program.h"
#include "runtime/language_descriptor.h"
#include "runtime/funcreg.h"
#include "runtime/memory/alloc.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/format/snprintf.h"
#include "runtime/conv/num_parse.h"
#include "runtime/strings.h"
#include "runtime/string.h"
#include "memory/memory.h"

static const LangDesc g_program_desc = {
    .name = "PROGRAM$",
    .category = "Introspection",
    .syntax = "PROGRAM$(line_number) | PROGRAM$(\"COUNT\")",
    .description = "Returns the source text of a stored program line or program metrics string.",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_FUNCTION
};

void func_program_register(void) {
    lang_desc_register(&g_program_desc);

    FunctionEntry entry = {
        .name = "PROGRAM$",
        .keyword = KW_NONE,
        .category = FCAT_STRING,
        .ret_type = FRET_STRING,
        .min_args = 0,
        .max_args = 1,
        .safety = FSAFE_STATE,
        .overridable = 0,
        .handler = NULL,
        .help_text = "Introspect stored program: PROGRAM$(line_number) or PROGRAM$(\"COUNT\")",
        .module_name = "System"
    };
    funcreg_register(&entry);
}

BValue func_program_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    (void)err;
    BValue res;
    runtime_memset(&res, 0, sizeof(res));
    res.type = VAL_STRING;

    if (!vm) {
        return res;
    }
    MemoryContext *mem = vm_get_mem(vm);
    StringContext *sp = vm_get_str(vm);
    if (!mem || !sp) {
        return res;
    }

    size_t total_lines = 0;
    BppProgramLine *lines = mem_program_get_all(mem, &total_lines);

    if (arg_count == 0) {
        char buf[32];
        runtime_snprintf(buf, sizeof(buf), "%zu", total_lines);
        res.as.string = str_create(sp, buf, runtime_strlen(buf));
        return res;
    }

    if (args[0].type == VAL_STRING && args[0].as.string) {
        const char *s = str_data(args[0].as.string);
        if (runtime_strcasecmp(s, "COUNT") == 0 || runtime_strcasecmp(s, "LINES") == 0) {
            char buf[32];
            runtime_snprintf(buf, sizeof(buf), "%zu", total_lines);
            res.as.string = str_create(sp, buf, runtime_strlen(buf));
            return res;
        }
    }

    BppLineNumber target_line = 0;
    if (args[0].type == VAL_NUMBER) {
        target_line = (BppLineNumber)args[0].as.number;
    } else if (args[0].type == VAL_STRING && args[0].as.string) {
        target_line = (BppLineNumber)runtime_atoi(str_data(args[0].as.string));
    }

    size_t idx = 0;
    if (lines && mem_program_find_line_index(mem, target_line, &idx)) {
        const char *txt = lines[idx].text ? lines[idx].text : "";
        res.as.string = str_create(sp, txt, runtime_strlen(txt));
        return res;
    }

    res.as.string = str_create(sp, "", 0);
    return res;
}
