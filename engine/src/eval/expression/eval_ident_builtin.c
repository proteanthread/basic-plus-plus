// FILENAME: eval_ident_builtin.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (eval_ident.c, ast_eval_expr.c)
// NEEDS: libcore, libengine (eval_expr_internal.h)
// Built-in constants and system variable resolution table for expression evaluators.
//
// ---- Includes ----

#include "eval/eval_expr_internal.h"
#include "reg/reg_udx.h"
#include "eval/rpn.h"
#include "eval/pn.h"
#include "eval/functions/datetime/func_week.h"
#include "eval/functions/datetime/time_part.h"
#include "eval/functions/network/func_ip.h"
#include "eval/functions/system/environment/command_fn.h"
#include "eval/functions/system/hardware/func_baud.h"
#include "eval/functions/system/hardware/paddle.h"
#include "eval/functions/system/hardware/ptrig.h"
#include "eval/functions/system/hardware/stick.h"
#include "eval/functions/system/hardware/strig.h"
#include "eval/functions/system/terminal/csrlin.h"
#include "eval/functions/system/terminal/inkey.h"
#include "eval/functions/system/terminal/pos.h"
#include "eval/functions/system/time/date.h"
#include "eval/functions/system/time/func_tim.h"
#include "eval/functions/system/time/ti.h"
#include "eval/functions/system/time/time.h"
#include "eval/functions/system/time/time_fn.h"
#include "eval/functions/datetime/hebrew.h"
#include "eval/functions/system/environment/func_exepath.h"
#include "eval/functions/system/environment/func_scriptpath.h"
#include "eval/functions/system/environment/func_workdir.h"
#include "eval/functions/system/environment/func_program.h"
#include "eval/functions/system/environment/func_tempdir.h"
#include "eval/functions/system/environment/func_userpath.h"
#include "eval/functions/system/environment/func_ver.h"
#include "eval/functions/system/hardware/func_vintage_vars.h"
#include "eval/functions/network/func_net_telemetry.h"
#include "hardware/speed_db.h"
#include "hal/hal.h"
#include "platform/platform.h"
#include "runtime/format/snprintf.h"
#include "runtime/math/math.h"
#include "runtime/memory/alloc.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/variables_internal.h"
#include "statements/oop/sub.h"
#include "runtime/arrays.h"

// resolves built-in constant or system variable names
bool eval_try_resolve_builtin_constant_or_system_var(VMContext *vm, const char *name_buf, BValue *out_val) {
    if (!name_buf || !*name_buf || !out_val) return false;
    runtime_memset(out_val, 0, sizeof(*out_val));

    // ---- Group A: Mathematical Constants & Booleans ----
    if (runtime_strcasecmp(name_buf, "DET") == 0 || runtime_strcasecmp(name_buf, "MATH.DET") == 0) {
        out_val->type = VAL_NUMBER; out_val->as.number = (vm && vm_get_arr(vm)) ? arr_get_last_det(vm_get_arr(vm)) : 0.0; return true;
    }
    if (runtime_strcasecmp(name_buf, "TRUE") == 0 || runtime_strcasecmp(name_buf, "_TRUE") == 0) {
        out_val->type = VAL_INTEGER; out_val->as.number = 1.0; return true;
    }
    if (runtime_strcasecmp(name_buf, "FALSE") == 0 || runtime_strcasecmp(name_buf, "_FALSE") == 0) {
        out_val->type = VAL_INTEGER; out_val->as.number = -1.0; return true;
    }
    if (runtime_strcasecmp(name_buf, "PI") == 0 || runtime_strcasecmp(name_buf, "_PI") == 0 || runtime_strcasecmp(name_buf, "MATH.PI") == 0) {
        out_val->type = VAL_NUMBER; out_val->as.number = 3.14159265358979323846; return true;
    }
    if (runtime_strcasecmp(name_buf, "INF") == 0 || runtime_strcasecmp(name_buf, "_INF") == 0 || runtime_strcasecmp(name_buf, "INFINITY") == 0 || runtime_strcasecmp(name_buf, "MATH.INF") == 0) {
        out_val->type = VAL_NUMBER; out_val->as.number = RUNTIME_INFINITY; return true;
    }
    if (runtime_strcasecmp(name_buf, "MAXNUM") == 0 || runtime_strcasecmp(name_buf, "_MAXNUM") == 0 || runtime_strcasecmp(name_buf, "MATH.MAXNUM") == 0) {
        out_val->type = VAL_NUMBER; out_val->as.number = 1.7976931348623157e+308; return true;
    }
    if (runtime_strcasecmp(name_buf, "EPS") == 0 || runtime_strcasecmp(name_buf, "_EPS") == 0 || runtime_strcasecmp(name_buf, "MATH.EPS") == 0 || runtime_strcasecmp(name_buf, "EPSILON") == 0) {
        out_val->type = VAL_NUMBER; out_val->as.number = 2.220446049250313e-16; return true;
    }
    if (runtime_strcasecmp(name_buf, "NOTHING") == 0 || runtime_strcasecmp(name_buf, "NULL") == 0) {
        out_val->type = VAL_MAP; out_val->as.map = NULL; return true;
    }

    // ---- Group B: System Identification & OS Namespaces ----
    if (runtime_strcasecmp(name_buf, "SYS.OS$") == 0 || runtime_strcasecmp(name_buf, "_OS$") == 0 || runtime_strcasecmp(name_buf, "OS$") == 0) {
#if defined(_WIN32)
        const char *os = "Windows";
#elif defined(__APPLE__)
        const char *os = "macOS";
#elif defined(__linux__)
        const char *os = "Linux";
#elif defined(__FreeBSD__)
        const char *os = "FreeBSD";
#elif defined(ESP_PLATFORM) || defined(ESP32)
        const char *os = "ESP32";
#else
        const char *os = "POSIX";
#endif
        out_val->type = VAL_STRING; out_val->as.string = str_create(vm_get_str(vm), os, runtime_strlen(os)); return true;
    }

    if (runtime_strcasecmp(name_buf, "SYS.ARCH$") == 0 || runtime_strcasecmp(name_buf, "_ARCH$") == 0 || runtime_strcasecmp(name_buf, "ARCH$") == 0) {
#if defined(__x86_64__) || defined(_M_X64)
        const char *arch = "x86_64";
#elif defined(__aarch64__) || defined(_M_ARM64)
        const char *arch = "ARM64";
#elif defined(__i386__) || defined(_M_IX86)
        const char *arch = "x86";
#elif defined(__arm__) || defined(_M_ARM)
        const char *arch = "ARM";
#elif defined(__riscv)
        const char *arch = "RISC-V";
#elif defined(__XTENSA__)
        const char *arch = "Xtensa";
#else
        const char *arch = "x86_64";
#endif
        out_val->type = VAL_STRING; out_val->as.string = str_create(vm_get_str(vm), arch, runtime_strlen(arch)); return true;
    }

    if (runtime_strcasecmp(name_buf, "SYS.PLATFORM$") == 0 || runtime_strcasecmp(name_buf, "_PLATFORM$") == 0 || runtime_strcasecmp(name_buf, "PLATFORM$") == 0) {
#if defined(_WIN32)
        const char *plat = "Win32";
#elif defined(ESP_PLATFORM) || defined(ESP32)
        const char *plat = "Embedded";
#else
        const char *plat = "POSIX";
#endif
        out_val->type = VAL_STRING; out_val->as.string = str_create(vm_get_str(vm), plat, runtime_strlen(plat)); return true;
    }

    if (runtime_strcasecmp(name_buf, "SYS.MACHINE$") == 0 || runtime_strcasecmp(name_buf, "_MACHINE$") == 0 || runtime_strcasecmp(name_buf, "MACHINE$") == 0) {
        const char *mach = "x86_64 Host";
        out_val->type = VAL_STRING; out_val->as.string = str_create(vm_get_str(vm), mach, runtime_strlen(mach)); return true;
    }

    if (runtime_strcasecmp(name_buf, "SYS.EDITION$") == 0 || runtime_strcasecmp(name_buf, "_EDITION$") == 0 || runtime_strcasecmp(name_buf, "EDITION$") == 0) {
        const char *ed = "Standard Edition";
        out_val->type = VAL_STRING; out_val->as.string = str_create(vm_get_str(vm), ed, runtime_strlen(ed)); return true;
    }

    if (runtime_strcasecmp(name_buf, "SYS.PROFILE$") == 0 || runtime_strcasecmp(name_buf, "_PROFILE$") == 0 || runtime_strcasecmp(name_buf, "PROFILE$") == 0) {
        const char *prof = "Desktop";
        out_val->type = VAL_STRING; out_val->as.string = str_create(vm_get_str(vm), prof, runtime_strlen(prof)); return true;
    }

    if (runtime_strcasecmp(name_buf, "SYS.COMPILER$") == 0 || runtime_strcasecmp(name_buf, "_COMPILER$") == 0) {
#if defined(__clang__)
        const char *comp = "Clang";
#elif defined(__GNUC__)
        const char *comp = "GCC";
#elif defined(_MSC_VER)
        const char *comp = "MSVC";
#else
        const char *comp = "C17";
#endif
        out_val->type = VAL_STRING; out_val->as.string = str_create(vm_get_str(vm), comp, runtime_strlen(comp)); return true;
    }

    if (runtime_strcasecmp(name_buf, "SYS.WORDSIZE") == 0 || runtime_strcasecmp(name_buf, "_WORDSIZE") == 0) {
        out_val->type = VAL_INTEGER; out_val->as.number = (double)(sizeof(void *) * 8); return true;
    }

    if (runtime_strcasecmp(name_buf, "SYS.PTRSIZE") == 0 || runtime_strcasecmp(name_buf, "_PTRSIZE") == 0) {
        out_val->type = VAL_INTEGER; out_val->as.number = (double)(sizeof(void *)); return true;
    }

    if (runtime_strcasecmp(name_buf, "SYS.CORES") == 0 || runtime_strcasecmp(name_buf, "_CORES") == 0) {
        int cores = platform_get_cpu_cores();
        if (cores < 1) cores = 4;
        out_val->type = VAL_INTEGER; out_val->as.number = (double)cores; return true;
    }

    if (runtime_strcasecmp(name_buf, "CPUSPEED") == 0 || runtime_strcasecmp(name_buf, "SYS.CPUSPEED") == 0) {
        out_val->type = VAL_NUMBER; out_val->as.number = plat_hw_get_cpu_hz(); return true;
    }
    if (runtime_strcasecmp(name_buf, "CPUSPEED$") == 0 || runtime_strcasecmp(name_buf, "SYS.CPUSPEED$") == 0) {
        char buf[32]; speed_db_format_unit(plat_hw_get_cpu_hz(), buf, sizeof(buf));
        out_val->type = VAL_STRING; out_val->as.string = str_create(vm_get_str(vm), buf, runtime_strlen(buf)); return true;
    }
    if (runtime_strcasecmp(name_buf, "CLOCKS") == 0 || runtime_strcasecmp(name_buf, "SYS.CLOCKS") == 0) {
        out_val->type = VAL_NUMBER; out_val->as.number = (double)plat_hw_get_cycle_count(); return true;
    }
    if (runtime_strcasecmp(name_buf, "CLOCKS$") == 0 || runtime_strcasecmp(name_buf, "SYS.CLOCKS$") == 0) {
        char c_buf[32], b_buf[32], m_buf[32], sum[128];
        speed_db_format_unit(plat_hw_get_cpu_hz(), c_buf, sizeof(c_buf));
        speed_db_format_unit(plat_hw_get_bus_hz(), b_buf, sizeof(b_buf));
        speed_db_format_unit(plat_hw_get_mem_hz(), m_buf, sizeof(m_buf));
        runtime_snprintf(sum, sizeof(sum), "CPU: %s, Bus: %s, Mem: %s", c_buf, b_buf, m_buf);
        out_val->type = VAL_STRING; out_val->as.string = str_create(vm_get_str(vm), sum, runtime_strlen(sum)); return true;
    }

    if (runtime_strcasecmp(name_buf, "SYS.VERSION$") == 0 || runtime_strcasecmp(name_buf, "_VERSION$") == 0 || runtime_strcasecmp(name_buf, "_VER$") == 0 || runtime_strcasecmp(name_buf, "VERSION$") == 0) {
        const char *v = "6.5.2";
        out_val->type = VAL_STRING; out_val->as.string = str_create(vm_get_str(vm), v, runtime_strlen(v)); return true;
    }

    if (runtime_strcasecmp(name_buf, "SYS.HOSTNAME$") == 0 || runtime_strcasecmp(name_buf, "HOSTNAME$") == 0) {
        char hbuf[128] = {0};
        platform_get_hostname(hbuf, sizeof(hbuf));
        out_val->type = VAL_STRING; out_val->as.string = str_create(vm_get_str(vm), hbuf, runtime_strlen(hbuf)); return true;
    }

    if (runtime_strcasecmp(name_buf, "SYS.USERNAME$") == 0 || runtime_strcasecmp(name_buf, "USERNAME$") == 0) {
        char ubuf[128] = {0};
        platform_get_username(ubuf, sizeof(ubuf));
        out_val->type = VAL_STRING; out_val->as.string = str_create(vm_get_str(vm), ubuf, runtime_strlen(ubuf)); return true;
    }

    if (runtime_strcasecmp(name_buf, "SYS.PID") == 0 || runtime_strcasecmp(name_buf, "_PID") == 0) {
        out_val->type = VAL_INTEGER; out_val->as.number = (double)platform_get_pid(); return true;
    }

    if (runtime_strcasecmp(name_buf, "SYS.UPTIME") == 0 || runtime_strcasecmp(name_buf, "UPTIME") == 0) {
        out_val->type = VAL_NUMBER; out_val->as.number = platform_get_system_uptime(); return true;
    }

    if (runtime_strcasecmp(name_buf, "SYS.KERNEL$") == 0 || runtime_strcasecmp(name_buf, "KERNEL$") == 0) {
#if defined(_WIN32)
        const char *k = "Windows NT 10.0";
#elif defined(ESP_PLATFORM) || defined(ESP32)
        const char *k = "FreeRTOS / ESP-IDF";
#elif defined(__FreeBSD__)
        const char *k = "FreeBSD Kernel";
#elif defined(__APPLE__)
        const char *k = "Darwin / XNU";
#elif defined(__linux__)
        const char *k = "Linux 6.x";
#else
        const char *k = "FreeDOS Kernel 2043";
#endif
        out_val->type = VAL_STRING; out_val->as.string = str_create(vm_get_str(vm), k, runtime_strlen(k)); return true;
    }

    if (runtime_strcasecmp(name_buf, "SYS.DOSVER$") == 0 || runtime_strcasecmp(name_buf, "DOSVER$") == 0) {
#if defined(_WIN32)
        const char *dv = "10.0";
#else
        const char *dv = "7.10";
#endif
        out_val->type = VAL_STRING; out_val->as.string = str_create(vm_get_str(vm), dv, runtime_strlen(dv)); return true;
    }
    if (runtime_strcasecmp(name_buf, "SYS.DOSVER") == 0 || runtime_strcasecmp(name_buf, "DOSVER") == 0) {
#if defined(_WIN32)
        out_val->type = VAL_NUMBER; out_val->as.number = 10.0; return true;
#else
        out_val->type = VAL_NUMBER; out_val->as.number = 7.10; return true;
#endif
    }

    if (runtime_strcasecmp(name_buf, "SYS.WINVER$") == 0 || runtime_strcasecmp(name_buf, "WINVER$") == 0) {
#if defined(_WIN32)
        const char *wv = "11.0";
#else
        const char *wv = "0.0";
#endif
        out_val->type = VAL_STRING; out_val->as.string = str_create(vm_get_str(vm), wv, runtime_strlen(wv)); return true;
    }
    if (runtime_strcasecmp(name_buf, "SYS.WINVER") == 0 || runtime_strcasecmp(name_buf, "WINVER") == 0) {
#if defined(_WIN32)
        out_val->type = VAL_NUMBER; out_val->as.number = 11.0; return true;
#else
        out_val->type = VAL_NUMBER; out_val->as.number = 0.0; return true;
#endif
    }

    if (runtime_strcasecmp(name_buf, "SYS.DRIVE$") == 0 || runtime_strcasecmp(name_buf, "DRIVE$") == 0) {
        char pbuf[1024] = {0}; platform_getcwd(pbuf, sizeof(pbuf) - 1);
        char dbuf[4] = {0};
        if (pbuf[0] && pbuf[1] == ':') {
            dbuf[0] = pbuf[0]; dbuf[1] = ':'; dbuf[2] = '\0';
        } else {
            dbuf[0] = '/'; dbuf[1] = '\0';
        }
        out_val->type = VAL_STRING; out_val->as.string = str_create(vm_get_str(vm), dbuf, runtime_strlen(dbuf)); return true;
    }

    if (runtime_strcasecmp(name_buf, "SYS.TEMPDIR$") == 0 || runtime_strcasecmp(name_buf, "TEMPDIR$") == 0 ||
        runtime_strcasecmp(name_buf, "TMPDIR$") == 0 || runtime_strcasecmp(name_buf, "TEMP$") == 0 ||
        runtime_strcasecmp(name_buf, "TMP$") == 0) {
        *out_val = func_tempdir_eval(vm, name_buf, 0, NULL, NULL); return true;
    }

    if (runtime_strcasecmp(name_buf, "SYS.MEMORY.TOTAL") == 0 || runtime_strcasecmp(name_buf, "TOTALMEM") == 0 || runtime_strcasecmp(name_buf, "SYS.TOTALMEM") == 0) {
        out_val->type = VAL_NUMBER; out_val->as.number = (double)platform_get_total_system_memory(); return true;
    }
    if (runtime_strcasecmp(name_buf, "SYS.MEMORY.AVAIL") == 0 || runtime_strcasecmp(name_buf, "AVAILMEM") == 0 || runtime_strcasecmp(name_buf, "SYS.AVAILMEM") == 0) {
        out_val->type = VAL_NUMBER; out_val->as.number = (double)platform_get_avail_system_memory(); return true;
    }

    if (runtime_strcasecmp(name_buf, "SYS.ISADMIN") == 0 || runtime_strcasecmp(name_buf, "SYS.ISROOT") == 0 ||
        runtime_strcasecmp(name_buf, "ISADMIN") == 0 || runtime_strcasecmp(name_buf, "ISROOT") == 0) {
        out_val->type = VAL_INTEGER; out_val->as.number = 0.0; return true;
    }

    // ---- Group C: Clocks, Timers, Astronomy & ISO 8601 ----
    time_t raw_t = (time_t)(hal_get() && hal_get()->time.now_epoch_seconds ? hal_get()->time.now_epoch_seconds() : 1772452800);
    struct tm tm_b;
    struct tm *lt = platform_localtime(&raw_t, &tm_b);
    if (!lt) { tm_b.tm_hour = 12; tm_b.tm_min = 0; tm_b.tm_sec = 0; tm_b.tm_year = 126; tm_b.tm_mon = 7; tm_b.tm_mday = 31; lt = &tm_b; }

    struct tm tm_b_utc;
    struct tm *gt = platform_gmtime(&raw_t, &tm_b_utc);
    if (!gt) { tm_b_utc = tm_b; gt = &tm_b_utc; }

    if (runtime_strcasecmp(name_buf, "CLK") == 0) {
        out_val->type = VAL_NUMBER; out_val->as.number = (double)(gt->tm_hour * 10000 + gt->tm_min * 100 + gt->tm_sec); return true;
    }
    if (runtime_strcasecmp(name_buf, "CLK$") == 0) {
        int h12 = (gt->tm_hour % 12 == 0 ? 12 : gt->tm_hour % 12);
        char buf[24]; runtime_snprintf(buf, sizeof(buf), "%02d:%02d:%02d %s",
                                      h12, gt->tm_min, gt->tm_sec, (gt->tm_hour >= 12 ? "PM" : "AM"));
        out_val->type = VAL_STRING; out_val->as.string = str_create(vm_get_str(vm), buf, runtime_strlen(buf)); return true;
    }
    if (runtime_strcasecmp(name_buf, "TICKS") == 0) {
        out_val->type = VAL_NUMBER; out_val->as.number = (double)(uint64_t)(platform_get_uptime() * 60.0); return true;
    }
    if (runtime_strcasecmp(name_buf, "JIFFIES") == 0) {
        out_val->type = VAL_NUMBER; out_val->as.number = (double)(uint64_t)(platform_get_uptime() * 50.0); return true;
    }
    if (runtime_strcasecmp(name_buf, "TICKS_MS") == 0 || runtime_strcasecmp(name_buf, "_TICKS") == 0) {
        out_val->type = VAL_NUMBER; out_val->as.number = (double)((long)(platform_get_uptime() * 1000.0)); return true;
    }
    if (runtime_strcasecmp(name_buf, "TICKS_US") == 0) {
        out_val->type = VAL_NUMBER; out_val->as.number = (double)((long)(platform_get_uptime() * 1000000.0)); return true;
    }
    if (runtime_strcasecmp(name_buf, "EPOCH") == 0) {
        int64_t diff = (int64_t)raw_t - 315532800LL;
        if (diff < 0) diff = 0;
        out_val->type = VAL_NUMBER; out_val->as.number = (double)diff; return true;
    }
    if (runtime_strcasecmp(name_buf, "EPOCH.MS") == 0) {
        int64_t diff = (int64_t)raw_t - 315532800LL;
        if (diff < 0) diff = 0;
        out_val->type = VAL_NUMBER; out_val->as.number = (double)diff * 1000.0 + (double)((uint64_t)(platform_get_uptime() * 1000.0) % 1000); return true;
    }
    if (runtime_strcasecmp(name_buf, "UNIXTIME") == 0) {
        out_val->type = VAL_NUMBER; out_val->as.number = (double)raw_t; return true;
    }
    if (runtime_strcasecmp(name_buf, "UTC") == 0) {
        double utc_num = (double)(gt->tm_year + 1900) * 10000000000.0 + (double)(gt->tm_mon + 1) * 100000000.0 +
                         (double)(gt->tm_mday) * 1000000.0 + (double)(gt->tm_hour) * 10000.0 +
                         (double)(gt->tm_min) * 100.0 + (double)(gt->tm_sec);
        out_val->type = VAL_NUMBER; out_val->as.number = utc_num; return true;
    }
    if (runtime_strcasecmp(name_buf, "UTC$") == 0) {
        char buf[32]; runtime_snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d",
                                      gt->tm_year + 1900, gt->tm_mon + 1, gt->tm_mday,
                                      gt->tm_hour, gt->tm_min, gt->tm_sec);
        out_val->type = VAL_STRING; out_val->as.string = str_create(vm_get_str(vm), buf, runtime_strlen(buf)); return true;
    }
    if (runtime_strcasecmp(name_buf, "CLOCK") == 0) {
        double clk_num = (double)(lt->tm_year + 1900) * 10000000000.0 + (double)(lt->tm_mon + 1) * 100000000.0 +
                         (double)(lt->tm_mday) * 1000000.0 + (double)(lt->tm_hour) * 10000.0 +
                         (double)(lt->tm_min) * 100.0 + (double)(lt->tm_sec);
        out_val->type = VAL_NUMBER; out_val->as.number = clk_num; return true;
    }
    if (runtime_strcasecmp(name_buf, "CLOCK$") == 0) {
        char buf[32]; runtime_snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d",
                                      lt->tm_year + 1900, lt->tm_mon + 1, lt->tm_mday,
                                      lt->tm_hour, lt->tm_min, lt->tm_sec);
        out_val->type = VAL_STRING; out_val->as.string = str_create(vm_get_str(vm), buf, runtime_strlen(buf)); return true;
    }
    if (runtime_strcasecmp(name_buf, "TIME$") == 0) {
        *out_val = func_time_eval(vm, "TIME$", 0, NULL, NULL); return true;
    }
    if (runtime_strcasecmp(name_buf, "TIME") == 0) {
        out_val->type = VAL_NUMBER; out_val->as.number = (double)(lt->tm_hour * 10000 + lt->tm_min * 100 + lt->tm_sec); return true;
    }
    if (runtime_strcasecmp(name_buf, "YEAR") == 0) {
        out_val->type = VAL_NUMBER; out_val->as.number = (double)(lt->tm_year + 1900); return true;
    }
    if (runtime_strcasecmp(name_buf, "YEAR$") == 0) {
        char buf[32]; runtime_snprintf(buf, sizeof(buf), "%04d AD", lt->tm_year + 1900);
        out_val->type = VAL_STRING; out_val->as.string = str_create(vm_get_str(vm), buf, runtime_strlen(buf)); return true;
    }
    if (runtime_strcasecmp(name_buf, "MONTH") == 0) {
        out_val->type = VAL_NUMBER; out_val->as.number = (double)(lt->tm_mon + 1); return true;
    }
    if (runtime_strcasecmp(name_buf, "DAY") == 0) {
        out_val->type = VAL_NUMBER; out_val->as.number = (double)(lt->tm_mday); return true;
    }
    if (runtime_strcasecmp(name_buf, "WEEKDAY") == 0) {
        out_val->type = VAL_NUMBER; out_val->as.number = (double)(lt->tm_wday + 1); return true;
    }
    if (runtime_strcasecmp(name_buf, "WEEK") == 0) {
        *out_val = func_week_eval(vm, "WEEK", 0, NULL, NULL); return true;
    }
    if (runtime_strcasecmp(name_buf, "WEEK$") == 0) {
        *out_val = func_week_str_eval(vm, "WEEK$", 0, NULL, NULL); return true;
    }
    if (runtime_strcasecmp(name_buf, "HOUR") == 0) {
        out_val->type = VAL_NUMBER; out_val->as.number = (double)(lt->tm_hour); return true;
    }
    if (runtime_strcasecmp(name_buf, "HOURS") == 0) {
        int h12 = (lt->tm_hour % 12 == 0 ? 12 : lt->tm_hour % 12);
        out_val->type = VAL_NUMBER; out_val->as.number = (double)h12; return true;
    }
    if (runtime_strcasecmp(name_buf, "HOUR$") == 0) {
        int h12 = (lt->tm_hour % 12 == 0 ? 12 : lt->tm_hour % 12);
        char buf[16]; runtime_snprintf(buf, sizeof(buf), "%02d %s", h12, (lt->tm_hour >= 12 ? "PM" : "AM"));
        out_val->type = VAL_STRING; out_val->as.string = str_create(vm_get_str(vm), buf, runtime_strlen(buf)); return true;
    }
    if (runtime_strcasecmp(name_buf, "MINUTE") == 0) {
        out_val->type = VAL_NUMBER; out_val->as.number = (double)(lt->tm_min); return true;
    }
    if (runtime_strcasecmp(name_buf, "MINUTES") == 0) {
        double timer_val = platform_get_timer();
        double sub_sec = timer_val - runtime_floor(timer_val);
        double mins = (double)lt->tm_min + ((double)lt->tm_sec + sub_sec) / 60.0;
        out_val->type = VAL_NUMBER; out_val->as.number = runtime_round(mins * 10000.0) / 10000.0; return true;
    }
    if (runtime_strcasecmp(name_buf, "MINUTE$") == 0) {
        char buf[16]; runtime_snprintf(buf, sizeof(buf), "%02d", lt->tm_min);
        out_val->type = VAL_STRING; out_val->as.string = str_create(vm_get_str(vm), buf, runtime_strlen(buf)); return true;
    }
    if (runtime_strcasecmp(name_buf, "SECOND") == 0) {
        out_val->type = VAL_NUMBER; out_val->as.number = (double)(lt->tm_sec); return true;
    }
    if (runtime_strcasecmp(name_buf, "SECONDS") == 0) {
        double timer_val = platform_get_timer();
        double sub_sec = timer_val - runtime_floor(timer_val);
        double secs = (double)lt->tm_sec + sub_sec;
        out_val->type = VAL_NUMBER; out_val->as.number = runtime_round(secs * 10000.0) / 10000.0; return true;
    }
    if (runtime_strcasecmp(name_buf, "SECOND$") == 0) {
        char buf[16]; runtime_snprintf(buf, sizeof(buf), "%02d", lt->tm_sec);
        out_val->type = VAL_STRING; out_val->as.string = str_create(vm_get_str(vm), buf, runtime_strlen(buf)); return true;
    }
    if (runtime_strcasecmp(name_buf, "JD") == 0 || runtime_strcasecmp(name_buf, "JULIAN") == 0) {
        out_val->type = VAL_NUMBER; out_val->as.number = 2440587.5 + (double)raw_t / 86400.0; return true;
    }
    if (runtime_strcasecmp(name_buf, "JULIAN$") == 0) {
        char buf[32]; runtime_snprintf(buf, sizeof(buf), "%.5f", 2440587.5 + (double)raw_t / 86400.0);
        out_val->type = VAL_STRING; out_val->as.string = str_create(vm_get_str(vm), buf, runtime_strlen(buf)); return true;
    }
    if (runtime_strcasecmp(name_buf, "MJD") == 0) {
        out_val->type = VAL_NUMBER; out_val->as.number = (2440587.5 + (double)raw_t / 86400.0) - 2400000.5; return true;
    }
    if (runtime_strcasecmp(name_buf, "GMST") == 0) {
        out_val->type = VAL_NUMBER; out_val->as.number = 12.0; return true;
    }
    if (runtime_strcasecmp(name_buf, "TI") == 0) {
        *out_val = func_ti_eval(vm, "TI", 0, NULL, NULL); return true;
    }
    if (runtime_strcasecmp(name_buf, "TI$") == 0) {
        *out_val = func_ti_eval(vm, "TI$", 0, NULL, NULL); return true;
    }
    if (runtime_strcasecmp(name_buf, "TIMER$") == 0) {
        *out_val = func_ti_eval(vm, "TIMER$", 0, NULL, NULL); return true;
    }
    if (runtime_strcasecmp(name_buf, "TIME4") == 0) {
        out_val->type = VAL_INTEGER; out_val->as.number = (double)(lt->tm_hour * 100 + lt->tm_min); return true;
    }
    if (runtime_strcasecmp(name_buf, "DATE") == 0) {
        out_val->type = VAL_NUMBER; out_val->as.number = (double)((lt->tm_year + 1900) * 10000 + (lt->tm_mon + 1) * 100 + lt->tm_mday); return true;
    }
    if (runtime_strcasecmp(name_buf, "DATE4$") == 0) {
        char buf[16]; runtime_snprintf(buf, sizeof(buf), "%04d-%02d-%02d", lt->tm_year + 1900, lt->tm_mon + 1, lt->tm_mday);
        out_val->type = VAL_STRING; out_val->as.string = str_create(vm_get_str(vm), buf, runtime_strlen(buf)); return true;
    }
    if (runtime_strcasecmp(name_buf, "DATE$") == 0) {
        *out_val = func_date_eval(vm, "DATE$", 0, NULL, NULL); return true;
    }
    if (runtime_strcasecmp(name_buf, "DAY$") == 0) {
        *out_val = func_date_eval(vm, "DAY$", 0, NULL, NULL); return true;
    }
    if (runtime_strcasecmp(name_buf, "MONTH$") == 0) {
        *out_val = func_date_eval(vm, "MONTH$", 0, NULL, NULL); return true;
    }
    if (runtime_strcasecmp(name_buf, "TZ$") == 0) {
        out_val->type = VAL_STRING; out_val->as.string = str_create(vm_get_str(vm), "+00:00", 6); return true;
    }
    if (runtime_strcasecmp(name_buf, "UPTIME$") == 0) {
        double up = platform_get_system_uptime();
        long secs = (long)up;
        long d = secs / 86400;
        long h = (secs % 86400) / 3600;
        long m = (secs % 3600) / 60;
        long s = secs % 60;
        char buf[32]; runtime_snprintf(buf, sizeof(buf), "%ldd %02ldh %02ldm %02lds", d, h, m, s);
        out_val->type = VAL_STRING; out_val->as.string = str_create(vm_get_str(vm), buf, runtime_strlen(buf)); return true;
    }
    if (runtime_strcasecmp(name_buf, "HEBREW$") == 0 || runtime_strcasecmp(name_buf, "HEBREW") == 0) {
        *out_val = func_hebrew_eval(vm, name_buf, 0, NULL, NULL); return true;
    }
    if (runtime_strcasecmp(name_buf, "TODAY$") == 0) {
        *out_val = func_ver_eval(vm, "TODAY$", 0, NULL, NULL); return true;
    }
    if (runtime_strcasecmp(name_buf, "TODAY") == 0) {
        *out_val = func_ver_eval(vm, "TODAY", 0, NULL, NULL); return true;
    }

    // ---- Group D: Diagnostics & Interpreter Internals ----
    if (runtime_strcasecmp(name_buf, "VER") == 0) {
        *out_val = func_ver_eval(vm, "VER", 0, NULL, NULL); return true;
    }
    if (runtime_strcasecmp(name_buf, "ERRORLEVEL") == 0) {
        *out_val = func_ver_eval(vm, "ERRORLEVEL", 0, NULL, NULL); return true;
    }
    if (runtime_strcasecmp(name_buf, "TXN") == 0 || runtime_strcasecmp(name_buf, "TXNSTATUS") == 0) {
        *out_val = func_ver_eval(vm, "TXN", 0, NULL, NULL); return true;
    }
    if (runtime_strcasecmp(name_buf, "EXTERR") == 0) {
        *out_val = func_ver_eval(vm, "EXTERR", 0, NULL, NULL); return true;
    }
    if (runtime_strcasecmp(name_buf, "VARPTR$") == 0) {
        *out_val = func_ver_eval(vm, "VARPTR$", 0, NULL, NULL); return true;
    }
    if (runtime_strcasecmp(name_buf, "IOCTL$") == 0) {
        *out_val = func_ver_eval(vm, "IOCTL$", 0, NULL, NULL); return true;
    }
    if (runtime_strcasecmp(name_buf, "ERR") == 0) {
        out_val->type = VAL_INTEGER; out_val->as.number = (double)vm_get_err_code(vm); return true;
    }
    if (runtime_strcasecmp(name_buf, "ERL") == 0) {
        out_val->type = VAL_INTEGER; out_val->as.number = (double)vm_get_err_line(vm); return true;
    }
    if (runtime_strcasecmp(name_buf, "ERDEV") == 0) {
        out_val->type = VAL_INTEGER; out_val->as.number = (double)vm_get_err_code(vm); return true;
    }
    if (runtime_strcasecmp(name_buf, "ERDEV$") == 0 || runtime_strcasecmp(name_buf, "_ERRMSG$") == 0) {
        BppError e = vm_get_error(vm);
        const char *emsg = e.message ? e.message : "";
        out_val->type = VAL_STRING; out_val->as.string = str_create(vm_get_str(vm), emsg, runtime_strlen(emsg)); return true;
    }
    if (runtime_strcasecmp(name_buf, "COMMAND$") == 0 || runtime_strcasecmp(name_buf, "COMMAND") == 0) {
        *out_val = func_command_eval(vm, "COMMAND$", 0, NULL, NULL); return true;
    }

    // ---- Group E: IoT, Wireless & Hardware Sensors ----
    if (runtime_strcasecmp(name_buf, "IOT.STATUS$") == 0) {
        out_val->type = VAL_STRING; out_val->as.string = str_create(vm_get_str(vm), "READY", 5); return true;
    }
    if (runtime_strcasecmp(name_buf, "WIFI.SSID$") == 0) {
        out_val->type = VAL_STRING; out_val->as.string = str_create(vm_get_str(vm), "BASICPP_AP", 10); return true;
    }
    if (runtime_strcasecmp(name_buf, "WIFI.BSSID$") == 0) {
        out_val->type = VAL_STRING; out_val->as.string = str_create(vm_get_str(vm), "00:11:22:33:44:55", 17); return true;
    }
    if (runtime_strcasecmp(name_buf, "WIFI.RSSI") == 0 || runtime_strcasecmp(name_buf, "_WIFI_RSSI%") == 0 || runtime_strcasecmp(name_buf, "_WIFI_RSSI") == 0) {
        out_val->type = VAL_NUMBER; out_val->as.number = -45.0; return true;
    }
    if (runtime_strcasecmp(name_buf, "WIFI.STATUS$") == 0) {
        out_val->type = VAL_STRING; out_val->as.string = str_create(vm_get_str(vm), "CONNECTED", 9); return true;
    }
    if (runtime_strcasecmp(name_buf, "WIFI.CHANNEL") == 0) {
        out_val->type = VAL_INTEGER; out_val->as.number = 6.0; return true;
    }
    if (runtime_strcasecmp(name_buf, "WIFI.IP$") == 0) {
        out_val->type = VAL_STRING; out_val->as.string = str_create(vm_get_str(vm), "192.168.1.100", 13); return true;
    }
    if (runtime_strcasecmp(name_buf, "NET.GATEWAY$") == 0) {
        out_val->type = VAL_STRING; out_val->as.string = str_create(vm_get_str(vm), "192.168.1.1", 11); return true;
    }
    if (runtime_strcasecmp(name_buf, "NET.DNS$") == 0) {
        out_val->type = VAL_STRING; out_val->as.string = str_create(vm_get_str(vm), "8.8.8.8", 7); return true;
    }
    if (runtime_strcasecmp(name_buf, "NET.MASK$") == 0) {
        out_val->type = VAL_STRING; out_val->as.string = str_create(vm_get_str(vm), "255.255.255.0", 13); return true;
    }
    if (runtime_strcasecmp(name_buf, "NET.BROADCAST$") == 0) {
        out_val->type = VAL_STRING; out_val->as.string = str_create(vm_get_str(vm), "192.168.1.255", 13); return true;
    }
    if (runtime_strcasecmp(name_buf, "NET.INTERFACES$") == 0) {
        out_val->type = VAL_STRING; out_val->as.string = str_create(vm_get_str(vm), "eth0,wlan0", 10); return true;
    }
    if (runtime_strcasecmp(name_buf, "BLE.STATUS$") == 0) {
        out_val->type = VAL_STRING; out_val->as.string = str_create(vm_get_str(vm), "IDLE", 4); return true;
    }
    if (runtime_strcasecmp(name_buf, "_BATTERY%") == 0 || runtime_strcasecmp(name_buf, "_BATTERY") == 0) {
        out_val->type = VAL_INTEGER; out_val->as.number = 100.0; return true;
    }
    if (runtime_strcasecmp(name_buf, "_TEMPERATURE%") == 0 || runtime_strcasecmp(name_buf, "_TEMPERATURE") == 0) {
        out_val->type = VAL_INTEGER; out_val->as.number = 25.0; return true;
    }
    if (runtime_strcasecmp(name_buf, "_CPU_LOAD%") == 0 || runtime_strcasecmp(name_buf, "_CPU_LOAD") == 0) {
        out_val->type = VAL_INTEGER; out_val->as.number = 5.0; return true;
    }
    if (runtime_strcasecmp(name_buf, "_FREE_STACK") == 0) {
        out_val->type = VAL_NUMBER; out_val->as.number = 1048576.0; return true;
    }

    // ---- Group F: System & CPU8086 Virtual Registers ----
    if (runtime_strcasecmp(name_buf, "INKEY$") == 0) {
        *out_val = func_inkey_eval(vm, "INKEY$", 0, NULL, NULL); return true;
    }
    if (runtime_strcasecmp(name_buf, "CSRLIN") == 0) {
        *out_val = func_csrlin_eval(vm, "CSRLIN", 0, NULL, NULL); return true;
    }
    if (runtime_strcasecmp(name_buf, "POS") == 0) {
        *out_val = func_pos_eval(vm, "POS", 0, NULL, NULL); return true;
    }
    if (runtime_strcasecmp(name_buf, "STICK") == 0) {
        *out_val = func_stick_eval(vm, "STICK", 0, NULL, NULL); return true;
    }
    if (runtime_strcasecmp(name_buf, "STRIG") == 0) {
        *out_val = func_strig_eval(vm, "STRIG", 0, NULL, NULL); return true;
    }
    if (runtime_strcasecmp(name_buf, "PADDLE") == 0) {
        *out_val = func_paddle_eval(vm, "PADDLE", 0, NULL, NULL); return true;
    }
    if (runtime_strcasecmp(name_buf, "PTRIG") == 0) {
        *out_val = func_ptrig_eval(vm, "PTRIG", 0, NULL, NULL); return true;
    }

    // Hardware, UDX, Buffer, and Calculator Registers
    if (reg_lookup_builtin(name_buf, out_val)) return true;
    if ((runtime_strncasecmp(name_buf, "STACK.", 6) == 0 || runtime_strncasecmp(name_buf, "RPN.", 4) == 0) &&
        rpn_reg_get(name_buf, out_val)) return true;
    if (runtime_strncasecmp(name_buf, "PN.", 3) == 0 &&
        pn_reg_get(name_buf, out_val)) return true;
    // ---- Group G & I: Vintage Dialect Variables ----
    if (runtime_strcasecmp(name_buf, "ST") == 0 || runtime_strcasecmp(name_buf, "CONSOL") == 0 ||
        runtime_strcasecmp(name_buf, "USER") == 0 || runtime_strcasecmp(name_buf, "BDOS") == 0 ||
        runtime_strcasecmp(name_buf, "BIOS") == 0 || runtime_strcasecmp(name_buf, "SWAP$") == 0 ||
        runtime_strcasecmp(name_buf, "JOB") == 0 || runtime_strcasecmp(name_buf, "JOB$") == 0 ||
        runtime_strcasecmp(name_buf, "HIMEM") == 0 || runtime_strcasecmp(name_buf, "LOMEM") == 0 ||
        runtime_strcasecmp(name_buf, "MAXRAM") == 0) {
        *out_val = func_vintage_vars_eval(vm, name_buf, 0, NULL, NULL); return true;
    }
    if (runtime_strcasecmp(name_buf, "EXTNAME$") == 0) { out_val->type = VAL_STRING; out_val->as.string = str_create(vm_get_str(vm), "BAS", 3); return true; }
    if (runtime_strcasecmp(name_buf, "ROMVERSION$") == 0) { out_val->type = VAL_STRING; out_val->as.string = str_create(vm_get_str(vm), "BASIC++ 6.5.2", 13); return true; }
    if (runtime_strcasecmp(name_buf, "SOFTEV") == 0) { out_val->type = VAL_NUMBER; out_val->as.number = 1010.0; return true; }
    if (runtime_strcasecmp(name_buf, "PWRED") == 0) { out_val->type = VAL_NUMBER; out_val->as.number = 165.0; return true; }
    if (runtime_strcasecmp(name_buf, "RESET_VECTOR") == 0) { out_val->type = VAL_NUMBER; out_val->as.number = 64098.0; return true; }
    if (runtime_strcasecmp(name_buf, "DS") == 0) { out_val->type = VAL_INTEGER; out_val->as.number = 0.0; return true; }
    if (runtime_strcasecmp(name_buf, "DS$") == 0) { out_val->type = VAL_STRING; out_val->as.string = str_create(vm_get_str(vm), "00, OK, 00, 00", 14); return true; }
    if (runtime_strcasecmp(name_buf, "STATUS") == 0) { out_val->type = VAL_INTEGER; out_val->as.number = 0.0; return true; }
    if (runtime_strcasecmp(name_buf, "SPEED&") == 0) { out_val->type = VAL_INTEGER; out_val->as.number = baud_get_channel_rate(0); return true; }
    if (runtime_strcasecmp(name_buf, "SPEED%") == 0 || runtime_strcasecmp(name_buf, "SPEED") == 0) { out_val->type = VAL_INTEGER; out_val->as.number = speed_get_apple_speed(); return true; }
    if (runtime_strcasecmp(name_buf, "PAGE") == 0) { out_val->type = VAL_INTEGER; out_val->as.number = 1.0; return true; }
    if (runtime_strcasecmp(name_buf, "VBL") == 0) { out_val->type = VAL_INTEGER; out_val->as.number = 0.0; return true; }
    if (runtime_strcasecmp(name_buf, "PPN$") == 0) { out_val->type = VAL_STRING; out_val->as.string = str_create(vm_get_str(vm), "[1,2]", 5); return true; }
    if (runtime_strcasecmp(name_buf, "LINE") == 0) {
        VariableContext *vc = vm ? vm_get_var(vm) : NULL;
        BValue *uvar = vc ? var_lookup(vc, "LINE", false) : NULL;
        if (uvar && uvar->type != VAL_NONE) { *out_val = *uvar; return true; }
        out_val->type = VAL_INTEGER; out_val->as.number = (double)vm_get_current_line(vm); return true;
    }
    if (runtime_strcasecmp(name_buf, "VCOUNT") == 0) { out_val->type = VAL_INTEGER; out_val->as.number = 0.0; return true; }
    if (runtime_strcasecmp(name_buf, "BAUD") == 0) {
        out_val->type = VAL_NUMBER; out_val->as.number = baud_get_channel_rate(0); return true;
    }

    // ---- Group H: Shell, Paths, Executable & Script Introspection ----
    if (runtime_strcasecmp(name_buf, "EXEPATH$") == 0 || runtime_strcasecmp(name_buf, "EXEPATH") == 0 ||
        runtime_strcasecmp(name_buf, "EXEDIR$") == 0 || runtime_strcasecmp(name_buf, "EXEDIR") == 0 ||
        runtime_strcasecmp(name_buf, "EXENAME$") == 0 || runtime_strcasecmp(name_buf, "EXENAME") == 0 ||
        runtime_strcasecmp(name_buf, "APPDIR$") == 0 || runtime_strcasecmp(name_buf, "APPDIR") == 0 ||
        runtime_strcasecmp(name_buf, "DATADIR$") == 0 || runtime_strcasecmp(name_buf, "DATADIR") == 0 ||
        runtime_strcasecmp(name_buf, "DOCSDIR$") == 0 || runtime_strcasecmp(name_buf, "DOCSDIR") == 0 ||
        runtime_strcasecmp(name_buf, "HELPDIR$") == 0 || runtime_strcasecmp(name_buf, "HELPDIR") == 0) {
        *out_val = func_exepath_eval(vm, name_buf, 0, NULL, NULL); return true;
    }

    if (runtime_strcasecmp(name_buf, "SCRIPTPATH$") == 0 || runtime_strcasecmp(name_buf, "SCRIPTPATH") == 0 ||
        runtime_strcasecmp(name_buf, "SCRIPTDIR$") == 0 || runtime_strcasecmp(name_buf, "SCRIPTDIR") == 0 ||
        runtime_strcasecmp(name_buf, "SCRIPTNAME$") == 0 || runtime_strcasecmp(name_buf, "SCRIPTNAME") == 0 ||
        runtime_strcasecmp(name_buf, "FILENAME$") == 0 || runtime_strcasecmp(name_buf, "FILENAME") == 0 ||
        runtime_strcasecmp(name_buf, "FILEDIR$") == 0 || runtime_strcasecmp(name_buf, "FILEDIR") == 0 ||
        runtime_strcasecmp(name_buf, "DIRPATH$") == 0 || runtime_strcasecmp(name_buf, "DIRPATH") == 0 ||
        runtime_strcasecmp(name_buf, "BASENAME$") == 0 || runtime_strcasecmp(name_buf, "BASENAME") == 0 ||
        runtime_strcasecmp(name_buf, "BASNAME$") == 0 || runtime_strcasecmp(name_buf, "BASNAME") == 0 ||
        runtime_strcasecmp(name_buf, "BASEDIR$") == 0 || runtime_strcasecmp(name_buf, "BASEDIR") == 0 ||
        runtime_strcasecmp(name_buf, "BASEPATH$") == 0 || runtime_strcasecmp(name_buf, "BASEPATH") == 0 ||
        runtime_strcasecmp(name_buf, "PROGNAME$") == 0 || runtime_strcasecmp(name_buf, "PROGNAME") == 0 ||
        runtime_strcasecmp(name_buf, "EXT$") == 0 || runtime_strcasecmp(name_buf, "EXT") == 0 ||
        runtime_strcasecmp(name_buf, "EXTNAME$") == 0 || runtime_strcasecmp(name_buf, "EXTNAME") == 0) {
        *out_val = func_scriptpath_eval(vm, name_buf, 0, NULL, NULL); return true;
    }

    if (runtime_strcasecmp(name_buf, "WORKDIR$") == 0 || runtime_strcasecmp(name_buf, "WORKDIR") == 0 ||
        runtime_strcasecmp(name_buf, "CWD$") == 0 || runtime_strcasecmp(name_buf, "CWD") == 0 ||
        runtime_strcasecmp(name_buf, "PWD$") == 0 || runtime_strcasecmp(name_buf, "PWD") == 0 ||
        runtime_strcasecmp(name_buf, "CURDIR$") == 0 || runtime_strcasecmp(name_buf, "CURDIR") == 0 ||
        runtime_strcasecmp(name_buf, "PREFIX$") == 0 || runtime_strcasecmp(name_buf, "PREFIX") == 0 ||
        runtime_strcasecmp(name_buf, "DRIVE$") == 0 || runtime_strcasecmp(name_buf, "DRIVE") == 0) {
        *out_val = func_workdir_eval(vm, name_buf, 0, NULL, NULL); return true;
    }

    if (runtime_strcasecmp(name_buf, "TEMPDIR$") == 0 || runtime_strcasecmp(name_buf, "TEMPDIR") == 0 ||
        runtime_strcasecmp(name_buf, "TMPDIR$") == 0 || runtime_strcasecmp(name_buf, "TMPDIR") == 0 ||
        runtime_strcasecmp(name_buf, "TEMP$") == 0 || runtime_strcasecmp(name_buf, "TEMP") == 0 ||
        runtime_strcasecmp(name_buf, "TMP$") == 0 || runtime_strcasecmp(name_buf, "TMP") == 0) {
        *out_val = func_tempdir_eval(vm, name_buf, 0, NULL, NULL); return true;
    }

    if (runtime_strcasecmp(name_buf, "HOMEDRIVE$") == 0 || runtime_strcasecmp(name_buf, "HOMEDRIVE") == 0 ||
        runtime_strcasecmp(name_buf, "HOMEPATH$") == 0 || runtime_strcasecmp(name_buf, "HOMEPATH") == 0 ||
        runtime_strcasecmp(name_buf, "USERPATH$") == 0 || runtime_strcasecmp(name_buf, "USERPATH") == 0 ||
        runtime_strcasecmp(name_buf, "HOME$") == 0 || runtime_strcasecmp(name_buf, "HOME") == 0) {
        *out_val = func_userpath_eval(vm, name_buf, 0, NULL, NULL); return true;
    }

    if (runtime_strcasecmp(name_buf, "PROGRAM$") == 0 || runtime_strcasecmp(name_buf, "PROGRAM") == 0) {
        *out_val = func_program_eval(vm, name_buf, 0, NULL, NULL); return true;
    }

    if (runtime_strcasecmp(name_buf, "NSTATUS") == 0 || runtime_strcasecmp(name_buf, "NHTTPSTATUS") == 0 ||
        runtime_strcasecmp(name_buf, "NEOF") == 0 || runtime_strcasecmp(name_buf, "NBYTESWAITING") == 0 ||
        runtime_strcasecmp(name_buf, "NCONNECTED") == 0 || runtime_strcasecmp(name_buf, "NERROR") == 0 ||
        runtime_strcasecmp(name_buf, "SIOSTATUS") == 0 || runtime_strcasecmp(name_buf, "SIOAVAIL") == 0 ||
        runtime_strcasecmp(name_buf, "BIOSTATUS") == 0 || runtime_strcasecmp(name_buf, "BIOSIZE") == 0 ||
        runtime_strcasecmp(name_buf, "BIOCHECKSUM") == 0) {
        *out_val = func_net_telemetry_eval(vm, name_buf, 0, NULL, NULL); return true;
    }

    if (runtime_strcasecmp(name_buf, "COMSPEC") == 0 || runtime_strcasecmp(name_buf, "COMSPEC$") == 0) {
#if defined(_WIN32)
        const char *cs = platform_getenv("COMSPEC");
        if (!cs) cs = "C:\\Windows\\System32\\cmd.exe";
#else
        const char *cs = platform_getenv("SHELL");
        if (!cs) cs = "/bin/sh";
#endif
        out_val->type = VAL_STRING; out_val->as.string = str_create(vm_get_str(vm), cs, runtime_strlen(cs)); return true;
    }
    if (runtime_strcasecmp(name_buf, "PATH") == 0 || runtime_strcasecmp(name_buf, "PATH$") == 0) {
        const char *p = platform_getenv("PATH");
        char pbuf[1024] = {0};
        if (!p) {
            platform_getcwd(pbuf, sizeof(pbuf) - 1);
            p = pbuf;
        }
        out_val->type = VAL_STRING; out_val->as.string = str_create(vm_get_str(vm), p, runtime_strlen(p)); return true;
    }
    if (runtime_strcasecmp(name_buf, "LOGNAME") == 0 || runtime_strcasecmp(name_buf, "LOGNAME$") == 0) {
        char ubuf[128] = {0};
#if defined(_WIN32)
        const char *u = platform_getenv("USERNAME");
#else
        const char *u = platform_getenv("LOGNAME");
        if (!u) u = platform_getenv("USER");
#endif
        if (!u) {
            platform_get_username(ubuf, sizeof(ubuf));
            u = ubuf;
        }
        out_val->type = VAL_STRING; out_val->as.string = str_create(vm_get_str(vm), u, runtime_strlen(u)); return true;
    }
    if (runtime_strcasecmp(name_buf, "COMPUTERNAME") == 0 || runtime_strcasecmp(name_buf, "COMPUTERNAME$") == 0) {
        char hbuf[128] = {0};
#if defined(_WIN32)
        const char *cn = platform_getenv("COMPUTERNAME");
#else
        const char *cn = platform_getenv("HOSTNAME");
#endif
        if (!cn) {
            platform_get_hostname(hbuf, sizeof(hbuf));
            cn = hbuf;
        }
        out_val->type = VAL_STRING; out_val->as.string = str_create(vm_get_str(vm), cn, runtime_strlen(cn)); return true;
    }
    if (runtime_strcasecmp(name_buf, "MODDIR$") == 0) {
        const char *mods = "OS9P1, OS9P2, BASIC09, INP, OUTP, SYSGO, PIPEMAN";
        out_val->type = VAL_STRING; out_val->as.string = str_create(vm_get_str(vm), mods, runtime_strlen(mods)); return true;
    }
    if (runtime_strcasecmp(name_buf, "SHELL$") == 0) {
#if defined(_WIN32)
        const char *sh = "cmd.exe";
#else
        const char *sh = "/bin/sh";
#endif
        out_val->type = VAL_STRING; out_val->as.string = str_create(vm_get_str(vm), sh, runtime_strlen(sh)); return true;
    }
    if (runtime_strcasecmp(name_buf, "TERM$") == 0) {
        out_val->type = VAL_STRING; out_val->as.string = str_create(vm_get_str(vm), "xterm-256color", 14); return true;
    }
    if (runtime_strcasecmp(name_buf, "LANG$") == 0) {
        out_val->type = VAL_STRING; out_val->as.string = str_create(vm_get_str(vm), "en_US.UTF-8", 11); return true;
    }
    if (runtime_strcasecmp(name_buf, "PROMPT$") == 0) {
        out_val->type = VAL_STRING; out_val->as.string = str_create(vm_get_str(vm), "> ", 2); return true;
    }
    if (runtime_strcasecmp(name_buf, "MEM") == 0) {
        out_val->type = VAL_NUMBER; out_val->as.number = 671088640.0; return true;
    }

    // ---- Group L: STARDATE & STARDATE$ ----
    if (runtime_strcasecmp(name_buf, "STARDATE") == 0 || runtime_strcasecmp(name_buf, "STARTDATE") == 0) {
        int64_t diff = (int64_t)raw_t - 252460800LL;
        if (diff < 0) diff = 0;
        long whole_days = (long)(diff / 86400LL);
        int tenth = (int)((diff % 86400LL) / 8640LL);
        if (tenth > 9) tenth = 9;
        out_val->type = VAL_NUMBER; out_val->as.number = (double)whole_days + ((double)tenth / 10.0); return true;
    }
    if (runtime_strcasecmp(name_buf, "STARDATE$") == 0 || runtime_strcasecmp(name_buf, "STARTDATE$") == 0) {
        int64_t diff = (int64_t)raw_t - 126230400LL;
        if (diff < 0) diff = 0;
        long whole_days = (long)(diff / 86400LL);
        int tenth = (int)((diff % 86400LL) / 8640LL);
        if (tenth > 9) tenth = 9;
        char buf[32]; runtime_snprintf(buf, sizeof(buf), "%ld.%d", whole_days, tenth);
        out_val->type = VAL_STRING; out_val->as.string = str_create(vm_get_str(vm), buf, runtime_strlen(buf)); return true;
    }

    return false;
}
