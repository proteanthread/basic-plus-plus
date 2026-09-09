// FILENAME: func_systems_types.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (dispatch_call.c)
// NEEDS: libcore (language_descriptor.h), libengine (func_systems_types.h)
// Provides runtime function implementation for systems types, pointers, and masking.
//
// ---- Includes ----

#include "eval/functions/system/func_systems_types.h"
#include "eval/functions/system/hardware/peek.h"
#include "runtime/language_descriptor.h"

static const LangDesc g_cbyte_desc = {
    .name = "CBYTE",
    .category = "Systems Types",
    .syntax = "b = CBYTE(x) | BYTE(x)",
    .description = "Converts and clamps value to an 8-bit unsigned byte (0-255).",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_FUNCTION
};

static const LangDesc g_cword_desc = {
    .name = "CWORD",
    .category = "Systems Types",
    .syntax = "w = CWORD(x) | WORD(x)",
    .description = "Converts and clamps value to a 16-bit unsigned word (0-65535).",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_FUNCTION
};

static const LangDesc g_cdword_desc = {
    .name = "CDWORD",
    .category = "Systems Types",
    .syntax = "dw = CDWORD(x) | DWORD(x)",
    .description = "Converts and clamps value to a 32-bit unsigned dword (0-4294967295).",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_FUNCTION
};

static const LangDesc g_ptr_desc = {
    .name = "PTR",
    .category = "Systems Types",
    .syntax = "p = PTR(addr) | PTR[addr]",
    .description = "Wraps or dereferences a memory pointer address.",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_FUNCTION
};

static const LangDesc g_deref_desc = {
    .name = "DEREF",
    .category = "Systems Types",
    .syntax = "val = DEREF(ptr)",
    .description = "Dereferences memory address pointed to by ptr.",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_FUNCTION
};

static const LangDesc g_mask_desc = {
    .name = "MASK",
    .category = "Systems Types",
    .syntax = "res = MASK(val, mask) | MASK[mask](val)",
    .description = "Performs bitwise mask operation on value.",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_FUNCTION
};

static const LangDesc g_set_bitfield_desc = {
    .name = "SET_BITFIELD",
    .category = "Systems Types",
    .syntax = "res = SET_BITFIELD(val, start_bit, bit_count, new_val)",
    .description = "Sets bitfield of length bit_count at start_bit to new_val.",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_FUNCTION
};

void func_systems_types_register(void) {
    lang_desc_register(&g_cbyte_desc);
    lang_desc_register(&g_cword_desc);
    lang_desc_register(&g_cdword_desc);
    lang_desc_register(&g_ptr_desc);
    lang_desc_register(&g_deref_desc);
    lang_desc_register(&g_mask_desc);
    lang_desc_register(&g_set_bitfield_desc);
}

BValue func_byte_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    (void)uname;
    (void)err;
    BValue res = { .type = VAL_NUMBER, .as.number = 0.0 };
    if (arg_count < 1 || !args) return res;

    double v = (args[0].type == VAL_NUMBER || args[0].type == VAL_INTEGER) ? args[0].as.number : 0.0;
    uint8_t b = (uint8_t)((uint32_t)v);
    res.as.number = (double)b;
    return res;
}

BValue func_word_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    (void)uname;
    (void)err;
    BValue res = { .type = VAL_NUMBER, .as.number = 0.0 };
    if (arg_count < 1 || !args) return res;

    double v = (args[0].type == VAL_NUMBER || args[0].type == VAL_INTEGER) ? args[0].as.number : 0.0;
    uint16_t w = (uint16_t)((uint32_t)v);
    res.as.number = (double)w;
    return res;
}

BValue func_dword_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    (void)uname;
    (void)err;
    BValue res = { .type = VAL_NUMBER, .as.number = 0.0 };
    if (arg_count < 1 || !args) return res;

    double v = (args[0].type == VAL_NUMBER || args[0].type == VAL_INTEGER) ? args[0].as.number : 0.0;
    uint32_t dw = (uint32_t)v;
    res.as.number = (double)dw;
    return res;
}

BValue func_ptr_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    (void)uname;
    (void)err;
    BValue res = { .type = VAL_NUMBER, .as.number = 0.0 };
    if (arg_count < 1 || !args) return res;

    double v = (args[0].type == VAL_NUMBER || args[0].type == VAL_INTEGER) ? args[0].as.number : 0.0;
    uintptr_t p = (uintptr_t)v;
    res.as.number = (double)p;
    return res;
}

BValue func_deref_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    if (arg_count < 1 || !args) {
        BValue res = { .type = VAL_NUMBER, .as.number = 0.0 };
        return res;
    }
    return func_peek_eval(vm, "PEEK", arg_count, args, err);
}

BValue func_mask_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    (void)uname;
    (void)err;
    BValue res = { .type = VAL_NUMBER, .as.number = 0.0 };
    if (arg_count < 2 || !args) return res;

    uint64_t val = (uint64_t)((args[0].type == VAL_NUMBER || args[0].type == VAL_INTEGER) ? args[0].as.number : 0.0);
    uint64_t mask = (uint64_t)((args[1].type == VAL_NUMBER || args[1].type == VAL_INTEGER) ? args[1].as.number : 0.0);
    res.as.number = (double)(val & mask);
    return res;
}

BValue func_set_bitfield_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    (void)uname;
    (void)err;
    BValue res = { .type = VAL_NUMBER, .as.number = 0.0 };
    if (arg_count < 4 || !args) return res;

    uint64_t val = (uint64_t)((args[0].type == VAL_NUMBER || args[0].type == VAL_INTEGER) ? args[0].as.number : 0.0);
    int start = (int)((args[1].type == VAL_NUMBER || args[1].type == VAL_INTEGER) ? args[1].as.number : 0.0);
    int count = (int)((args[2].type == VAL_NUMBER || args[2].type == VAL_INTEGER) ? args[2].as.number : 0.0);
    uint64_t new_val = (uint64_t)((args[3].type == VAL_NUMBER || args[3].type == VAL_INTEGER) ? args[3].as.number : 0.0);

    if (count <= 0 || start < 0 || start >= 64) {
        res.as.number = (double)val;
        return res;
    }
    if (count > 64 - start) count = 64 - start;

    uint64_t mask = (count >= 64) ? ~0ULL : ((1ULL << count) - 1ULL);
    uint64_t clear_mask = ~(mask << start);
    uint64_t inserted = (new_val & mask) << start;
    uint64_t result = (val & clear_mask) | inserted;

    res.as.number = (double)result;
    return res;
}
