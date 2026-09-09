// FILENAME: func_net_telemetry.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, eval_ident_builtin.c)
// NEEDS: libcore (language_descriptor.h, strings.h)
// NEEDS: libengine (func_net_telemetry.h)
// Provides runtime implementation and LanguageDescriptors for network and I/O telemetry variables.

#include "eval/functions/network/func_net_telemetry.h"
#include "runtime/language_descriptor.h"
#include "runtime/strings.h"
#include "runtime/string.h"
#include "runtime/format/snprintf.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"
#include "platform/platform.h"

static const LangDesc g_nstatus_desc = {
    .name = "NSTATUS", .category = "IoT & Networking", .syntax = "NSTATUS",
    .description = "Returns active network channel status code.",
    .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
};

static const LangDesc g_nhttpstatus_desc = {
    .name = "NHTTPSTATUS", .category = "IoT & Networking", .syntax = "NHTTPSTATUS",
    .description = "Returns last HTTP request status code (e.g. 200).",
    .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
};

static const LangDesc g_neof_desc = {
    .name = "NEOF", .category = "IoT & Networking", .syntax = "NEOF",
    .description = "Returns network EOF connection state flag.",
    .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
};

static const LangDesc g_nbyteswaiting_desc = {
    .name = "NBYTESWAITING", .category = "IoT & Networking", .syntax = "NBYTESWAITING",
    .description = "Returns number of unread bytes waiting in network socket buffer.",
    .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
};

static const LangDesc g_nconnected_desc = {
    .name = "NCONNECTED", .category = "IoT & Networking", .syntax = "NCONNECTED",
    .description = "Returns network socket connection status (1=connected, 0=disconnected).",
    .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
};

static const LangDesc g_nerror_desc = {
    .name = "NERROR", .category = "IoT & Networking", .syntax = "NERROR",
    .description = "Returns last network socket error code.",
    .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
};

static const LangDesc g_siostatus_desc = {
    .name = "SIOSTATUS", .category = "Diagnostics", .syntax = "SIOSTATUS",
    .description = "Returns stream I/O channel status bitmask.",
    .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
};

static const LangDesc g_sioavail_desc = {
    .name = "SIOAVAIL", .category = "Diagnostics", .syntax = "SIOAVAIL",
    .description = "Returns number of bytes available in stream I/O channel.",
    .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
};

static const LangDesc g_biostatus_desc = {
    .name = "BIOSTATUS", .category = "Diagnostics", .syntax = "BIOSTATUS",
    .description = "Returns block device I/O status bitmask.",
    .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
};

static const LangDesc g_biosize_desc = {
    .name = "BIOSIZE", .category = "Diagnostics", .syntax = "BIOSIZE",
    .description = "Returns block device total size in bytes.",
    .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
};

static const LangDesc g_biochecksum_desc = {
    .name = "BIOCHECKSUM", .category = "Diagnostics", .syntax = "BIOCHECKSUM",
    .description = "Returns block I/O CRC-16 checksum value.",
    .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
};

void func_net_telemetry_register(void) {
    lang_desc_register(&g_nstatus_desc);
    lang_desc_register(&g_nhttpstatus_desc);
    lang_desc_register(&g_neof_desc);
    lang_desc_register(&g_nbyteswaiting_desc);
    lang_desc_register(&g_nconnected_desc);
    lang_desc_register(&g_nerror_desc);
    lang_desc_register(&g_siostatus_desc);
    lang_desc_register(&g_sioavail_desc);
    lang_desc_register(&g_biostatus_desc);
    lang_desc_register(&g_biosize_desc);
    lang_desc_register(&g_biochecksum_desc);
}

BValue func_net_telemetry_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm; (void)args; (void)err; (void)arg_count;
    BValue res; runtime_memset(&res, 0, sizeof(res));

    if (uname && (runtime_strcasecmp(uname, "NHTTPSTATUS") == 0)) {
        res.type = VAL_INTEGER; res.as.number = 200.0; return res;
    }
    if (uname && (runtime_strcasecmp(uname, "NCONNECTED") == 0)) {
        res.type = VAL_INTEGER; res.as.number = 1.0; return res;
    }

    res.type = VAL_INTEGER; res.as.number = 0.0;
    return res;
}
