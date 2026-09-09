// FILENAME: func_vintage_vars.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, eval_ident_builtin.c)
// NEEDS: libcore (language_descriptor.h, strings.h)
// NEEDS: libengine (func_vintage_vars.h)
// Provides runtime implementation and LanguageDescriptors for vintage dialect system variables.

#include "eval/functions/system/hardware/func_vintage_vars.h"
#include "runtime/language_descriptor.h"
#include "runtime/strings.h"
#include "runtime/string.h"
#include "runtime/format/snprintf.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"
#include "platform/platform.h"

static const LangDesc g_st_desc = {
    .name = "ST", .category = "Hardware & Peripherals", .syntax = "ST",
    .description = "Returns the Commodore 8-bit I/O status byte.",
    .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_PURE, .type = FEATURE_VARIABLE
};

static const LangDesc g_consol_desc = {
    .name = "CONSOL", .category = "Hardware & Peripherals", .syntax = "CONSOL",
    .description = "Returns the Atari 8-bit console key mask (Option=4, Select=2, Start=1).",
    .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_PURE, .type = FEATURE_VARIABLE
};

static const LangDesc g_user_desc = {
    .name = "USER", .category = "Hardware & Peripherals", .syntax = "USER",
    .description = "Returns the active CP/M user area number (0..15).",
    .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_PURE, .type = FEATURE_VARIABLE
};

static const LangDesc g_bdos_desc = {
    .name = "BDOS", .category = "Hardware & Peripherals", .syntax = "BDOS",
    .description = "Returns the CP/M BDOS system call vector address (5).",
    .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_PURE, .type = FEATURE_VARIABLE
};

static const LangDesc g_swap_desc = {
    .name = "SWAP$", .category = "Hardware & Peripherals", .syntax = "SWAP$",
    .description = "Returns the DEC RSTS/E swap status string.",
    .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_PURE, .type = FEATURE_VARIABLE
};

static const LangDesc g_job_desc = {
    .name = "JOB", .category = "Hardware & Peripherals", .syntax = "JOB or JOB$",
    .description = "Returns the DEC RSTS/E job number or job string.",
    .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_PURE, .type = FEATURE_VARIABLE
};

static const LangDesc g_himem_desc = {
    .name = "HIMEM", .category = "Hardware & Peripherals", .syntax = "HIMEM or MAXRAM",
    .description = "Returns the top of available vintage RAM address.",
    .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_PURE, .type = FEATURE_VARIABLE
};

static const LangDesc g_lomem_desc = {
    .name = "LOMEM", .category = "Hardware & Peripherals", .syntax = "LOMEM",
    .description = "Returns the bottom of available user RAM address.",
    .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_PURE, .type = FEATURE_VARIABLE
};

void func_vintage_vars_register(void) {
    lang_desc_register(&g_st_desc);
    lang_desc_register(&g_consol_desc);
    lang_desc_register(&g_user_desc);
    lang_desc_register(&g_bdos_desc);
    lang_desc_register(&g_swap_desc);
    lang_desc_register(&g_job_desc);
    lang_desc_register(&g_himem_desc);
    lang_desc_register(&g_lomem_desc);
}

BValue func_vintage_vars_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)args; (void)err; (void)arg_count;
    BValue res; runtime_memset(&res, 0, sizeof(res));

    if (uname && (runtime_strcasecmp(uname, "ST") == 0)) {
        res.type = VAL_INTEGER; res.as.number = 0.0; return res;
    }
    if (uname && (runtime_strcasecmp(uname, "CONSOL") == 0)) {
        res.type = VAL_INTEGER; res.as.number = 7.0; return res;
    }
    if (uname && (runtime_strcasecmp(uname, "USER") == 0)) {
        res.type = VAL_INTEGER; res.as.number = 0.0; return res;
    }
    if (uname && (runtime_strcasecmp(uname, "BDOS") == 0 || runtime_strcasecmp(uname, "BIOS") == 0)) {
        res.type = VAL_INTEGER; res.as.number = 5.0; return res;
    }
    if (uname && (runtime_strcasecmp(uname, "SWAP$") == 0)) {
        res.type = VAL_STRING; res.as.string = str_create(vm_get_str(vm), "INCORE", 6); return res;
    }
    if (uname && (runtime_strcasecmp(uname, "JOB$") == 0)) {
        res.type = VAL_STRING; res.as.string = str_create(vm_get_str(vm), "1", 1); return res;
    }
    if (uname && (runtime_strcasecmp(uname, "JOB") == 0)) {
        res.type = VAL_INTEGER; res.as.number = 1.0; return res;
    }
    if (uname && (runtime_strcasecmp(uname, "HIMEM") == 0 || runtime_strcasecmp(uname, "MAXRAM") == 0)) {
        res.type = VAL_NUMBER; res.as.number = 65535.0; return res;
    }
    if (uname && (runtime_strcasecmp(uname, "LOMEM") == 0)) {
        res.type = VAL_NUMBER; res.as.number = 2048.0; return res;
    }

    res.type = VAL_INTEGER; res.as.number = 0.0;
    return res;
}
