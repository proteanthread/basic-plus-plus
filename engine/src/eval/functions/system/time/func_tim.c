// FILENAME: func_tim.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (hal.h, memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (string.c, func_tim.h)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the HP 2000/3000 TIM built-in function in BASIC++.

#include "eval/functions/system/time/func_tim.h"
#include "runtime/language_descriptor.h"
#include "platform/platform.h"
#include "runtime/strings.h"
#include "runtime/string.h"
#include "runtime/memory.h"
#include "hal/hal.h"

static const LangDesc g_tim_desc = {
    .name = "TIM",
    .category = "System Functions",
    .syntax = "TIM(n)",
    .description = "Returns HP 2000/3000 time components: 0=sec, 1=min, 2=hour, 3=day, 4=month, 5=year, 6=day-of-week (1-7), 7=day-of-year (1-366), 8=millisecond (0-999).",
    .error_summary = "Error 5: Illegal Function Call, Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_FUNCTION
};

void func_tim_register(void) {
    lang_desc_register(&g_tim_desc);
}

BValue func_tim_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    BValue res;
    res.type = VAL_NUMBER;
    res.as.number = 0.0;
    (void)vm; (void)uname;

    int code = 0;
    if (arg_count >= 1) {
        if (args[0].type == VAL_STRING) {
            err->code = ERR_TYPE_MISMATCH;
            err->message = "Type mismatch in TIM(n)";
            return res;
        }
        code = (int)args[0].as.number;
    }

    time_t rawtime = (time_t)(hal_get() && hal_get()->time.now_epoch_seconds ? hal_get()->time.now_epoch_seconds() : 0);
    if (rawtime == 0) {
        rawtime = time(NULL);
    }
    struct tm tm_buf;
    struct tm *info = platform_localtime(&rawtime, &tm_buf);

    if (!info) {
        return res;
    }

    switch (code) {
        case 0:
            res.as.number = (double)info->tm_sec;
            break;
        case 1:
            res.as.number = (double)info->tm_min;
            break;
        case 2:
            res.as.number = (double)info->tm_hour;
            break;
        case 3:
            res.as.number = (double)info->tm_mday;
            break;
        case 4:
            res.as.number = (double)(info->tm_mon + 1);
            break;
        case 5:
            res.as.number = (double)(info->tm_year + 1900);
            break;
        case 6:
            res.as.number = (double)(info->tm_wday + 1); // 1 = Sunday .. 7 = Saturday
            break;
        case 7:
            res.as.number = (double)(info->tm_yday + 1); // 1 .. 366
            break;
        case 8: {
            uint64_t ms = (uint64_t)(platform_get_uptime() * 1000.0);
            res.as.number = (double)(ms % 1000);
            break;
        }
        default:
            err->code = ERR_ILLEGAL_FUNCTION_CALL;
            err->message = "Illegal function call: TIM(n) index must be 0-8";
            break;
    }

    return res;
}
