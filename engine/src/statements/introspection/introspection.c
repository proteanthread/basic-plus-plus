// FILENAME: introspection.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, memops.h)
// NEEDS: libengine (introspection.h, stmt.h)
// NEEDS: libkernel (security.h, vdev.h)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for system diagnostic statements in BASIC++.

#include "statements/introspection/introspection.h"
#include "stmt/stmt.h"
#include "platform/platform.h"
#include "security/security.h"
#include "device/vdev.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "hal/hal.h"
#include "runtime/math/math.h"
#include "runtime/num_format.h"

static const LangDesc g_hostname_desc = {
    .name = "HOSTNAME", .category = "Introspection", .syntax = "HOSTNAME",
    .description = "Outputs the system hostname to console.",
    .error_summary = "Error 70: Permission Denied", .subsystem = SUBSYSTEM_PLATFORM,
    .safety = SAFETY_SYSTEM, .type = FEATURE_STATEMENT
};

static const LangDesc g_username_desc = {
    .name = "USERNAME", .category = "Introspection", .syntax = "USERNAME",
    .description = "Outputs the current user identity name to console.",
    .error_summary = "Error 70: Permission Denied", .subsystem = SUBSYSTEM_PLATFORM,
    .safety = SAFETY_SYSTEM, .type = FEATURE_STATEMENT
};

static const LangDesc g_comspec_desc = {
    .name = "COMSPEC", .category = "Introspection", .syntax = "COMSPEC",
    .description = "Outputs the active command shell interpreter path to console.",
    .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM,
    .safety = SAFETY_SAFE, .type = FEATURE_STATEMENT
};

static const LangDesc g_logname_desc = {
    .name = "LOGNAME", .category = "Introspection", .syntax = "LOGNAME",
    .description = "Outputs the active login user account name to console.",
    .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM,
    .safety = SAFETY_SAFE, .type = FEATURE_STATEMENT
};

static const LangDesc g_homepath_desc = {
    .name = "HOMEPATH", .category = "Introspection", .syntax = "HOMEPATH",
    .description = "Outputs the user home directory path to console.",
    .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM,
    .safety = SAFETY_SAFE, .type = FEATURE_STATEMENT
};

static const LangDesc g_homedrive_desc = {
    .name = "HOMEDRIVE", .category = "Introspection", .syntax = "HOMEDRIVE",
    .description = "Outputs the host home drive letter to console.",
    .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM,
    .safety = SAFETY_SAFE, .type = FEATURE_STATEMENT
};

static const LangDesc g_userpath_desc = {
    .name = "USERPATH", .category = "Introspection", .syntax = "USERPATH",
    .description = "Outputs the full user profile directory path to console.",
    .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM,
    .safety = SAFETY_SAFE, .type = FEATURE_STATEMENT
};

static const LangDesc g_computername_desc = {
    .name = "COMPUTERNAME", .category = "Introspection", .syntax = "COMPUTERNAME",
    .description = "Outputs the host computer machine name to console.",
    .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM,
    .safety = SAFETY_SAFE, .type = FEATURE_STATEMENT
};

static const LangDesc g_totalmem_desc = {
    .name = "TOTALMEM", .category = "Introspection", .syntax = "TOTALMEM",
    .description = "Outputs total installed physical system memory in bytes to console.",
    .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM,
    .safety = SAFETY_SAFE, .type = FEATURE_STATEMENT
};

static const LangDesc g_availmem_desc = {
    .name = "AVAILMEM", .category = "Introspection", .syntax = "AVAILMEM",
    .description = "Outputs free available physical system memory in bytes to console.",
    .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM,
    .safety = SAFETY_SAFE, .type = FEATURE_STATEMENT
};

static const LangDesc g_uptime_desc = {
    .name = "UPTIME", .category = "Introspection", .syntax = "UPTIME",
    .description = "Outputs host operating system uptime in seconds to console.",
    .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM,
    .safety = SAFETY_SAFE, .type = FEATURE_STATEMENT
};

static const LangDesc g_epoch_desc = {
    .name = "EPOCH", .category = "Introspection", .syntax = "EPOCH",
    .description = "Outputs seconds elapsed since 1980-01-01 MS-DOS epoch to console.",
    .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM,
    .safety = SAFETY_SAFE, .type = FEATURE_STATEMENT
};

static const LangDesc g_unixtime_desc = {
    .name = "UNIXTIME", .category = "Introspection", .syntax = "UNIXTIME",
    .description = "Outputs seconds elapsed since 1970-01-01 Unix epoch to console.",
    .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM,
    .safety = SAFETY_SAFE, .type = FEATURE_STATEMENT
};

static const LangDesc g_stardate_desc = {
    .name = "STARDATE", .category = "Introspection", .syntax = "STARDATE",
    .description = "Outputs CP/M days with tenth-day decimal to console.",
    .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM,
    .safety = SAFETY_SAFE, .type = FEATURE_STATEMENT
};

void stmt_introspection_register(void) {
    lang_desc_register(&g_hostname_desc);
    lang_desc_register(&g_username_desc);
    lang_desc_register(&g_comspec_desc);
    lang_desc_register(&g_logname_desc);
    lang_desc_register(&g_homepath_desc);
    lang_desc_register(&g_homedrive_desc);
    lang_desc_register(&g_userpath_desc);
    lang_desc_register(&g_computername_desc);
    lang_desc_register(&g_totalmem_desc);
    lang_desc_register(&g_availmem_desc);
    lang_desc_register(&g_uptime_desc);
    lang_desc_register(&g_epoch_desc);
    lang_desc_register(&g_unixtime_desc);
    lang_desc_register(&g_stardate_desc);
}

void stmt_introspection_register_all(VMContext *vm) {
    StmtRegistry *reg = vm_get_stmt_registry(vm);
    if (!reg) return;
    stmt_register(reg, KW_HOSTNAME, stmt_hostname_handler, "HOSTNAME", STMT_FLAG_BOTH);
    stmt_register(reg, KW_USERNAME, stmt_username_handler, "USERNAME", STMT_FLAG_BOTH);
    stmt_register(reg, KW_COMSPEC, stmt_comspec_handler, "COMSPEC", STMT_FLAG_BOTH);
    stmt_register(reg, KW_LOGNAME, stmt_logname_handler, "LOGNAME", STMT_FLAG_BOTH);
    stmt_register(reg, KW_HOMEPATH, stmt_homepath_handler, "HOMEPATH", STMT_FLAG_BOTH);
    stmt_register(reg, KW_HOMEDRIVE, stmt_homedrive_handler, "HOMEDRIVE", STMT_FLAG_BOTH);
    stmt_register(reg, KW_USERPATH, stmt_userpath_handler, "USERPATH", STMT_FLAG_BOTH);
    stmt_register(reg, KW_COMPUTERNAME, stmt_computername_handler, "COMPUTERNAME", STMT_FLAG_BOTH);
    stmt_register(reg, KW_TOTALMEM, stmt_totalmem_handler, "TOTALMEM", STMT_FLAG_BOTH);
    stmt_register(reg, KW_AVAILMEM, stmt_availmem_handler, "AVAILMEM", STMT_FLAG_BOTH);
    stmt_register(reg, KW_UPTIME, stmt_uptime_handler, "UPTIME", STMT_FLAG_BOTH);
    stmt_register(reg, KW_EPOCH, stmt_epoch_handler, "EPOCH", STMT_FLAG_BOTH);
    stmt_register(reg, KW_UNIXTIME, stmt_unixtime_handler, "UNIXTIME", STMT_FLAG_BOTH);
    stmt_register(reg, KW_STARDATE, stmt_stardate_handler, "STARDATE", STMT_FLAG_BOTH);
}

BppError stmt_hostname_handler(VMContext *vm, LexerContext *lex) {
    BppError err; runtime_memset(&err, 0, sizeof(err)); (void)lex;
    if (security_check(SECOP_SYSTEM, 0) != 0) { err.code = 70; err.message = "Permission denied"; return err; }
    char name[256] = ""; platform_get_hostname(name, sizeof(name));
    VDevContext *vdev = vm_get_vdev(vm); vdev_printf(vdev, "%s\n", name);
    return err;
}

BppError stmt_username_handler(VMContext *vm, LexerContext *lex) {
    BppError err; runtime_memset(&err, 0, sizeof(err)); (void)lex;
    if (security_check(SECOP_SYSTEM, 0) != 0) { err.code = 70; err.message = "Permission denied"; return err; }
    char name[256] = ""; platform_get_username(name, sizeof(name));
    VDevContext *vdev = vm_get_vdev(vm); vdev_printf(vdev, "%s\n", name);
    return err;
}

BppError stmt_comspec_handler(VMContext *vm, LexerContext *lex) {
    BppError err; runtime_memset(&err, 0, sizeof(err)); (void)lex;
    const char *cs = platform_getenv("COMSPEC");
    if (!cs) cs = platform_getenv("SHELL");
    if (!cs) cs = "cmd.exe";
    VDevContext *vdev = vm_get_vdev(vm); vdev_printf(vdev, "%s\n", cs);
    return err;
}

BppError stmt_logname_handler(VMContext *vm, LexerContext *lex) {
    BppError err; runtime_memset(&err, 0, sizeof(err)); (void)lex;
    const char *un = platform_getenv("USERNAME");
    if (!un) un = platform_getenv("LOGNAME");
    if (!un) un = platform_getenv("USER");
    if (!un) un = "user";
    VDevContext *vdev = vm_get_vdev(vm); vdev_printf(vdev, "%s\n", un);
    return err;
}

BppError stmt_homepath_handler(VMContext *vm, LexerContext *lex) {
    BppError err; runtime_memset(&err, 0, sizeof(err)); (void)lex;
    const char *hp = platform_getenv("HOMEPATH");
    if (!hp) hp = platform_getenv("HOME");
    if (!hp) hp = "\\";
    VDevContext *vdev = vm_get_vdev(vm); vdev_printf(vdev, "%s\n", hp);
    return err;
}

BppError stmt_homedrive_handler(VMContext *vm, LexerContext *lex) {
    BppError err; runtime_memset(&err, 0, sizeof(err)); (void)lex;
    const char *hd = platform_getenv("HOMEDRIVE");
    if (!hd) hd = platform_getenv("SYSTEMDRIVE");
    if (!hd) hd = "C:";
    VDevContext *vdev = vm_get_vdev(vm); vdev_printf(vdev, "%s\n", hd);
    return err;
}

BppError stmt_userpath_handler(VMContext *vm, LexerContext *lex) {
    BppError err; runtime_memset(&err, 0, sizeof(err)); (void)lex;
    const char *up = platform_getenv("USERPROFILE");
    if (!up) up = platform_getenv("HOME");
    if (!up) up = "";
    VDevContext *vdev = vm_get_vdev(vm); vdev_printf(vdev, "%s\n", up);
    return err;
}

BppError stmt_computername_handler(VMContext *vm, LexerContext *lex) {
    BppError err; runtime_memset(&err, 0, sizeof(err)); (void)lex;
    char name[256] = "";
    const char *cn = platform_getenv("COMPUTERNAME");
    if (!cn) cn = platform_getenv("HOSTNAME");
    if (cn && *cn) {
        runtime_strncpy(name, cn, sizeof(name) - 1);
    } else {
        platform_get_hostname(name, sizeof(name));
    }
    VDevContext *vdev = vm_get_vdev(vm); vdev_printf(vdev, "%s\n", name);
    return err;
}

BppError stmt_totalmem_handler(VMContext *vm, LexerContext *lex) {
    BppError err; runtime_memset(&err, 0, sizeof(err)); (void)lex;
    double bytes = (double)platform_get_total_system_memory();
    char buf[64]; num_format_display(buf, sizeof(buf), bytes, false, true);
    VDevContext *vdev = vm_get_vdev(vm); vdev_printf(vdev, "%s\n", buf);
    return err;
}

BppError stmt_availmem_handler(VMContext *vm, LexerContext *lex) {
    BppError err; runtime_memset(&err, 0, sizeof(err)); (void)lex;
    double bytes = (double)platform_get_avail_system_memory();
    char buf[64]; num_format_display(buf, sizeof(buf), bytes, false, true);
    VDevContext *vdev = vm_get_vdev(vm); vdev_printf(vdev, "%s\n", buf);
    return err;
}

BppError stmt_uptime_handler(VMContext *vm, LexerContext *lex) {
    BppError err; runtime_memset(&err, 0, sizeof(err)); (void)lex;
    double sec = platform_get_system_uptime();
    char buf[64]; num_format_display(buf, sizeof(buf), sec, false, true);
    VDevContext *vdev = vm_get_vdev(vm); vdev_printf(vdev, "%s\n", buf);
    return err;
}

BppError stmt_epoch_handler(VMContext *vm, LexerContext *lex) {
    BppError err; runtime_memset(&err, 0, sizeof(err)); (void)lex;
    time_t raw_t = (time_t)(hal_get() && hal_get()->time.now_epoch_seconds ? hal_get()->time.now_epoch_seconds() : (int64_t)time(NULL));
    int64_t diff = (int64_t)raw_t - 315532800LL;
    if (diff < 0) diff = 0;
    char buf[64]; num_format_display(buf, sizeof(buf), (double)diff, false, true);
    VDevContext *vdev = vm_get_vdev(vm); vdev_printf(vdev, "%s\n", buf);
    return err;
}

BppError stmt_unixtime_handler(VMContext *vm, LexerContext *lex) {
    BppError err; runtime_memset(&err, 0, sizeof(err)); (void)lex;
    time_t raw_t = (time_t)(hal_get() && hal_get()->time.now_epoch_seconds ? hal_get()->time.now_epoch_seconds() : (int64_t)time(NULL));
    char buf[64]; num_format_display(buf, sizeof(buf), (double)raw_t, false, true);
    VDevContext *vdev = vm_get_vdev(vm); vdev_printf(vdev, "%s\n", buf);
    return err;
}

BppError stmt_stardate_handler(VMContext *vm, LexerContext *lex) {
    BppError err; runtime_memset(&err, 0, sizeof(err)); (void)lex;
    time_t raw_t = (time_t)(hal_get() && hal_get()->time.now_epoch_seconds ? hal_get()->time.now_epoch_seconds() : (int64_t)time(NULL));
    double cpm_start = 252460800.0; // 1978-01-01
    double elapsed_sec = ((double)raw_t >= cpm_start) ? ((double)raw_t - cpm_start) : 0.0;
    double days = runtime_floor(elapsed_sec / 86400.0);
    double day_sec = elapsed_sec - (days * 86400.0);
    double tenth = runtime_floor((day_sec / 86400.0) * 10.0);
    if (tenth > 9.0) tenth = 9.0;
    double sd = days + (tenth / 10.0);
    char buf[64]; num_format_display(buf, sizeof(buf), sd, false, true);
    VDevContext *vdev = vm_get_vdev(vm); vdev_printf(vdev, "%s\n", buf);
    return err;
}
