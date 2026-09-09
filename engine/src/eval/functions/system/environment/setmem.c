// FILENAME: setmem.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, sys_fn.c)
// NEEDS: libcore (language_descriptor.h, memory.h)
// NEEDS: libengine (setmem.h)
// Provides runtime implementation for the SETMEM function in BASIC++.

#include "eval/functions/system/environment/setmem.h"
#include "runtime/language_descriptor.h"
#include "runtime/memory.h"
#include "memory/memory.h"

static const LangDesc g_setmem_desc = {
    .name = "SETMEM",
    .category = "System Functions",
    .syntax = "SETMEM(bytes&)",
    .description = "Adjusts the memory pool reservation and returns available RAM in bytes.",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};

void func_setmem_register(void) {
    lang_desc_register(&g_setmem_desc);
}

BValue func_setmem_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname; (void)arg_count; (void)args; (void)err;
    BValue res;
    res.type = VAL_NUMBER;
    res.as.number = (double)mem_get_free_ram(vm_get_mem(vm));
    return res;
}
