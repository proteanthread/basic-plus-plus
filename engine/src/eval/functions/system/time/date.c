// FILENAME: date.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (conversion_fn.c)
// NEEDS: libcore (hal.h, memory.h, memory.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (date.h, string.c)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the DATE built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/system/time/date.h"
#include "platform/platform.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/strings.h"
#include "runtime/string.h"
#include "runtime/memory.h"
#include "hal/hal.h"
void func_date_register(void) {
    MicroLibMetadata meta = {
        .name = "DATE$",
        .category = "System Functions",
        .syntax = "DATE$ | DATE$(day_num)",
        .help_text = "Returns the current system date string, or formats a numeric day number into DD-Mon-YY (DEC BASIC-PLUS).",
        .error_codes = "Error 13: Type Mismatch"
    };
    microlib_register(&meta);
}

BValue func_date_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (arg_count > 1) {
        err->code = 13;
        err->message = "DATE / DATE$ expects at most 1 argument";
        return res;
    }

    if (arg_count == 1) {
        if (args[0].type == VAL_STRING) {
            err->code = 13;
            err->message = "Type mismatch in DATE$: expected numeric argument";
            return res;
        }

        int day_val = (int)args[0].as.number;
        time_t t_now = (time_t)(hal_get() && hal_get()->time.now_epoch_seconds ? hal_get()->time.now_epoch_seconds() : 0);
        if (day_val > 0 && day_val < 366) {
            struct tm tm_buf;
            struct tm *lt = platform_localtime(&t_now, &tm_buf);
            if (lt) {
                lt->tm_mday = day_val;
                lt->tm_mon = 0;
                mktime(lt);
                char buf[32] = "";
                strftime(buf, sizeof(buf), "%d-%b-%y", lt);
                res.type = VAL_STRING;
                res.as.string = str_create(vm_get_str(vm), buf, runtime_strlen(buf));
                return res;
            }
        }

        char buf[32] = "";
        runtime_snprintf(buf, sizeof(buf), "%02d-Jan-26", (day_val > 0) ? (day_val % 31) + 1 : 1);
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), buf, runtime_strlen(buf));
        return res;
    }

    time_t t = (time_t)(hal_get() && hal_get()->time.now_epoch_seconds ? hal_get()->time.now_epoch_seconds() : 0);
    struct tm tm_buf;
    struct tm *lt = platform_localtime(&t, &tm_buf);

    if (runtime_strcmp(uname, "DAY$") == 0) {
        char buf[32] = "";
        if (lt) {
            strftime(buf, sizeof(buf), "%a", lt);
            for (char *p = buf; *p; p++) *p = (char)runtime_toupper((unsigned char)*p);
        }
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), buf, runtime_strlen(buf));
    } else if (runtime_strcmp(uname, "MONTH$") == 0) {
        char buf[32] = "";
        if (lt) {
            strftime(buf, sizeof(buf), "%b", lt);
            for (char *p = buf; *p; p++) *p = (char)runtime_toupper((unsigned char)*p);
        }
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), buf, runtime_strlen(buf));
    } else if (runtime_strcmp(uname, "DAY") == 0) {
        res.type = VAL_NUMBER;
        res.as.number = lt ? (double)lt->tm_mday : 1.0;
    } else if (runtime_strcmp(uname, "MONTH") == 0) {
        res.type = VAL_NUMBER;
        res.as.number = lt ? (double)(lt->tm_mon + 1) : 1.0;
    } else if (runtime_strcmp(uname, "YEAR") == 0) {
        res.type = VAL_NUMBER;
        res.as.number = lt ? (double)(lt->tm_year + 1900) : 2026.0;
    } else if (runtime_strcmp(uname, "DATE$") == 0) {
        char buf[32] = "";
        if (lt) {
            strftime(buf, sizeof(buf), "%m-%d-%Y", lt);
        }
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), buf, runtime_strlen(buf));
    } else {
        double val = 0.0;
        if (lt) {
            val = (lt->tm_year + 1900) * 10000.0 + (lt->tm_mon + 1) * 100.0 + lt->tm_mday;
        }
        res.type = VAL_NUMBER;
        res.as.number = val;
    }

    return res;
}
