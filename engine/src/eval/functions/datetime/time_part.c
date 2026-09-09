// FILENAME: time_part.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (hal.h, language_descriptor.h, memory.h, string.h)
// NEEDS: libengine (string.c, time_part.h)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for TIME_PART / TIMEPART built-in functions in BASIC++.

#include "eval/functions/datetime/time_part.h"
#include "runtime/language_descriptor.h"
#include "platform/platform.h"
#include "runtime/strings.h"
#include "runtime/string.h"
#include "runtime/memory.h"
#include "hal/hal.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"

static const LangDesc g_time_part_desc = {
    .name = "TIME_PART",
    .category = "Date & Time",
    .syntax = "TIME_PART(interval$ [, date_serial#]) / TIMEPART(...)",
    .description = "Extracts date or time component (\"yyyy\", \"m\", \"d\", \"h\", \"n\", \"s\", \"w\", \"q\", \"yday\", \"ms\") from date_serial or current time.",
    .error_summary = "Error 5: Illegal Function Call, Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_FUNCTION
};

void func_time_part_register(void) {
    lang_desc_register(&g_time_part_desc);
}

BValue func_time_part_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    BValue res;
    res.type = VAL_NUMBER;
    res.as.number = 0.0;
    (void)vm; (void)uname;

    if (arg_count < 1) {
        err->code = 5;
        err->message = "Expected at least 1 argument in TIME_PART(interval$ [, date_serial#])";
        return res;
    }

    time_t rawtime;
    if (arg_count >= 2 && args[1].type == VAL_NUMBER) {
        double serial = args[1].as.number;
        if (serial > 100000000.0) {
            rawtime = (time_t)serial;
        } else {
            // Serial day offset from 1899-12-30 (VB / COM standard)
            rawtime = (time_t)((serial - 25569.0) * 86400.0);
        }
    } else {
        rawtime = (time_t)(hal_get() && hal_get()->time.now_epoch_seconds ? hal_get()->time.now_epoch_seconds() : (uint64_t)platform_get_timer());
    }

    struct tm tm_buf;
    struct tm *info = platform_localtime(&rawtime, &tm_buf);
    if (!info) {
        return res;
    }

    if (args[0].type == VAL_STRING && args[0].as.string) {
        const char *part = str_data(args[0].as.string);
        if (runtime_strcasecmp(part, "yyyy") == 0 || runtime_strcasecmp(part, "year") == 0 || runtime_strcasecmp(part, "y") == 0) {
            res.as.number = (double)(info->tm_year + 1900);
        } else if (runtime_strcasecmp(part, "m") == 0 || runtime_strcasecmp(part, "month") == 0) {
            res.as.number = (double)(info->tm_mon + 1);
        } else if (runtime_strcasecmp(part, "d") == 0 || runtime_strcasecmp(part, "day") == 0) {
            res.as.number = (double)info->tm_mday;
        } else if (runtime_strcasecmp(part, "h") == 0 || runtime_strcasecmp(part, "hour") == 0) {
            res.as.number = (double)info->tm_hour;
        } else if (runtime_strcasecmp(part, "n") == 0 || runtime_strcasecmp(part, "min") == 0 || runtime_strcasecmp(part, "minute") == 0) {
            res.as.number = (double)info->tm_min;
        } else if (runtime_strcasecmp(part, "s") == 0 || runtime_strcasecmp(part, "sec") == 0 || runtime_strcasecmp(part, "second") == 0) {
            res.as.number = (double)info->tm_sec;
        } else if (runtime_strcasecmp(part, "w") == 0 || runtime_strcasecmp(part, "weekday") == 0) {
            res.as.number = (double)(info->tm_wday + 1); // 1 = Sunday
        } else if (runtime_strcasecmp(part, "q") == 0 || runtime_strcasecmp(part, "quarter") == 0) {
            res.as.number = (double)((info->tm_mon / 3) + 1);
        } else if (runtime_strcasecmp(part, "yday") == 0 || runtime_strcasecmp(part, "dayofyear") == 0) {
            res.as.number = (double)(info->tm_yday + 1);
        } else if (runtime_strcasecmp(part, "ms") == 0 || runtime_strcasecmp(part, "millisecond") == 0) {
            uint64_t ms = (uint64_t)(platform_get_uptime() * 1000.0);
            res.as.number = (double)(ms % 1000);
        } else {
            err->code = 5;
            err->message = "Unknown interval format in TIME_PART (use yyyy, m, d, h, n, s, w, q, yday, ms)";
        }
    } else if (args[0].type == VAL_NUMBER) {
        int code = (int)args[0].as.number;
        switch (code) {
            case 1: res.as.number = (double)(info->tm_year + 1900); break;
            case 2: res.as.number = (double)(info->tm_mon + 1); break;
            case 3: res.as.number = (double)info->tm_mday; break;
            case 4: res.as.number = (double)info->tm_hour; break;
            case 5: res.as.number = (double)info->tm_min; break;
            case 6: res.as.number = (double)info->tm_sec; break;
            case 7: res.as.number = (double)(info->tm_wday + 1); break;
            case 8: res.as.number = (double)((info->tm_mon / 3) + 1); break;
            case 9: res.as.number = (double)(info->tm_yday + 1); break;
            case 10: {
                uint64_t ms = (uint64_t)(platform_get_uptime() * 1000.0);
                res.as.number = (double)(ms % 1000);
                break;
            }
            default:
                err->code = 5;
                err->message = "Illegal numeric code in TIME_PART (must be 1-10)";
                break;
        }
    } else {
        err->code = 13;
        err->message = "Type mismatch in TIME_PART";
    }

    return res;
}
