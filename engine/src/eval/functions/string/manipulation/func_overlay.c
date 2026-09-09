// FILENAME: func_overlay.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libengine (func_overlay.h, vm.h)
// Provides runtime implementation for the OVERLAY$ string overwriting function in BASIC++.

#include "eval/functions/string/manipulation/func_overlay.h"
#include "runtime/language_descriptor.h"
#include "runtime/strings.h"
#include "runtime/string.h"
#include "runtime/memory.h"

static const LangDesc g_func_overlay_desc = {
    .name = "OVERLAY$",
    .category = "String Functions",
    .syntax = "OVERLAY$(target$, source$, pos)",
    .description = "Overwrites characters in target$ with source$ starting at 1-based index pos (supports negative pos).",
    .error_summary = "Error 13: Type Mismatch, Error 7: Out of Memory",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_FUNCTION
};

void func_overlay_register(void) {
    lang_desc_register(&g_func_overlay_desc);
}

BValue func_overlay_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (arg_count != 3 || args[0].type != VAL_STRING || args[1].type != VAL_STRING || args[2].type == VAL_STRING) {
        err->code = 13;
        err->message = "OVERLAY$ expects (target$, source$, pos)";
        if (args[0].type == VAL_STRING && args[0].as.string) str_release(vm_get_str(vm), args[0].as.string);
        if (arg_count > 1 && args[1].type == VAL_STRING && args[1].as.string) str_release(vm_get_str(vm), args[1].as.string);
        return res;
    }

    BppStringRef sr_target = args[0].as.string;
    BppStringRef sr_source = args[1].as.string;
    const char *target = str_data(sr_target);
    size_t target_len = str_len(sr_target);
    const char *source = str_data(sr_source);
    size_t source_len = str_len(sr_source);
    int raw_pos = (int)args[2].as.number;

    if (source_len == 0) {
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), target, target_len);
        str_release(vm_get_str(vm), sr_target);
        str_release(vm_get_str(vm), sr_source);
        return res;
    }

    int ovl_pos = raw_pos;
    if (ovl_pos < 0) {
        ovl_pos = (int)target_len + ovl_pos + 1;
    }
    if (ovl_pos < 1) {
        ovl_pos = 1;
    }

    size_t ovl_idx = (size_t)(ovl_pos - 1);
    size_t max_len = (ovl_idx + source_len > target_len) ? (ovl_idx + source_len) : target_len;
    char *buf = (char *)runtime_malloc(max_len + 1);
    if (!buf) {
        err->code = 7;
        err->message = "Out of memory in OVERLAY$";
        str_release(vm_get_str(vm), sr_target);
        str_release(vm_get_str(vm), sr_source);
        return res;
    }

    if (target_len > 0) {
        runtime_memcpy(buf, target, target_len);
    }
    if (ovl_idx > target_len) {
        runtime_memset(buf + target_len, ' ', ovl_idx - target_len);
    }
    runtime_memcpy(buf + ovl_idx, source, source_len);
    buf[max_len] = '\0';

    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), buf, max_len);
    runtime_free(buf);
    str_release(vm_get_str(vm), sr_target);
    str_release(vm_get_str(vm), sr_source);
    return res;
}
