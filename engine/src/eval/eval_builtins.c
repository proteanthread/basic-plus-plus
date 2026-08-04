/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file eval_builtins.c
 * @brief Extracted Builtin Functions for Evaluator.
 */

#include "eval/eval_internal.h"
#include "runtime/file.h"
#include "runtime/task.h"
#include "device/vdev.h"
#include "runtime/vfs.h"
#include "runtime/metadata.h"
#include "runtime/funcreg.h"
#include "module/module.h"
#include "security/security.h"
#include "eval/eval.h"
#include "runtime/num_format.h"
#include "runtime/vnet.h"
#include "device/bus.h"
#include "core/dialect.h"
#include "core/struct.h"
#include "device/fujinet.h"
#include "platform/platform.h"
#include "device/vcon.h"
#include "runtime/variables.h"
#include "runtime/map.h"


#ifndef BASIC_LITE_BUILD
#include "memory/segmented_mem.h"
#endif

BppDirSearch *g_eval_dir_search = NULL;
uint16_t eval_compute_crc16(const unsigned char *data, size_t len) {
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < len; ++i) {
        crc ^= (uint16_t)(data[i] << 8);
        for (int b = 0; b < 8; ++b) {
            if (crc & 0x8000) {
                crc = (uint16_t)((crc << 1) ^ 0x1021);
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

BValue eval_builtin_function_impl(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (strcmp(uname, "SQR") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "SQR expects one numeric argument"; return res;
        }
        if (args[0].as.number < 0.0) {
            err->code = 5; err->message = "SQR of negative number"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = sqrt(args[0].as.number);
    }
    else if (strcmp(uname, "_SHL") == 0 || strcmp(uname, "BITS.SHL") == 0) {
        if (arg_count != 2 || args[0].type == VAL_STRING || args[1].type == VAL_STRING) {
            err->code = 13; err->message = "SHL expects two numeric arguments"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = (double)((uint64_t)(int64_t)args[0].as.number << (uint64_t)(int64_t)args[1].as.number);
    }
    else if (strcmp(uname, "_SHR") == 0 || strcmp(uname, "BITS.SHR") == 0) {
        if (arg_count != 2 || args[0].type == VAL_STRING || args[1].type == VAL_STRING) {
            err->code = 13; err->message = "SHR expects two numeric arguments"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = (double)((uint64_t)(int64_t)args[0].as.number >> (uint64_t)(int64_t)args[1].as.number);
    }
    else if (strcmp(uname, "_READBIT") == 0 || strcmp(uname, "BITS.READ") == 0) {
        if (arg_count != 2 || args[0].type == VAL_STRING || args[1].type == VAL_STRING) {
            err->code = 13; err->message = "READBIT expects two numeric arguments"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = (double)(((uint64_t)(int64_t)args[0].as.number >> (uint64_t)(int64_t)args[1].as.number) & 1);
    }
    else if (strcmp(uname, "_SETBIT") == 0 || strcmp(uname, "BITS.SET") == 0) {
        if (arg_count != 2 || args[0].type == VAL_STRING || args[1].type == VAL_STRING) {
            err->code = 13; err->message = "SETBIT expects two numeric arguments"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = (double)((uint64_t)(int64_t)args[0].as.number | ((uint64_t)1 << (uint64_t)(int64_t)args[1].as.number));
    }
    else if (strcmp(uname, "_RESETBIT") == 0 || strcmp(uname, "BITS.RESET") == 0) {
        if (arg_count != 2 || args[0].type == VAL_STRING || args[1].type == VAL_STRING) {
            err->code = 13; err->message = "RESETBIT expects two numeric arguments"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = (double)((uint64_t)(int64_t)args[0].as.number & ~((uint64_t)1 << (uint64_t)(int64_t)args[1].as.number));
    }
    else if (strcmp(uname, "_TOGGLEBIT") == 0 || strcmp(uname, "BITS.TOGGLE") == 0) {
        if (arg_count != 2 || args[0].type == VAL_STRING || args[1].type == VAL_STRING) {
            err->code = 13; err->message = "TOGGLEBIT expects two numeric arguments"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = (double)((uint64_t)(int64_t)args[0].as.number ^ ((uint64_t)1 << (uint64_t)(int64_t)args[1].as.number));
    }
    else if (strcmp(uname, "_BITCOUNT") == 0 || strcmp(uname, "BITS.COUNT") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "BITCOUNT expects one numeric argument"; return res;
        }
        uint64_t temp = (uint64_t)(int64_t)args[0].as.number;
        int count = 0;
        while (temp) {
            if (temp & 1) count++;
            temp >>= 1;
        }
        res.type = VAL_NUMBER;
        res.as.number = (double)count;
    }
    else if (strcmp(uname, "_ACOS") == 0 || strcmp(uname, "MATH.ACOS") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "ACOS expects one numeric argument"; return res;
        }
        if (args[0].as.number < -1.0 || args[0].as.number > 1.0) {
            err->code = 5; err->message = "ACOS domain error"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = acos(args[0].as.number);
    }
    else if (strcmp(uname, "_ASIN") == 0 || strcmp(uname, "MATH.ASIN") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "ASIN expects one numeric argument"; return res;
        }
        if (args[0].as.number < -1.0 || args[0].as.number > 1.0) {
            err->code = 5; err->message = "ASIN domain error"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = asin(args[0].as.number);
    }
    else if (strcmp(uname, "_ATAN2") == 0 || strcmp(uname, "MATH.ATAN2") == 0) {
        if (arg_count != 2 || args[0].type == VAL_STRING || args[1].type == VAL_STRING) {
            err->code = 13; err->message = "ATAN2 expects two numeric arguments"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = atan2(args[0].as.number, args[1].as.number);
    }
    else if (strcmp(uname, "_ACOSH") == 0 || strcmp(uname, "MATH.ACOSH") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "ACOSH expects one numeric argument"; return res;
        }
        if (args[0].as.number < 1.0) {
            err->code = 5; err->message = "ACOSH domain error (x must be >= 1.0)"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = acosh(args[0].as.number);
    }
    else if (strcmp(uname, "_ASINH") == 0 || strcmp(uname, "MATH.ASINH") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "ASINH expects one numeric argument"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = asinh(args[0].as.number);
    }
    else if (strcmp(uname, "_ATANH") == 0 || strcmp(uname, "MATH.ATANH") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "ATANH expects one numeric argument"; return res;
        }
        if (args[0].as.number <= -1.0 || args[0].as.number >= 1.0) {
            err->code = 5; err->message = "ATANH domain error (x must be between -1.0 and 1.0)"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = atanh(args[0].as.number);
    }
    else if (strcmp(uname, "_CEIL") == 0 || strcmp(uname, "MATH.CEIL") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "CEIL expects one numeric argument"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = ceil(args[0].as.number);
    }
    else if (strcmp(uname, "_HYPOT") == 0 || strcmp(uname, "MATH.HYPOT") == 0) {
        if (arg_count != 2 || args[0].type == VAL_STRING || args[1].type == VAL_STRING) {
            err->code = 13; err->message = "HYPOT expects two numeric arguments"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = hypot(args[0].as.number, args[1].as.number);
    }
    else if (strcmp(uname, "_PI") == 0 || strcmp(uname, "MATH.PI") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "PI expects no arguments"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = 3.14159265358979323846;
    }
    else if (strcmp(uname, "_D2R") == 0 || strcmp(uname, "MATH.D2R") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "D2R expects one numeric argument"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = args[0].as.number * (3.14159265358979323846 / 180.0);
    }
    else if (strcmp(uname, "_R2D") == 0 || strcmp(uname, "MATH.R2D") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "R2D expects one numeric argument"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = args[0].as.number * (180.0 / 3.14159265358979323846);
    }
    else if (strcmp(uname, "_D2G") == 0 || strcmp(uname, "MATH.D2G") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "D2G expects one numeric argument"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = args[0].as.number * (10.0 / 9.0);
    }
    else if (strcmp(uname, "_G2D") == 0 || strcmp(uname, "MATH.G2D") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "G2D expects one numeric argument"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = args[0].as.number * (9.0 / 10.0);
    }
    else if (strcmp(uname, "INKEY$") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "INKEY$ expects no arguments"; return res;
        }
        int ch = platform_inkey_char();
        char buf[2] = {0};
        if (ch > 0) {
            buf[0] = (char)ch;
        }
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), buf, buf[0] ? 1 : 0);
    }
    else if (strcmp(uname, "PEN") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "PEN expects one numeric argument"; return res;
        }
        int n = (int)args[0].as.number;
        int val = 0;
        if (n == 3) val = platform_mouse_btn();
        else if (n == 4) val = platform_mouse_x();
        else if (n == 5) val = platform_mouse_y();
        res.type = VAL_NUMBER;
        res.as.number = val;
    }
    else if (strcmp(uname, "TIME$") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "TIME$ expects no arguments"; return res;
        }
        time_t t = time(NULL);
        struct tm tm_buf;
        struct tm *lt = platform_localtime(&t, &tm_buf);
        char buf[64] = "";
        if (lt) {
            int hour12 = lt->tm_hour % 12;
            if (hour12 == 0) hour12 = 12;
            char ap = (lt->tm_hour >= 12) ? 'P' : 'A';
            snprintf(buf, sizeof(buf), "%02d:%02d:%02d %c", hour12, lt->tm_min, lt->tm_sec, ap);
        }
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), buf, strlen(buf));
    }
    else if (strcmp(uname, "TI$") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "TI$ expects no arguments"; return res;
        }
        double adjusted = platform_get_uptime() + vm_get_ti_offset(vm);
        if (adjusted < 0.0) adjusted = 0.0;
        long long sec_total = (long long)adjusted;
        long long days = sec_total / 86400;
        long long remaining = sec_total % 86400;
        int hr = (int)(remaining / 3600);
        int min = (int)((remaining / 60) % 60);
        int sec = (int)(remaining % 60);
        char buf[64];
        if (days == 0) {
            snprintf(buf, sizeof(buf), "%02d%02d%02d", hr, min, sec);
        } else {
            snprintf(buf, sizeof(buf), "%03lld:%02d%02d%02d", days, hr, min, sec);
        }
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), buf, strlen(buf));
    }
    else if (strcmp(uname, "CLOCK$") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "CLOCK$ expects no arguments"; return res;
        }
        time_t t = time(NULL);
        struct tm tm_buf;
        extern struct tm *platform_gmtime(const time_t *timep, struct tm *result);
        struct tm *gt = platform_gmtime(&t, &tm_buf);
        char buf[64] = "";
        if (gt) {
            snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d",
                     gt->tm_year + 1900, gt->tm_mon + 1, gt->tm_mday,
                     gt->tm_hour, gt->tm_min, gt->tm_sec);
        }
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), buf, strlen(buf));
    }
    else if (strcmp(uname, "CLOCK") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "CLOCK expects no arguments"; return res;
        }
        time_t t = time(NULL);
        struct tm tm_buf;
        struct tm *lt = platform_localtime(&t, &tm_buf);
        double val = 0.0;
        if (lt) {
            long long yr = (long long)(lt->tm_year + 1900);
            long long mon = (long long)(lt->tm_mon + 1);
            long long mday = (long long)lt->tm_mday;
            long long hr = (long long)lt->tm_hour;
            long long min = (long long)lt->tm_min;
            long long sec = (long long)lt->tm_sec;
            val = (double)(yr * 10000000000LL + mon * 100000000LL + mday * 1000000LL + hr * 10000LL + min * 100LL + sec);
        }
        res.type = VAL_NUMBER;
        res.as.number = val;
    }
    else if (strcmp(uname, "PI") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "PI expects no arguments"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = 3.141592653589793;
    }
    else if (strcmp(uname, "CSRLIN") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "CSRLIN expects no arguments"; return res;
        }
        extern int g_cursor_y;
        res.type = VAL_NUMBER;
        res.as.number = (double)(g_cursor_y + 1);
    }
    else if (strcmp(uname, "POS") == 0) {
        if (arg_count != 1) {
            err->code = 13; err->message = "POS expects 1 argument"; return res;
        }
        extern int g_cursor_x;
        res.type = VAL_NUMBER;
        res.as.number = (double)(g_cursor_x + 1);
    }
    else if (strcmp(uname, "LPOS") == 0) {
        if (arg_count != 1) {
            err->code = 13; err->message = "LPOS expects 1 argument"; return res;
        }
        extern int g_lpos;
        res.type = VAL_NUMBER;
        res.as.number = (double)g_lpos;
    }
    else if (strcmp(uname, "TZ$") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "TZ$ expects no arguments"; return res;
        }
        time_t t = time(NULL);
        struct tm tm_buf;
        struct tm *lt = platform_localtime(&t, &tm_buf);
        char buf[64] = "UTC";
        if (lt) {
            char tz_buf[64] = "";
            strftime(tz_buf, sizeof(tz_buf), "%Z", lt);
            if (tz_buf[0] != '\0') {
                size_t len = strlen(tz_buf);
                bool is_abbr = (len <= 5);
                if (is_abbr) {
                    for (size_t i = 0; i < len; i++) {
                        if (tz_buf[i] >= 'a' && tz_buf[i] <= 'z') {
                            is_abbr = false;
                            break;
                        }
                    }
                }
                if (is_abbr) {
                    strncpy(buf, tz_buf, sizeof(buf) - 1);
                    buf[sizeof(buf) - 1] = '\0';
                } else {
                    int w_idx = 0;
                    for (size_t i = 0; i < len; i++) {
                        if (tz_buf[i] >= 'A' && tz_buf[i] <= 'Z') {
                            if (w_idx < (int)sizeof(buf) - 1) {
                                buf[w_idx++] = tz_buf[i];
                            }
                        }
                    }
                    buf[w_idx] = '\0';
                    if (w_idx == 0) {
                        strcpy(buf, "UTC");
                    }
                }
            }
        }
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), buf, strlen(buf));
    }
    else if (strcmp(uname, "TIMEZONE$") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "TIMEZONE$ expects no arguments"; return res;
        }
        time_t t = time(NULL);
        struct tm tm_buf;
        struct tm *lt = platform_localtime(&t, &tm_buf);
        char buf[64] = "UTC";
        if (lt) {
            char tz_buf[64] = "";
            strftime(tz_buf, sizeof(tz_buf), "%Z", lt);
            if (tz_buf[0] != '\0') {
                strncpy(buf, tz_buf, sizeof(buf) - 1);
                buf[sizeof(buf) - 1] = '\0';
            }
        }
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), buf, strlen(buf));
    }
    else if (strcmp(uname, "TZ") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "TZ expects no arguments"; return res;
        }
        time_t t = time(NULL);
        struct tm tm_buf;
        struct tm *lt = platform_localtime(&t, &tm_buf);
        double val = 0.0;
        if (lt) {
            char buf[16] = "";
            strftime(buf, sizeof(buf), "%z", lt);
            val = atof(buf);
        }
        res.type = VAL_NUMBER;
        res.as.number = val;
    }
    else if (strcmp(uname, "UTC") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "UTC expects no arguments"; return res;
        }
        time_t t = time(NULL);
        struct tm tm_buf;
        struct tm *lt = platform_localtime(&t, &tm_buf);
        double val = 0.0;
        if (lt) {
            char buf[16] = "";
            strftime(buf, sizeof(buf), "%z", lt);
            if (strlen(buf) >= 5) {
                int sign = (buf[0] == '-') ? -1 : 1;
                int hours = (buf[1] - '0') * 10 + (buf[2] - '0');
                int mins = (buf[3] - '0') * 10 + (buf[4] - '0');
                val = sign * (hours * 3600.0 + mins * 60.0);
            }
        }
        res.type = VAL_NUMBER;
        res.as.number = val;
    }
    else if (strcmp(uname, "DATE$") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "DATE$ expects no arguments"; return res;
        }
        time_t t = time(NULL);
        struct tm tm_buf;
        struct tm *lt = platform_localtime(&t, &tm_buf);
        char buf[32] = "";
        if (lt) {
            strftime(buf, sizeof(buf), "%m-%d-%Y", lt);
        }
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), buf, strlen(buf));
    }
    else if (strcmp(uname, "TRUE") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "TRUE expects no arguments"; return res;
        }
        res.type = VAL_NUMBER; res.as.number = 1.0;
    }
    else if (strcmp(uname, "FALSE") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "FALSE expects no arguments"; return res;
        }
        res.type = VAL_NUMBER; res.as.number = -1.0;
    }
    else if (strcmp(uname, "TI") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "TI expects no arguments"; return res;
        }
        double val = platform_get_uptime() + vm_get_ti_offset(vm);
        val = fmod(val, 999999999.0);
        res.type = VAL_NUMBER; res.as.number = val;
    }
    else if (strcmp(uname, "TIME") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "TIME expects no arguments"; return res;
        }
        time_t t = time(NULL);
        struct tm tm_buf;
        struct tm *lt = platform_localtime(&t, &tm_buf);
        double val = 0;
        if (lt) {
            val = lt->tm_hour * 10000.0 + lt->tm_min * 100.0 + lt->tm_sec;
        }
        res.type = VAL_NUMBER; res.as.number = val;
    }
    else if (strcmp(uname, "DATE") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "DATE expects no arguments"; return res;
        }
        time_t t = time(NULL);
        struct tm tm_buf;
        struct tm *lt = platform_localtime(&t, &tm_buf);
        double val = 0;
        if (lt) {
            val = (lt->tm_year + 1900) * 10000.0 + (lt->tm_mon + 1) * 100.0 + lt->tm_mday;
        }
        res.type = VAL_NUMBER; res.as.number = val;
    }
    else if (strcmp(uname, "DAY") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "DAY expects no arguments"; return res;
        }
        time_t t = time(NULL);
        struct tm tm_buf;
        struct tm *lt = platform_localtime(&t, &tm_buf);
        res.type = VAL_NUMBER; res.as.number = lt ? lt->tm_mday : 1.0;
    }
    else if (strcmp(uname, "MONTH") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "MONTH expects no arguments"; return res;
        }
        time_t t = time(NULL);
        struct tm tm_buf;
        struct tm *lt = platform_localtime(&t, &tm_buf);
        res.type = VAL_NUMBER; res.as.number = lt ? lt->tm_mon + 1 : 1.0;
    }
    else if (strcmp(uname, "YEAR") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "YEAR expects no arguments"; return res;
        }
        time_t t = time(NULL);
        struct tm tm_buf;
        struct tm *lt = platform_localtime(&t, &tm_buf);
        res.type = VAL_NUMBER; res.as.number = lt ? lt->tm_year + 1900 : 2026.0;
    }
    else if (strcmp(uname, "DAY$") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "DAY$ expects no arguments"; return res;
        }
        time_t t = time(NULL);
        struct tm tm_buf;
        struct tm *lt = platform_localtime(&t, &tm_buf);
        const char *days[] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
        const char *day_str = (lt && lt->tm_wday >= 0 && lt->tm_wday < 7) ? days[lt->tm_wday] : "SUN";
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), day_str, strlen(day_str));
    }
    else if (strcmp(uname, "MONTH$") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "MONTH$ expects no arguments"; return res;
        }
        time_t t = time(NULL);
        struct tm tm_buf;
        struct tm *lt = platform_localtime(&t, &tm_buf);
        const char *months[] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};
        const char *mon_str = (lt && lt->tm_mon >= 0 && lt->tm_mon < 12) ? months[lt->tm_mon] : "JAN";
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), mon_str, strlen(mon_str));
    }
    else if (strcmp(uname, "HOURS") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "HOURS expects no arguments"; return res;
        }
        time_t t = time(NULL);
        struct tm tm_buf;
        extern struct tm *platform_gmtime(const time_t *timep, struct tm *result);
        struct tm *gt = platform_gmtime(&t, &tm_buf);
        res.type = VAL_NUMBER; res.as.number = gt ? gt->tm_hour : 0.0;
    }
    else if (strcmp(uname, "MINUTES") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "MINUTES expects no arguments"; return res;
        }
        time_t t = time(NULL);
        struct tm tm_buf;
        extern struct tm *platform_gmtime(const time_t *timep, struct tm *result);
        struct tm *gt = platform_gmtime(&t, &tm_buf);
        res.type = VAL_NUMBER; res.as.number = gt ? gt->tm_min : 0.0;
    }
    else if (strcmp(uname, "SECONDS") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "SECONDS expects no arguments"; return res;
        }
        time_t t = time(NULL);
        struct tm tm_buf;
        extern struct tm *platform_gmtime(const time_t *timep, struct tm *result);
        struct tm *gt = platform_gmtime(&t, &tm_buf);
        res.type = VAL_NUMBER; res.as.number = gt ? gt->tm_sec : 0.0;
    }
    else if (strcmp(uname, "JIFFIES") == 0) {
        if (arg_count > 1) {
            err->code = 13; err->message = "JIFFIES expects 0 or 1 arguments"; return res;
        }
        double mult = vm_get_jiffies_multiplier(vm);
        if (arg_count == 1) {
            if (args[0].type != VAL_STRING) {
                err->code = 13; err->message = "JIFFIES expects a string argument"; return res;
            }
            const char *mode = str_data(args[0].as.string);
            if (mode) {
                if (strcasecmp(mode, "NTSC") == 0) mult = 60.0;
                else if (strcasecmp(mode, "PAL") == 0) mult = 50.0;
                else if (strcasecmp(mode, "SECAM") == 0) mult = 50.0;
            }
        }
        res.type = VAL_NUMBER;
        res.as.number = platform_get_uptime() * mult;
    }
    else if (strcmp(uname, "TICKS") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "TICKS expects no arguments"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = platform_get_uptime() * 100.0;
    }
    else if (strcmp(uname, "HOSTNAME$") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "HOSTNAME$ expects no arguments"; return res;
        }
        const char *env_val = getenv("HOSTNAME");
        if (!env_val) env_val = getenv("COMPUTERNAME");
        if (!env_val) env_val = "localhost";
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), env_val, strlen(env_val));
    }
    else if (strcmp(uname, "USERNAME$") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "USERNAME$ expects no arguments"; return res;
        }
        const char *env_val = getenv("USERNAME");
        if (!env_val) env_val = getenv("USER");
        if (!env_val) env_val = "user";
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), env_val, strlen(env_val));
    }
    else if (strcmp(uname, "BASEPATH$") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "BASEPATH$ expects no arguments"; return res;
        }
        const char *path = vfs_get_category_path(vm_get_vfs(vm), "WORKING");
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), path ? path : "", path ? strlen(path) : 0);
    }
    else if (strcmp(uname, "BASEDIR$") == 0 || strcmp(uname, "BASENAME$") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "BASEDIR$/BASENAME$ expects no arguments"; return res;
        }
        const char *path = vfs_get_category_path(vm_get_vfs(vm), "WORKING");
        char tmp[512] = "";
        if (path) {
            size_t len = strlen(path);
            if (len < sizeof(tmp)) {
                strcpy(tmp, path);
            }
        }
        size_t len = strlen(tmp);
        if (len > 1 && (tmp[len - 1] == '/' || tmp[len - 1] == '\\')) {
            tmp[len - 1] = '\0';
            len--;
        }
        char *last_sep = strrchr(tmp, '/');
        char *last_back = strrchr(tmp, '\\');
        char *sep = (last_sep > last_back) ? last_sep : last_back;
        char res_buf[512] = "";
        if (strcmp(uname, "BASEDIR$") == 0) {
            if (sep) {
                if (sep == tmp + 2 && tmp[1] == ':') {
                    memcpy(res_buf, tmp, 3);
                    res_buf[3] = '\0';
                } else if (sep == tmp) {
                    strcpy(res_buf, "/");
                } else {
                    size_t parent_len = sep - tmp;
                    memcpy(res_buf, tmp, parent_len);
                    res_buf[parent_len] = '\0';
                }
            } else {
                strcpy(res_buf, ".");
            }
        } else {
            if (sep) {
                strcpy(res_buf, sep + 1);
            } else {
                strcpy(res_buf, tmp);
            }
        }
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), res_buf, strlen(res_buf));
    }
    else if (strcmp(uname, "PATH$") == 0) {
        if (arg_count > 1) {
            err->code = 13; err->message = "PATH$ expects 0 or 1 arguments"; return res;
        }
        const char *path_val = NULL;
        if (arg_count == 1) {
            if (args[0].type != VAL_STRING) {
                err->code = 13; err->message = "PATH$ expects a string argument"; return res;
            }
            path_val = vfs_get_category_path(vm_get_vfs(vm), str_data(args[0].as.string));
        } else {
            path_val = vfs_get_search_path(vm_get_vfs(vm));
        }
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), path_val ? path_val : "", path_val ? strlen(path_val) : 0);
    }
    else if (strcmp(uname, "VER") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "VER expects no arguments"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = 6.4;
    }
    else if (strcmp(uname, "MEM") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "MEM expects no arguments"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = (double)mem_get_free_ram(vm_get_mem(vm));
    }
    else if (strcmp(uname, "SIZE") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "SIZE expects no arguments"; return res;
        }
        MemoryContext *mctx = vm_get_mem(vm);
        size_t limit = mem_get_free_ram(mctx) + mem_get_used_ram(mctx);
        res.type = VAL_NUMBER;
        res.as.number = (double)limit;
    }
    else if (strcmp(uname, "DATE$") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "DATE$ expects no arguments"; return res;
        }
        time_t t = time(NULL);
        struct tm tm_buf;
        struct tm *lt = platform_localtime(&t, &tm_buf);
        char buf[32] = "";
        if (lt) {
            strftime(buf, sizeof(buf), "%m-%d-%Y", lt);
        }
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), buf, strlen(buf));
    }
    else if (strcmp(uname, "TIMER") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "TIMER expects no arguments"; return res;
        }
        time_t t = time(NULL);
        struct tm tm_buf;
        struct tm *lt = platform_localtime(&t, &tm_buf);
        double val = 0;
        if (lt) {
            val = lt->tm_hour * 3600.0 + lt->tm_min * 60.0 + lt->tm_sec;
        }
        res.type = VAL_NUMBER;
        res.as.number = val;
    }
    else if (strcmp(uname, "ALARM") == 0) {
        if (arg_count > 1) {
            err->code = 13; err->message = "ALARM expects 0 or 1 arguments"; return res;
        }
        extern double vm_get_alarm_countdown(VMContext *vm, double seconds);
        extern double vm_get_closest_alarm_countdown(VMContext *vm);
        double val = 0.0;
        if (arg_count == 1) {
            if (args[0].type != VAL_NUMBER) {
                err->code = 13; err->message = "ALARM expects a numeric argument"; return res;
            }
            val = vm_get_alarm_countdown(vm, args[0].as.number);
        } else {
            val = vm_get_closest_alarm_countdown(vm);
        }
        res.type = VAL_NUMBER;
        res.as.number = val;
    }
    else if (strcmp(uname, "ALARM$") == 0) {
        if (arg_count > 1) {
            err->code = 13; err->message = "ALARM$ expects 0 or 1 arguments"; return res;
        }
        extern double vm_get_alarm_daily_remaining(VMContext *vm, const char *time_str);
        extern double vm_get_closest_alarm_daily_remaining(VMContext *vm);
        double remaining = -1.0;
        if (arg_count == 1) {
            char time_str[128] = "";
            if (args[0].type == VAL_NUMBER) {
                int val_i = (int)args[0].as.number;
                int h = val_i / 10000;
                int m = (val_i % 10000) / 100;
                int s = val_i % 100;
                if (h < 0 || h >= 24 || m < 0 || m >= 60 || s < 0 || s >= 60) {
                    err->code = 5; err->message = "Illegal function call: Invalid time format";
                    return res;
                }
                snprintf(time_str, sizeof(time_str), "%02d:%02d:%02d", h, m, s);
            } else {
                size_t tlen = (str_len(args[0].as.string) < 127) ? str_len(args[0].as.string) : 127;
                memcpy(time_str, str_data(args[0].as.string), tlen);
                time_str[tlen] = '\0';
            }
            remaining = vm_get_alarm_daily_remaining(vm, time_str);
        } else {
            remaining = vm_get_closest_alarm_daily_remaining(vm);
        }

        if (remaining < 0.0) {
            res.type = VAL_STRING;
            res.as.string = str_create(vm_get_str(vm), "", 0);
        } else {
            int h = (int)(remaining / 3600.0);
            int m = (int)(((int)remaining % 3600) / 60.0);
            int s = (int)((int)remaining % 60);
            char buf[16];
            snprintf(buf, sizeof(buf), "%02d:%02d:%02d", h, m, s);
            res.type = VAL_STRING;
            res.as.string = str_create(vm_get_str(vm), buf, strlen(buf));
        }
    }
    else if (strcmp(uname, "EXISTS") == 0) {
        if (arg_count != 1 || args[0].type != VAL_STRING) {
            err->code = 13; err->message = "EXISTS expects one string argument"; return res;
        }
        const char *filename = str_data(args[0].as.string);
        char resolved[512] = "";
        bool exists = false;
        if (filename) {
            vfs_resolve(vm_get_vfs(vm), filename, resolved, sizeof(resolved));
            FILE *fp = fopen(resolved, "rb");
            if (fp) {
                exists = true;
                fclose(fp);
            }
        }
        res.type = VAL_NUMBER;
        res.as.number = exists ? -1.0 : 0.0;
    }
    else if (strcmp(uname, "RANDOMIZE") == 0) {
        if (arg_count == 0) {
            err->code = 13; err->message = "RANDOMIZE function expects arguments"; return res;
        }
        if (args[0].type == VAL_STRING) {
            const char *mode = str_data(args[0].as.string);
            if (mode && strcmp(mode, "STRING$") == 0) {
                int len = 8;
                if (arg_count > 1 && args[1].type == VAL_NUMBER) {
                    len = (int)args[1].as.number;
                }
                char *buf = (char *)calloc(1, len + 1);
                if (!buf) { err->code = 14; err->message = "Out of memory"; return res; }
                const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
                for (int i = 0; i < len; i++) {
                    buf[i] = charset[rand() % (sizeof(charset) - 1)];
                }
                buf[len] = '\0';
                res.type = VAL_STRING;
                res.as.string = str_create(vm_get_str(vm), buf, len);
                free(buf);
            } else if (mode && strcmp(mode, "DATE$") == 0) {
                int m = rand() % 12 + 1;
                int d = rand() % 28 + 1;
                int y = rand() % 100;
                char buf[16];
                snprintf(buf, sizeof(buf), "%02d-%02d-%02d", m, d, y);
                res.type = VAL_STRING;
                res.as.string = str_create(vm_get_str(vm), buf, strlen(buf));
            } else if (mode && strcmp(mode, "DAY$") == 0) {
                const char *days[] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
                const char *day = days[rand() % 7];
                res.type = VAL_STRING;
                res.as.string = str_create(vm_get_str(vm), day, strlen(day));
            } else if (mode && strcmp(mode, "TIME$") == 0) {
                int h = rand() % 24;
                int m = rand() % 60;
                int s = rand() % 60;
                char buf[16];
                snprintf(buf, sizeof(buf), "%02d:%02d:%02d", h, m, s);
                res.type = VAL_STRING;
                res.as.string = str_create(vm_get_str(vm), buf, strlen(buf));
            } else {
                size_t len = strlen(mode);
                char *buf = (char *)calloc(1, len + 1);
                if (!buf) { err->code = 14; err->message = "Out of memory"; return res; }
                strcpy(buf, mode);
                for (size_t i = len - 1; i > 0; i--) {
                    size_t j = rand() % (i + 1);
                    char tmp = buf[i];
                    buf[i] = buf[j];
                    buf[j] = tmp;
                }
                res.type = VAL_STRING;
                res.as.string = str_create(vm_get_str(vm), buf, len);
                free(buf);
            }
        } else {
            err->code = 13; err->message = "RANDOMIZE function expects a string as first argument"; return res;
        }
    }
    else if (strcmp(uname, "GUID$") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "GUID$ expects no arguments"; return res;
        }
        char buf[37];
        const char *chars = "0123456789abcdef";
        for (int i = 0; i < 36; i++) {
            if (i == 8 || i == 13 || i == 18 || i == 23) {
                buf[i] = '-';
            } else if (i == 14) {
                buf[i] = '4';
            } else if (i == 19) {
                buf[i] = chars[(rand() % 4) + 8];
            } else {
                buf[i] = chars[rand() % 16];
            }
        }
        buf[36] = '\0';
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), buf, 36);
    }
    else if (strcmp(uname, "TIM") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "TIM expects one numeric argument"; return res;
        }
        int comp = (int)args[0].as.number;
        time_t t = time(NULL);
        struct tm tm_buf;
        struct tm *lt = platform_localtime(&t, &tm_buf);
        double val = 0.0;
        if (lt) {
            if (comp == 0) val = (double)lt->tm_min;
            else if (comp == 1) val = (double)lt->tm_hour;
            else if (comp == 2) val = (double)(lt->tm_yday + 1);
            else if (comp == 3) val = (double)(lt->tm_year + 1900);
            else {
                err->code = 5; err->message = "Illegal function call: TIM component must be 0, 1, 2, or 3"; return res;
            }
        }
        res.type = VAL_NUMBER;
        res.as.number = val;
    }
    else if (strcmp(uname, "ABS") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "ABS expects one numeric argument"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = fabs(args[0].as.number);
    }
    else if (strcmp(uname, "SIN") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "SIN expects one numeric argument"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = sin(args[0].as.number);
    }
    else if (strcmp(uname, "COS") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "COS expects one numeric argument"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = cos(args[0].as.number);
    }
    else if (strcmp(uname, "TAN") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "TAN expects one numeric argument"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = tan(args[0].as.number);
    }
    else if (strcmp(uname, "ATN") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "ATN expects one numeric argument"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = atan(args[0].as.number);
    }
    else if (strcmp(uname, "LOG") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "LOG expects one numeric argument"; return res;
        }
        if (args[0].as.number <= 0.0) {
            err->code = 5; err->message = "LOG of zero or negative number"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = log(args[0].as.number);
    }
    else if (strcmp(uname, "EXP") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "EXP expects one numeric argument"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = exp(args[0].as.number);
    }
    else if (strcmp(uname, "INT") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "INT expects one numeric argument"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = floor(args[0].as.number);
    }
    else if (strcmp(uname, "CINT") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "CINT expects one numeric argument"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = round(args[0].as.number);
    }
    else if (strcmp(uname, "CSNG") == 0 || strcmp(uname, "CDBL") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "CSNG/CDBL expects one numeric argument"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = args[0].as.number;
    }
    else if (strcmp(uname, "FIX") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "FIX expects one numeric argument"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = (args[0].as.number >= 0.0) ? floor(args[0].as.number) : ceil(args[0].as.number);
    }
    else if (strcmp(uname, "RND") == 0) {
        res.type = VAL_NUMBER;
        double arg = 1.0;
        if (arg_count > 0 && args[0].type == VAL_NUMBER) {
            arg = args[0].as.number;
        }
        if (arg < 0.0) {
            srand((unsigned int)(-arg));
            double val = (double)rand() / (double)RAND_MAX;
            vm_set_last_rnd(vm, val);
            res.as.number = val;
        } else if (arg == 0.0) {
            res.as.number = vm_get_last_rnd(vm);
        } else {
            double val = (double)rand() / (double)RAND_MAX;
            vm_set_last_rnd(vm, val);
            res.as.number = val;
        }
    }
    else if (strcmp(uname, "PLAY") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "PLAY function expects one numeric argument (e.g. PLAY(0))"; return res;
        }
        res.type = VAL_NUMBER;
#ifdef BASIC_LITE_BUILD
        res.as.number = 0.0;
#else
        res.as.number = (double)vdev_music_queue_length();
#endif
    }
    else if (strcmp(uname, "TASK") == 0) {
        if (arg_count != 1) {
            err->code = 13; err->message = "TASK function expects one argument"; return res;
        }
        if (args[0].type == VAL_STRING) {
            const char *filename = str_data(args[0].as.string);
            int pid = task_spawn(vm_get_vdev(vm), filename);
            str_release(vm_get_str(vm), args[0].as.string);
            res.type = VAL_NUMBER;
            res.as.number = (double)pid;
        } else if (args[0].type == VAL_NUMBER) {
            int pid = (int)args[0].as.number;
            int status = task_get_status(pid);
            res.type = VAL_NUMBER;
            res.as.number = (double)status;
        } else {
            err->code = 13; err->message = "Type mismatch: TASK function expects filename string or numeric PID";
        }
    }
    else if (strcmp(uname, "LEN") == 0) {
        if (arg_count != 1 || args[0].type != VAL_STRING) {
            err->code = 13; err->message = "LEN expects one string argument"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = (double)str_len(args[0].as.string);
        str_release(vm_get_str(vm), args[0].as.string);
    }
    else if (strcmp(uname, "ASC") == 0) {
        if (arg_count != 1 || args[0].type != VAL_STRING) {
            err->code = 13; err->message = "ASC expects one string argument"; return res;
        }
        BppStringRef sr = args[0].as.string;
        if (str_len(sr) == 0) {
            err->code = 5; err->message = "ASC of empty string";
            str_release(vm_get_str(vm), sr);
            return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = (double)(unsigned char)str_data(sr)[0];
        str_release(vm_get_str(vm), sr);
    }
    else if (strcmp(uname, "CHR$") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "CHR$ expects one numeric argument"; return res;
        }
        int code = (int)args[0].as.number;
        if (code < 0 || code > 255) {
            err->code = 5; err->message = "Illegal function call in CHR$"; return res;
        }
        char ch = (char)code;
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), &ch, 1);
    }
    else if (strcmp(uname, "VAL") == 0) {
        if (arg_count != 1 || args[0].type != VAL_STRING) {
            err->code = 13; err->message = "VAL expects one string argument"; return res;
        }
        BppStringRef sr = args[0].as.string;
        res.type = VAL_NUMBER;
        res.as.number = strtod(str_data(sr), NULL);
        str_release(vm_get_str(vm), sr);
    }
    else if (strcmp(uname, "REVERSE$") == 0) {
        if (arg_count != 1 || args[0].type != VAL_STRING) {
            err->code = 13; err->message = "REVERSE$ expects one string argument"; return res;
        }
        BppStringRef sr = args[0].as.string;
        const char *data = str_data(sr);
        size_t len = str_len(sr);
        char *buf = (char *)calloc(1, len + 1);
        if (!buf) {
            err->code = 14; err->message = "Out of memory";
            str_release(vm_get_str(vm), sr);
            return res;
        }
        for (size_t idx = 0; idx < len; idx++) {
            buf[idx] = data[len - 1 - idx];
        }
        buf[len] = '\0';
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), buf, len);
        free(buf);
        str_release(vm_get_str(vm), sr);
    }
    else if (strcmp(uname, "REPLACE$") == 0) {
        if (arg_count != 3 || args[0].type != VAL_STRING || args[1].type != VAL_STRING || args[2].type != VAL_STRING) {
            err->code = 13; err->message = "REPLACE$ expects three string arguments";
            for (int j = 0; j < arg_count; j++) {
                if (args[j].type == VAL_STRING && args[j].as.string) {
                    str_release(vm_get_str(vm), args[j].as.string);
                }
            }
            return res;
        }
        BppStringRef sr_orig = args[0].as.string;
        BppStringRef sr_find = args[1].as.string;
        BppStringRef sr_repl = args[2].as.string;

        const char *orig = str_data(sr_orig);
        const char *find = str_data(sr_find);
        const char *repl = str_data(sr_repl);
        size_t orig_len = str_len(sr_orig);
        size_t find_len = str_len(sr_find);
        size_t repl_len = str_len(sr_repl);

        if (find_len == 0) {
            res.type = VAL_STRING;
            str_add_ref(sr_orig);
            res.as.string = sr_orig;
        } else {
            size_t count = 0;
            const char *p = orig;
            while ((p = strstr(p, find)) != NULL) {
                count++;
                p += find_len;
            }

            size_t new_len = orig_len - count * find_len + count * repl_len;
            char *buf = (char *)calloc(1, new_len + 1);
            if (!buf) {
                err->code = 14; err->message = "Out of memory";
                str_release(vm_get_str(vm), sr_orig);
                str_release(vm_get_str(vm), sr_find);
                str_release(vm_get_str(vm), sr_repl);
                return res;
            }

            char *dst = buf;
            const char *src = orig;
            while (true) {
                const char *next = strstr(src, find);
                if (!next) {
                    strcpy(dst, src);
                    break;
                }
                size_t prefix_len = next - src;
                memcpy(dst, src, prefix_len);
                dst += prefix_len;
                memcpy(dst, repl, repl_len);
                dst += repl_len;
                src = next + find_len;
            }
            res.type = VAL_STRING;
            res.as.string = str_create(vm_get_str(vm), buf, new_len);
            free(buf);
        }
        str_release(vm_get_str(vm), sr_orig);
        str_release(vm_get_str(vm), sr_find);
        str_release(vm_get_str(vm), sr_repl);
    }
    else if (strcmp(uname, "REMOVE$") == 0) {
        if (arg_count != 2 || args[0].type != VAL_STRING || args[1].type != VAL_STRING) {
            err->code = 13; err->message = "REMOVE$ expects two string arguments";
            for (int j = 0; j < arg_count; j++) {
                if (args[j].type == VAL_STRING && args[j].as.string) {
                    str_release(vm_get_str(vm), args[j].as.string);
                }
            }
            return res;
        }
        BppStringRef sr_orig = args[0].as.string;
        BppStringRef sr_find = args[1].as.string;

        const char *orig = str_data(sr_orig);
        const char *find = str_data(sr_find);
        size_t orig_len = str_len(sr_orig);
        size_t find_len = str_len(sr_find);

        if (find_len == 0) {
            res.type = VAL_STRING;
            str_add_ref(sr_orig);
            res.as.string = sr_orig;
        } else {
            size_t count = 0;
            const char *p = orig;
            while ((p = strstr(p, find)) != NULL) {
                count++;
                p += find_len;
            }

            size_t new_len = orig_len - count * find_len;
            char *buf = (char *)calloc(1, new_len + 1);
            if (!buf) {
                err->code = 14; err->message = "Out of memory";
                str_release(vm_get_str(vm), sr_orig);
                str_release(vm_get_str(vm), sr_find);
                return res;
            }

            char *dst = buf;
            const char *src = orig;
            while (true) {
                const char *next = strstr(src, find);
                if (!next) {
                    strcpy(dst, src);
                    break;
                }
                size_t prefix_len = next - src;
                memcpy(dst, src, prefix_len);
                dst += prefix_len;
                src = next + find_len;
            }
            res.type = VAL_STRING;
            res.as.string = str_create(vm_get_str(vm), buf, new_len);
            free(buf);
        }
        str_release(vm_get_str(vm), sr_orig);
        str_release(vm_get_str(vm), sr_find);
    }
    else if (strcmp(uname, "REMOVE") == 0) {
        if (arg_count != 2 || args[0].type == VAL_STRING || args[1].type == VAL_STRING) {
            err->code = 13; err->message = "REMOVE function expects two numeric arguments";
            for (int j = 0; j < arg_count; j++) {
                if (args[j].type == VAL_STRING && args[j].as.string) {
                    str_release(vm_get_str(vm), args[j].as.string);
                }
            }
            return res;
        }
        double val = args[0].as.number;
        double find = args[1].as.number;
        res.type = VAL_NUMBER;
        res.as.number = (val == find) ? 0.0 : val;
    }
    else if (strcmp(uname, "HASH") == 0) {
        if (arg_count != 1 || args[0].type != VAL_STRING) {
            err->code = 13; err->message = "HASH expects one string argument"; return res;
        }
        BppStringRef sr = args[0].as.string;
        const char *data = str_data(sr);
        size_t len = str_len(sr);
        unsigned int h = 5381;
        for (size_t idx = 0; idx < len; idx++) {
            h = ((h << 5) + h) + (unsigned char)data[idx];
        }
        res.type = VAL_NUMBER;
        res.as.number = (double)h;
        str_release(vm_get_str(vm), sr);
    }
    else if (strcmp(uname, "HEX$") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "HEX$ expects one numeric argument"; return res;
        }
        unsigned long uv = (unsigned long)(long)args[0].as.number;
        char tmp[20];
        snprintf(tmp, sizeof(tmp), "%lX", uv);
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), tmp, strlen(tmp));
    }
    else if (strcmp(uname, "OCT$") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "OCT$ expects one numeric argument"; return res;
        }
        unsigned long uv = (unsigned long)(long)args[0].as.number;
        char tmp[24];
        snprintf(tmp, sizeof(tmp), "%lo", uv);
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), tmp, strlen(tmp));
    }
    else if (strcmp(uname, "BIN$") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "BIN$ expects one numeric argument"; return res;
        }
        unsigned long uv = (unsigned long)(long)args[0].as.number;
        char raw[68];
        int raw_bits = 0;
        if (uv == 0) {
            raw[raw_bits++] = '0';
        } else {
            while (uv > 0 && raw_bits < 64) {
                raw[raw_bits++] = (char)('0' + (int)(uv & 1));
                uv >>= 1;
            }
        }
        int num_bytes = (raw_bits + 7) / 8;
        int total_bits = num_bytes * 8;
        while (raw_bits < total_bits) {
            raw[raw_bits++] = '0';
        }
        char out[80];
        int o = 0;
        for (int idx = total_bits - 1; idx >= 0; idx--) {
            out[o++] = raw[idx];
            if (idx > 0 && (idx % 8) == 0) {
                out[o++] = ' ';
            }
        }
        out[o] = '\0';
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), out, o);
    }
    else if (strcmp(uname, "EDIT$") == 0) {
        if (arg_count != 2 || args[0].type != VAL_STRING || args[1].type == VAL_STRING) {
            err->code = 13; err->message = "EDIT$ expects (string$, code)";
            for (int j = 0; j < arg_count; j++) {
                if (args[j].type == VAL_STRING && args[j].as.string) {
                    str_release(vm_get_str(vm), args[j].as.string);
                }
            }
            return res;
        }
        BppStringRef sr = args[0].as.string;
        const char *src = str_data(sr);
        size_t src_len = str_len(sr);
        long code = (long)args[1].as.number;

        char *dest = (char *)calloc(1, src_len + 1);
        if (!dest) {
            err->code = 14; err->message = "Out of memory";
            str_release(vm_get_str(vm), sr);
            return res;
        }

        int dest_len = 0;
        int in_quotes = 0;
        int last_was_space = 0;

        for (size_t idx = 0; idx < src_len; idx++) {
            char c = src[idx];
            if (code & 1) c = (char)(c & 127);
            if ((code & 256) && c == '"') in_quotes = !in_quotes;
            if (in_quotes) {
                dest[dest_len++] = c;
                continue;
            }
            if ((code & 2) && (c == ' ' || c == '\t')) continue;
            if ((code & 4) && (c == '\r' || c == '\n' || c == '\f' || c == '\b' || c == 27 || c == '\0')) continue;
            if (code & 64) {
                if (c == '[') c = '(';
                else if (c == ']') c = ')';
            }
            if (code & 32) {
                if (c >= 'a' && c <= 'z') c = (char)(c - 32);
            }
            if (code & 16) {
                if (c == ' ' || c == '\t') {
                    if (last_was_space) continue;
                    c = ' ';
                    last_was_space = 1;
                } else {
                    last_was_space = 0;
                }
            }
            dest[dest_len++] = c;
        }
        dest[dest_len] = '\0';

        int start_pos = 0;
        int end_pos = dest_len;
        if (code & 8) {
            while (start_pos < end_pos && (dest[start_pos] == ' ' || dest[start_pos] == '\t')) start_pos++;
        }
        if (code & 128) {
            while (end_pos > start_pos && (dest[end_pos - 1] == ' ' || dest[end_pos - 1] == '\t')) end_pos--;
        }

        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), dest + start_pos, end_pos - start_pos);
        free(dest);
        str_release(vm_get_str(vm), sr);
    }
    else if (strcmp(uname, "NUM$") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "NUM$ expects one numeric argument"; return res;
        }
        double val = args[0].as.number;
        char tmp[64];
        eval_format_double_clean(tmp, sizeof(tmp), val, false, false);
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), tmp, strlen(tmp));
    }
    else if (strcmp(uname, "TCASE$") == 0) {
        if (arg_count != 1 || args[0].type != VAL_STRING) {
            err->code = 13; err->message = "TCASE$ expects one string argument"; return res;
        }
        BppStringRef sr = args[0].as.string;
        const char *src = str_data(sr);
        size_t len = str_len(sr);
        char *buf = (char *)calloc(1, len + 1);
        if (!buf) {
            err->code = 14; err->message = "Out of memory";
            str_release(vm_get_str(vm), sr);
            return res;
        }
        bool next_upper = true;
        for (size_t idx = 0; idx < len; idx++) {
            char c = src[idx];
            if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
                buf[idx] = c;
                next_upper = true;
            } else {
                if (next_upper) {
                    buf[idx] = (char)toupper((unsigned char)c);
                    next_upper = false;
                } else {
                    buf[idx] = (char)tolower((unsigned char)c);
                }
            }
        }
        buf[len] = '\0';
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), buf, len);
        free(buf);
        str_release(vm_get_str(vm), sr);
    }
    else if (strcmp(uname, "ICASE$") == 0) {
        if (arg_count != 1 || args[0].type != VAL_STRING) {
            err->code = 13; err->message = "ICASE$ expects one string argument"; return res;
        }
        BppStringRef sr = args[0].as.string;
        const char *src = str_data(sr);
        size_t len = str_len(sr);
        char *buf = (char *)calloc(1, len + 1);
        if (!buf) {
            err->code = 14; err->message = "Out of memory";
            str_release(vm_get_str(vm), sr);
            return res;
        }
        for (size_t idx = 0; idx < len; idx++) {
            char c = src[idx];
            if (c >= 'A' && c <= 'Z') {
                buf[idx] = (char)(c + 32);
            } else if (c >= 'a' && c <= 'z') {
                buf[idx] = (char)(c - 32);
            } else {
                buf[idx] = c;
            }
        }
        buf[len] = '\0';
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), buf, len);
        free(buf);
        str_release(vm_get_str(vm), sr);
    }
    else if (strcmp(uname, "DIR$") == 0) {
        if (arg_count > 1) {
            err->code = 13; err->message = "DIR$ expects 0 or 1 argument"; return res;
        }
        char out_name[256];
        int found = 0;
        if (arg_count == 1) {
            if (args[0].type != VAL_STRING) {
                err->code = 13; err->message = "Type mismatch (expected string for DIR$)"; return res;
            }
            if (g_eval_dir_search) platform_find_close(g_eval_dir_search);
            g_eval_dir_search = platform_find_first_file(str_data(args[0].as.string), out_name, sizeof(out_name));
            if (g_eval_dir_search) found = 1;
            str_release(vm_get_str(vm), args[0].as.string);
        } else {
            if (g_eval_dir_search) {
                found = platform_find_next_file(g_eval_dir_search, out_name, sizeof(out_name));
                if (!found) {
                    platform_find_close(g_eval_dir_search);
                    g_eval_dir_search = NULL;
                }
            }
        }
        res.type = VAL_STRING;
        if (found) {
            res.as.string = str_create(vm_get_str(vm), out_name, strlen(out_name));
        } else {
            res.as.string = str_create(vm_get_str(vm), "", 0);
        }
    }
    else if (strcmp(uname, "ENVIRON$") == 0) {
        if (arg_count != 1 || args[0].type != VAL_STRING) {
            err->code = 13; err->message = "ENVIRON$ expects one string argument"; return res;
        }
        char *val = platform_getenv(str_data(args[0].as.string));
        res.type = VAL_STRING;
        if (val) {
            res.as.string = str_create(vm_get_str(vm), val, strlen(val));
        } else {
            res.as.string = str_create(vm_get_str(vm), "", 0);
        }
        str_release(vm_get_str(vm), args[0].as.string);
    }
    else if (strcmp(uname, "GETATTR") == 0) {
        if (arg_count != 1 || args[0].type != VAL_STRING) {
            err->code = 13; err->message = "GETATTR expects one string argument"; return res;
        }
        int attr = platform_get_attributes(str_data(args[0].as.string));
        str_release(vm_get_str(vm), args[0].as.string);
        if (attr == -1) {
            err->code = 53; err->message = "File not found"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = (double)attr;
    }
    else if (strcmp(uname, "STR$") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "STR$ expects one numeric argument"; return res;
        }
        char buf[64];
        num_format_display(buf, sizeof(buf), args[0].as.number, true, false);
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), buf, strlen(buf));
    }
    else if (strcmp(uname, "LEFT$") == 0) {
        if (arg_count != 2 || args[0].type != VAL_STRING || args[1].type == VAL_STRING) {
            err->code = 13; err->message = "LEFT$ expects a string and a number"; return res;
        }
        BppStringRef sr = args[0].as.string;
        int n = (int)args[1].as.number;
        if (n < 0) {
            err->code = 5; err->message = "Negative length in LEFT$";
            str_release(vm_get_str(vm), sr);
            return res;
        }
        int len = (int)str_len(sr);
        if (n > len) n = len;
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), str_data(sr), n);
        str_release(vm_get_str(vm), sr);
    }
    else if (strcmp(uname, "RIGHT$") == 0) {
        if (arg_count != 2 || args[0].type != VAL_STRING || args[1].type == VAL_STRING) {
            err->code = 13; err->message = "RIGHT$ expects a string and a number"; return res;
        }
        BppStringRef sr = args[0].as.string;
        int n = (int)args[1].as.number;
        if (n < 0) {
            err->code = 5; err->message = "Negative length in RIGHT$";
            str_release(vm_get_str(vm), sr);
            return res;
        }
        int len = (int)str_len(sr);
        if (n > len) n = len;
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), str_data(sr) + (len - n), n);
        str_release(vm_get_str(vm), sr);
    }
    else if (strcmp(uname, "UCASE$") == 0) {
        if (arg_count != 1 || args[0].type != VAL_STRING) {
            err->code = 13; err->message = "UCASE$ expects one string argument"; return res;
        }
        BppStringRef sr = args[0].as.string;
        size_t len = str_len(sr);
        char *buf = (char *)calloc(1, len + 1);
        if (!buf) {
            err->code = 14; err->message = "Out of memory";
            str_release(vm_get_str(vm), sr);
            return res;
        }
        const char *src = str_data(sr);
        for (size_t i = 0; i < len; i++) {
            buf[i] = (char)toupper((unsigned char)src[i]);
        }
        buf[len] = '\0';
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), buf, len);
        free(buf);
        str_release(vm_get_str(vm), sr);
    }
    else if (strcmp(uname, "LCASE$") == 0) {
        if (arg_count != 1 || args[0].type != VAL_STRING) {
            err->code = 13; err->message = "LCASE$ expects one string argument"; return res;
        }
        BppStringRef sr = args[0].as.string;
        size_t len = str_len(sr);
        char *buf = (char *)calloc(1, len + 1);
        if (!buf) {
            err->code = 14; err->message = "Out of memory";
            str_release(vm_get_str(vm), sr);
            return res;
        }
        const char *src = str_data(sr);
        for (size_t i = 0; i < len; i++) {
            buf[i] = (char)tolower((unsigned char)src[i]);
        }
        buf[len] = '\0';
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), buf, len);
        free(buf);
        str_release(vm_get_str(vm), sr);
    }
    else if (strcmp(uname, "LTRIM$") == 0) {
        if (arg_count != 1 || args[0].type != VAL_STRING) {
            err->code = 13; err->message = "LTRIM$ expects one string argument"; return res;
        }
        BppStringRef sr = args[0].as.string;
        const char *src = str_data(sr);
        size_t len = str_len(sr);
        size_t start = 0;
        while (start < len && isspace((unsigned char)src[start])) {
            start++;
        }
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), src + start, len - start);
        str_release(vm_get_str(vm), sr);
    }
    else if (strcmp(uname, "RTRIM$") == 0) {
        if (arg_count != 1 || args[0].type != VAL_STRING) {
            err->code = 13; err->message = "RTRIM$ expects one string argument"; return res;
        }
        BppStringRef sr = args[0].as.string;
        const char *src = str_data(sr);
        size_t len = str_len(sr);
        size_t end = len;
        while (end > 0 && isspace((unsigned char)src[end - 1])) {
            end--;
        }
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), src, end);
        str_release(vm_get_str(vm), sr);
    }
    else if (strcmp(uname, "TRIM$") == 0) {
        if (arg_count != 1 || args[0].type != VAL_STRING) {
            err->code = 13; err->message = "TRIM$ expects one string argument"; return res;
        }
        BppStringRef sr = args[0].as.string;
        const char *src = str_data(sr);
        size_t len = str_len(sr);
        size_t start = 0;
        while (start < len && isspace((unsigned char)src[start])) {
            start++;
        }
        size_t end = len;
        while (end > start && isspace((unsigned char)src[end - 1])) {
            end--;
        }
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), src + start, end - start);
        str_release(vm_get_str(vm), sr);
    }
    else if (strcmp(uname, "SPACE$") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "SPACE$ expects one numeric argument"; return res;
        }
        int n = (int)args[0].as.number;
        if (n < 0) {
            err->code = 5; err->message = "Negative count in SPACE$"; return res;
        }
        char *buf = (char *)calloc(1, n + 1);
        if (!buf) {
            err->code = 14; err->message = "Out of memory"; return res;
        }
        memset(buf, ' ', n);
        buf[n] = '\0';
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), buf, n);
        free(buf);
    }
    else if (strcmp(uname, "STRING$") == 0) {
        if (arg_count != 2 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "STRING$ expects a number and a character/string"; return res;
        }
        int n = (int)args[0].as.number;
        if (n < 0) {
            err->code = 5; err->message = "Negative count in STRING$"; return res;
        }
        char c = ' ';
        if (args[1].type == VAL_NUMBER) {
            c = (char)args[1].as.number;
        } else {
            BppStringRef sr = args[1].as.string;
            const char *s = str_data(sr);
            if (str_len(sr) > 0) c = s[0];
            str_release(vm_get_str(vm), sr);
        }
        char *buf = (char *)calloc(1, n + 1);
        if (!buf) {
            err->code = 14; err->message = "Out of memory"; return res;
        }
        memset(buf, c, n);
        buf[n] = '\0';
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), buf, n);
        free(buf);
    }
    else if (strcmp(uname, "REPLACE$") == 0) {
        if (arg_count != 3 || args[0].type != VAL_STRING || args[1].type != VAL_STRING || args[2].type != VAL_STRING) {
            err->code = 13; err->message = "REPLACE$ expects three string arguments"; return res;
        }
        BppStringRef sr_orig = args[0].as.string;
        BppStringRef sr_find = args[1].as.string;
        BppStringRef sr_repl = args[2].as.string;

        const char *orig = str_data(sr_orig);
        const char *find = str_data(sr_find);
        const char *repl = str_data(sr_repl);

        size_t orig_len = str_len(sr_orig);
        size_t find_len = str_len(sr_find);
        size_t repl_len = str_len(sr_repl);

        res.type = VAL_STRING;

        if (find_len == 0) {
            res.as.string = str_create(vm_get_str(vm), orig, orig_len);
        } else {
            size_t count = 0;
            const char *p = orig;
            while ((p = strstr(p, find)) != NULL) {
                count++;
                p += find_len;
            }

            size_t new_len = orig_len + count * (repl_len - find_len);
            char *buf = (char *)calloc(1, new_len + 1);
            if (!buf) {
                err->code = 14; err->message = "Out of memory";
                str_release(vm_get_str(vm), sr_orig);
                str_release(vm_get_str(vm), sr_find);
                str_release(vm_get_str(vm), sr_repl);
                return res;
            }

            char *dst = buf;
            const char *src = orig;
            while (true) {
                const char *next = strstr(src, find);
                if (!next) {
                    strcpy(dst, src);
                    break;
                }
                size_t prefix_len = next - src;
                memcpy(dst, src, prefix_len);
                dst += prefix_len;
                memcpy(dst, repl, repl_len);
                dst += repl_len;
                src = next + find_len;
            }
            res.as.string = str_create(vm_get_str(vm), buf, new_len);
            free(buf);
        }

        str_release(vm_get_str(vm), sr_orig);
        str_release(vm_get_str(vm), sr_find);
        str_release(vm_get_str(vm), sr_repl);
    }
    else if (strcmp(uname, "HELP") == 0 || strcmp(uname, "HELP$") == 0) {
        if (arg_count != 1 || args[0].type != VAL_STRING) {
            err->code = 13; err->message = "HELP expects a string argument"; return res;
        }
        BppStringRef sr = args[0].as.string;
        const char *target = str_data(sr);
        BppMetadataRegistry *reg = vm_get_metadata(vm);
        const char *doc = metadata_get_docstring(reg, target);
        
        res.type = VAL_STRING;
        if (doc) {
            res.as.string = str_create(vm_get_str(vm), doc, strlen(doc));
        } else {
            const BppMetadataBlock *block = metadata_get_block(reg, "KEYWORD", target);
            if (!block) block = metadata_get_block(reg, "SCOPE", target);
            if (!block) block = metadata_get_block(reg, "ALIAS", target);
            if (!block) block = metadata_get_block(reg, "OPTION", target);

            if (block) {
                char temp_buf[2048];
                snprintf(temp_buf, sizeof(temp_buf), "[%s BLOCK: %s]\nDocstring: %s\nBody:\n%s", 
                         block->block_type, block->target_name, block->docstring, block->body);
                res.as.string = str_create(vm_get_str(vm), temp_buf, strlen(temp_buf));
            } else {
                const FunctionEntry *entry = funcreg_find_by_name(target);
                if (entry) {
                    char temp_buf[512];
                    snprintf(temp_buf, sizeof(temp_buf), "Function: %s\nHelp:     %s", entry->name, entry->help_text);
                    res.as.string = str_create(vm_get_str(vm), temp_buf, strlen(temp_buf));
                } else {
                    res.as.string = str_create(vm_get_str(vm), "", 0);
                }
            }
        }
        str_release(vm_get_str(vm), sr);
    }
    else if (strcmp(uname, "MID$") == 0) {
        if ((arg_count != 2 && arg_count != 3) || args[0].type != VAL_STRING || 
            args[1].type == VAL_STRING || (arg_count == 3 && args[2].type == VAL_STRING)) {
            err->code = 13; err->message = "MID$ expects MID$(s$, start[, len])"; return res;
        }
        BppStringRef sr = args[0].as.string;
        int start = (int)args[1].as.number;
        int n = (arg_count == 3) ? (int)args[2].as.number : -1;
        if (start <= 0 || (arg_count == 3 && n < 0)) {
            err->code = 5; err->message = "Invalid bounds in MID$";
            str_release(vm_get_str(vm), sr);
            return res;
        }
        res.type = VAL_STRING;
        res.as.string = str_mid(vm_get_str(vm), sr, start, n);
        str_release(vm_get_str(vm), sr);
    }
    else if (strcmp(uname, "INSTR") == 0) {
        int start = 1;
        BppStringRef s1 = NULL;
        BppStringRef s2 = NULL;

        if (arg_count == 2) {
            if (args[0].type != VAL_STRING || args[1].type != VAL_STRING) {
                err->code = 13; err->message = "INSTR expects string arguments"; return res;
            }
            s1 = args[0].as.string;
            s2 = args[1].as.string;
        } else if (arg_count == 3) {
            if (args[0].type == VAL_STRING || args[1].type != VAL_STRING || args[2].type != VAL_STRING) {
                err->code = 13; err->message = "INSTR expects start, s1$, s2$"; return res;
            }
            start = (int)args[0].as.number;
            s1 = args[1].as.string;
            s2 = args[2].as.string;
        } else {
            err->code = 13; err->message = "Invalid argument count for INSTR"; return res;
        }

        int len1 = (int)str_len(s1);
        int len2 = (int)str_len(s2);
        double found_idx = 0.0;

        if (start <= 0 || start > len1) {
            found_idx = 0.0;
        } else if (len2 == 0) {
            found_idx = (double)start;
        } else {
            const char *str1 = str_data(s1);
            const char *str2 = str_data(s2);
            const char *ptr = strstr(str1 + (start - 1), str2);
            if (ptr) {
                found_idx = (double)(ptr - str1 + 1);
            }
        }

        res.type = VAL_NUMBER;
        res.as.number = found_idx;
        
        if (s1) str_release(vm_get_str(vm), s1);
        if (s2) str_release(vm_get_str(vm), s2);
    }
    else if (strcmp(uname, "EOF") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "EOF expects numeric channel"; return res;
        }
        int ch = (int)args[0].as.number;
        res.type = VAL_NUMBER;
        res.as.number = file_eof(vm_get_file(vm), ch) ? -1.0 : 0.0;
    }
    else if (strcmp(uname, "LOF") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "LOF expects numeric channel"; return res;
        }
        int ch = (int)args[0].as.number;
        res.type = VAL_NUMBER;
        res.as.number = (double)file_lof(vm_get_file(vm), ch);
    }
    else if (strcmp(uname, "LOC") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "LOC expects numeric channel"; return res;
        }
        int ch = (int)args[0].as.number;
        res.type = VAL_NUMBER;
        res.as.number = (double)file_loc(vm_get_file(vm), ch);
    }
    else if (strcmp(uname, "SEEK") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "SEEK expects numeric channel"; return res;
        }
        int ch = (int)args[0].as.number;
        res.type = VAL_NUMBER;
        res.as.number = (double)file_loc(vm_get_file(vm), ch);
    }
    else if (strcmp(uname, "FREEFILE") == 0) {
        int free_ch = 1;
        for (int c = 1; c <= 16; ++c) {
            if (!file_is_open(vm_get_file(vm), c)) {
                free_ch = c;
                break;
            }
        }
        res.type = VAL_NUMBER;
        res.as.number = (double)free_ch;
    }
    else if (strcmp(uname, "FILEATTR") == 0) {
        if (arg_count != 2 || args[0].type == VAL_STRING || args[1].type == VAL_STRING) {
            err->code = 13; err->message = "FILEATTR expects two numeric arguments"; return res;
        }
        int ch = (int)args[0].as.number;
        int attr = (int)args[1].as.number;
        if (!file_is_open(vm_get_file(vm), ch)) {
            err->code = 52; err->message = "Bad file number"; return res;
        }
        res.type = VAL_NUMBER;
        if (attr == 1) {
            BppFileMode fmode = file_get_mode(vm_get_file(vm), ch);
            switch (fmode) {
                case FILE_MODE_INPUT: res.as.number = 1; break;
                case FILE_MODE_OUTPUT: res.as.number = 2; break;
                case FILE_MODE_RANDOM: res.as.number = 4; break;
                case FILE_MODE_APPEND: res.as.number = 8; break;
                case FILE_MODE_BINARY: res.as.number = 32; break;
                default: res.as.number = 0; break;
            }
        } else if (attr == 2) {
            res.as.number = (double)ch; /* Map to internal VDev handle ID */
        } else {
            err->code = 5; err->message = "Invalid FILEATTR attribute"; return res;
        }
    }
    else if (strcmp(uname, "INPUT$") == 0) {
        if (arg_count < 1 || arg_count > 2) {
            err->code = 13; err->message = "INPUT$ expects 1 or 2 arguments"; return res;
        }
        if (args[0].type == VAL_STRING) {
            err->code = 13; err->message = "INPUT$ expects numeric count"; return res;
        }
        int n = (int)args[0].as.number;
        if (n < 1) {
            err->code = 5; err->message = "Count must be positive"; return res;
        }

        char *buf = (char *)calloc(1, n + 1);
        if (!buf) {
            err->code = 7; err->message = "Out of memory"; return res;
        }

        if (arg_count == 1) {
            for (int i = 0; i < n; ++i) {
                int c = platform_getch();
                buf[i] = (char)c;
            }
            buf[n] = '\0';
            res.type = VAL_STRING;
            res.as.string = str_create(vm_get_str(vm), buf, n);
        } else {
            if (args[1].type == VAL_STRING) {
                free(buf);
                err->code = 13; err->message = "Channel must be numeric"; return res;
            }
            int ch = (int)args[1].as.number;
            if (!file_is_open(vm_get_file(vm), ch)) {
                free(buf);
                err->code = 52; err->message = "Bad file number"; return res;
            }
            int read_res = file_read(vm_get_file(vm), ch, buf, n);
            if (read_res < 0) read_res = 0;
            buf[read_res] = '\0';
            res.type = VAL_STRING;
            res.as.string = str_create(vm_get_str(vm), buf, read_res);
        }
        free(buf);
    }
    else if (strcmp(uname, "SCREEN") == 0) {
        if (arg_count < 2 || arg_count > 3 || args[0].type == VAL_STRING || args[1].type == VAL_STRING) {
            err->code = 13; err->message = "SCREEN expects SCREEN(row, col [, color_flag])"; return res;
        }
        int row = (int)args[0].as.number;
        int col = (int)args[1].as.number;
        int color_flag = 0;
        if (arg_count == 3) {
            if (args[2].type == VAL_STRING) {
                err->code = 13; err->message = "color_flag must be numeric"; return res;
            }
            color_flag = (int)args[2].as.number;
        }
        
        int r = row - 1;
        int c = col - 1;
        int val = -1;

#ifndef NO_SDL2
        if (vdev_get(vm_get_vdev(vm), "CON:") != NULL) {
            if (color_flag == 0) {
                val = gfx_get_char_at(r, c);
            } else {
                val = gfx_get_attr_at(r, c);
            }
        }
#endif

        if (val <= 0) {
            if (color_flag == 0) {
                val = platform_screen_get_char(r, c);
            } else {
                val = platform_screen_get_attr(r, c);
            }
        }

        if (val <= 0) {
            VConContext *vcon = vm_get_vcon(vm);
            if (vcon) {
                int active_idx = vcon_get_active_index(vcon);
                if (color_flag == 0) {
                    val = vcon_get_char_at(vcon, active_idx, r, c);
                } else {
                    val = vcon_get_attr_at(vcon, active_idx, r, c);
                }
            } else {
                val = (color_flag == 0) ? 32 : 7;
            }
        }

        res.type = VAL_NUMBER;
        res.as.number = (double)val;
    }
    else if (strcmp(uname, "IOCTL$") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "IOCTL$ expects one numeric channel"; return res;
        }
        int ch = (int)args[0].as.number;
        if (!file_is_open(vm_get_file(vm), ch)) {
            err->code = 52; err->message = "Bad file number"; return res;
        }
        
        VDev *dev = file_get_vdev(vm_get_file(vm), ch);
        char buf[256] = "";
        if (dev && dev->dev_ioctl) {
            int io_res = dev->dev_ioctl(dev, 0x7FFF, buf);
            if (io_res < 0) {
                err->code = 57; err->message = "Device I/O error"; return res;
            }
        }
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), buf, strlen(buf));
    }
    else if (strcmp(uname, "TXNSTATUS") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "TXNSTATUS expects no arguments"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = (double)file_txn_status(vm_get_file(vm));
    }
    else if (strcmp(uname, "BIOSTATUS") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "BIOSTATUS expects BIOSTATUS(chan)"; return res;
        }
        int ch = (int)args[0].as.number;
        int status = 0;
        FileContext *fctx = vm_get_file(vm);
        if (file_is_open(fctx, ch)) {
            status |= 1;
            FILE *fp = file_get_handle(fctx, ch);
            if (fp) {
                status |= 2;
                status |= 4;
                status |= 8;
                status |= 16;
                status |= 32;
            } else if (file_get_vdev(fctx, ch) != NULL) {
                status |= 64;
            }
        }
        res.type = VAL_NUMBER;
        res.as.number = (double)status;
    }
    else if (strcmp(uname, "BIOSIZE") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "BIOSIZE expects BIOSIZE(chan)"; return res;
        }
        int ch = (int)args[0].as.number;
        if (!file_is_open(vm_get_file(vm), ch)) {
            err->code = 52; err->message = "Bad file number"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = (double)file_lof(vm_get_file(vm), ch);
    }
    else if (strcmp(uname, "BIOCHECKSUM") == 0) {
        if (arg_count != 1 || args[0].type != VAL_STRING) {
            err->code = 13; err->message = "BIOCHECKSUM expects BIOCHECKSUM(data$)"; return res;
        }
        BppStringRef data_ref = args[0].as.string;
        const unsigned char *data = data_ref ? (const unsigned char *)str_data(data_ref) : (const unsigned char *)"";
        size_t len = data_ref ? str_len(data_ref) : 0;
        res.type = VAL_NUMBER;
        res.as.number = (double)eval_compute_crc16(data, len);
        if (data_ref) str_release(vm_get_str(vm), data_ref);
    }
    else if (strcmp(uname, "BIOCOMPARE") == 0) {
        if (arg_count != 3 || args[0].type == VAL_STRING || args[1].type == VAL_STRING || args[2].type != VAL_STRING) {
            err->code = 13; err->message = "BIOCOMPARE expects BIOCOMPARE(chan, pos, data$)"; return res;
        }
        int ch = (int)args[0].as.number;
        long pos = (long)args[1].as.number;
        BppStringRef data_ref = args[2].as.string;
        const char *data = data_ref ? str_data(data_ref) : "";
        int len = data_ref ? (int)str_len(data_ref) : 0;

        FileContext *fctx = vm_get_file(vm);
        if (!file_is_open(fctx, ch)) {
            err->code = 52; err->message = "Bad file number"; return res;
        }

        FILE *fp = file_get_handle(fctx, ch);
        long saved_pos = fp ? ftell(fp) : 0;

        file_seek(fctx, ch, pos + 1);
        unsigned char *buf = (unsigned char *)calloc(1, len > 0 ? len : 1);
        if (!buf) {
            err->code = 7; err->message = "Out of memory"; return res;
        }
        int read_bytes = file_read(fctx, ch, buf, len);
        if (read_bytes < 0) read_bytes = 0;

        int diff_pos = 0;
        for (int i = 0; i < len; ++i) {
            unsigned char b_val = (i < read_bytes) ? buf[i] : 0;
            if (b_val != (unsigned char)data[i]) {
                diff_pos = i + 1;
                break;
            }
        }
        free(buf);

        if (fp) fseek(fp, saved_pos, SEEK_SET);

        res.type = VAL_NUMBER;
        res.as.number = (double)diff_pos;
        if (data_ref) str_release(vm_get_str(vm), data_ref);
    }
    else if (strcmp(uname, "SIOREAD$") == 0) {
        if (arg_count != 2 || args[0].type == VAL_STRING || args[1].type == VAL_STRING) {
            err->code = 13; err->message = "SIOREAD$ expects SIOREAD$(chan, n)"; return res;
        }
        int ch = (int)args[0].as.number;
        int n = (int)args[1].as.number;
        if (!file_is_open(vm_get_file(vm), ch)) {
            err->code = 52; err->message = "Bad file number"; return res;
        }
        if (n < 1 || n > 1024) {
            err->code = 5; err->message = "Invalid read length"; return res;
        }

        char *buf = (char *)calloc(1, n + 1);
        if (!buf) {
            err->code = 7; err->message = "Out of memory"; return res;
        }

        int bytes_read = file_read(vm_get_file(vm), ch, buf, n);
        if (bytes_read < 0) bytes_read = 0;
        buf[bytes_read] = '\0';
        
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), buf, bytes_read);
        free(buf);
    }
    else if (strcmp(uname, "SIOREADLN$") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "SIOREADLN$ expects SIOREADLN$(chan)"; return res;
        }
        int ch = (int)args[0].as.number;
        if (!file_is_open(vm_get_file(vm), ch)) {
            err->code = 52; err->message = "Bad file number"; return res;
        }

        char buf[1024] = "";
        char *gets_res = file_gets(vm_get_file(vm), ch, buf, sizeof(buf));
        if (!gets_res) {
            buf[0] = '\0';
        } else {
            size_t len = strlen(buf);
            while (len > 0 && (buf[len - 1] == '\r' || buf[len - 1] == '\n')) {
                buf[len - 1] = '\0';
                len--;
            }
        }
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), buf, strlen(buf));
    }
    else if (strcmp(uname, "SIOWRITE") == 0) {
        if (arg_count != 2 || args[0].type == VAL_STRING || args[1].type != VAL_STRING) {
            err->code = 13; err->message = "SIOWRITE expects SIOWRITE(chan, data$)"; return res;
        }
        int ch = (int)args[0].as.number;
        BppStringRef data_ref = args[1].as.string;
        const char *data = data_ref ? str_data(data_ref) : "";
        int len = data_ref ? (int)str_len(data_ref) : 0;

        FileContext *fctx = vm_get_file(vm);
        if (!file_is_open(fctx, ch)) {
            err->code = 52; err->message = "Bad file number"; return res;
        }

        if (file_txn_status(fctx) > 0) {
            FILE *fp = file_get_handle(fctx, ch);
            if (fp) {
                long pos = ftell(fp);
                unsigned char *orig = (unsigned char *)calloc(1, len);
                if (orig) {
                    int orig_read = (int)fread(orig, 1, len, fp);
                    if (orig_read < len) {
                        memset(orig + orig_read, 0, len - orig_read);
                    }
                    file_txn_log_write(fctx, ch, pos, orig, len);
                    free(orig);
                }
                fseek(fp, pos, SEEK_SET);
            }
        }

        int written = file_write(fctx, ch, data, len);
        if (written < 0) written = 0;
        res.type = VAL_NUMBER;
        res.as.number = (double)written;
    }
    else if (strcmp(uname, "SIOSEEK") == 0) {
        if (arg_count != 2 || args[0].type == VAL_STRING || args[1].type == VAL_STRING) {
            err->code = 13; err->message = "SIOSEEK expects SIOSEEK(chan, pos)"; return res;
        }
        int ch = (int)args[0].as.number;
        long pos = (long)args[1].as.number;

        if (!file_is_open(vm_get_file(vm), ch)) {
            err->code = 52; err->message = "Bad file number"; return res;
        }

        file_seek(vm_get_file(vm), ch, pos + 1);
        res.type = VAL_NUMBER;
        res.as.number = (double)file_loc(vm_get_file(vm), ch);
    }
    else if (strcmp(uname, "SIOFLUSH") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "SIOFLUSH expects SIOFLUSH(chan)"; return res;
        }
        int ch = (int)args[0].as.number;
        if (!file_is_open(vm_get_file(vm), ch)) {
            err->code = 52; err->message = "Bad file number"; return res;
        }

        int fres = file_flush(vm_get_file(vm), ch);
        res.type = VAL_NUMBER;
        res.as.number = (double)fres;
    }
    else if (strcmp(uname, "SIOSTATUS") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "SIOSTATUS expects SIOSTATUS(chan)"; return res;
        }
        int ch = (int)args[0].as.number;
        int status = 0;
        FileContext *fctx = vm_get_file(vm);
        if (file_is_open(fctx, ch)) {
            status |= 1;
            if (file_eof(fctx, ch)) status |= 2;
            if (file_get_vdev(fctx, ch) != NULL) status |= 8;
        }
        res.type = VAL_NUMBER;
        res.as.number = (double)status;
    }
    else if (strcmp(uname, "SIOAVAIL") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "SIOAVAIL expects SIOAVAIL(chan)"; return res;
        }
        int ch = (int)args[0].as.number;
        if (!file_is_open(vm_get_file(vm), ch)) {
            err->code = 52; err->message = "Bad file number"; return res;
        }

        long lof = file_lof(vm_get_file(vm), ch);
        long loc = file_loc(vm_get_file(vm), ch);
        res.type = VAL_NUMBER;
        res.as.number = (double)(lof - loc >= 0 ? lof - loc : 0);
    }
    else if (strcmp(uname, "BIOREAD$") == 0) {
        if (arg_count != 3 || args[0].type == VAL_STRING || args[1].type == VAL_STRING || args[2].type == VAL_STRING) {
            err->code = 13; err->message = "BIOREAD$ expects BIOREAD$(chan, pos, n)"; return res;
        }
        int ch = (int)args[0].as.number;
        long pos = (long)args[1].as.number;
        int n = (int)args[2].as.number;

        if (!file_is_open(vm_get_file(vm), ch)) {
            err->code = 52; err->message = "Bad file number"; return res;
        }
        if (n < 1 || n > 1024) {
            err->code = 5; err->message = "Invalid read length"; return res;
        }

        FILE *fp = file_get_handle(vm_get_file(vm), ch);
        long saved_pos = fp ? ftell(fp) : 0;

        file_seek(vm_get_file(vm), ch, pos + 1);

        char *buf = (char *)calloc(1, n + 1);
        if (!buf) {
            err->code = 7; err->message = "Out of memory"; return res;
        }
        int bytes_read = file_read(vm_get_file(vm), ch, buf, n);
        if (bytes_read < 0) bytes_read = 0;
        buf[bytes_read] = '\0';

        if (fp) fseek(fp, saved_pos, SEEK_SET);

        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), buf, bytes_read);
        free(buf);
    }
    else if (strcmp(uname, "BIOWRITE") == 0) {
        if (arg_count != 3 || args[0].type == VAL_STRING || args[1].type == VAL_STRING || args[2].type != VAL_STRING) {
            err->code = 13; err->message = "BIOWRITE expects BIOWRITE(chan, pos, data$)"; return res;
        }
        int ch = (int)args[0].as.number;
        long pos = (long)args[1].as.number;
        BppStringRef data_ref = args[2].as.string;
        const char *data = data_ref ? str_data(data_ref) : "";
        int len = data_ref ? (int)str_len(data_ref) : 0;

        FileContext *fctx = vm_get_file(vm);
        if (!file_is_open(fctx, ch)) {
            err->code = 52; err->message = "Bad file number"; return res;
        }

        FILE *fp = file_get_handle(fctx, ch);
        long saved_pos = fp ? ftell(fp) : 0;

        file_seek(fctx, ch, pos + 1);

        if (file_txn_status(fctx) > 0 && fp) {
            unsigned char *orig = (unsigned char *)calloc(1, len);
            if (orig) {
                int orig_read = (int)fread(orig, 1, len, fp);
                if (orig_read < len) {
                    memset(orig + orig_read, 0, len - orig_read);
                }
                file_txn_log_write(fctx, ch, pos, orig, len);
                free(orig);
            }
            fseek(fp, pos, SEEK_SET);
        }

        int written = file_write(fctx, ch, data, len);
        if (written < 0) written = 0;

        if (fp) fseek(fp, saved_pos, SEEK_SET);

        res.type = VAL_NUMBER;
        res.as.number = (double)written;
    }
    else if (strcmp(uname, "BIOCOPY") == 0) {
        if (arg_count != 4 || args[0].type == VAL_STRING || args[1].type == VAL_STRING || args[2].type == VAL_STRING || args[3].type == VAL_STRING) {
            err->code = 13; err->message = "BIOCOPY expects BIOCOPY(chan, src_pos, dest_pos, n)"; return res;
        }
        int ch = (int)args[0].as.number;
        long src_pos = (long)args[1].as.number;
        long dest_pos = (long)args[2].as.number;
        int n = (int)args[3].as.number;

        FileContext *fctx = vm_get_file(vm);
        if (!file_is_open(fctx, ch)) {
            err->code = 52; err->message = "Bad file number"; return res;
        }
        if (n <= 0 || n > 65536) {
            err->code = 5; err->message = "Invalid copy length"; return res;
        }

        FILE *fp = file_get_handle(fctx, ch);
        if (!fp) {
            err->code = 57; err->message = "BIOCOPY: file handle not available"; return res;
        }
        long saved_pos = ftell(fp);

        unsigned char *buf = (unsigned char *)calloc(1, n);
        if (!buf) {
            err->code = 7; err->message = "Out of memory"; return res;
        }
        fseek(fp, src_pos, SEEK_SET);
        int read_bytes = (int)fread(buf, 1, n, fp);
        if (read_bytes < n) {
            memset(buf + read_bytes, 0, n - read_bytes);
        }

        if (file_txn_status(fctx) > 0 && fp) {
            unsigned char *orig = (unsigned char *)calloc(1, n);
            if (orig) {
                fseek(fp, dest_pos, SEEK_SET);
                int orig_read = (int)fread(orig, 1, n, fp);
                if (orig_read < n) {
                    memset(orig + orig_read, 0, n - orig_read);
                }
                file_txn_log_write(fctx, ch, dest_pos, orig, n);
                free(orig);
            }
        }

        fseek(fp, dest_pos, SEEK_SET);
        int written = (int)fwrite(buf, 1, n, fp);
        fflush(fp);

        if (fp) fseek(fp, saved_pos, SEEK_SET);
        free(buf);

        res.type = VAL_NUMBER;
        res.as.number = (double)written;
    }
    else if (strcmp(uname, "BIOFILL") == 0) {
        if (arg_count != 4 || args[0].type == VAL_STRING || args[1].type == VAL_STRING || args[2].type == VAL_STRING || args[3].type == VAL_STRING) {
            err->code = 13; err->message = "BIOFILL expects BIOFILL(chan, pos, n, byte_val)"; return res;
        }
        int ch = (int)args[0].as.number;
        long pos = (long)args[1].as.number;
        int n = (int)args[2].as.number;
        unsigned char val = (unsigned char)args[3].as.number;

        FileContext *fctx = vm_get_file(vm);
        if (!file_is_open(fctx, ch)) {
            err->code = 52; err->message = "Bad file number"; return res;
        }
        if (n <= 0 || n > 65536) {
            err->code = 5; err->message = "Invalid fill length"; return res;
        }

        FILE *fp = file_get_handle(fctx, ch);
        if (!fp) {
            err->code = 57; err->message = "BIOFILL: file handle not available"; return res;
        }
        long saved_pos = ftell(fp);

        if (file_txn_status(fctx) > 0 && fp) {
            unsigned char *orig = (unsigned char *)calloc(1, n);
            if (orig) {
                fseek(fp, pos, SEEK_SET);
                int orig_read = (int)fread(orig, 1, n, fp);
                if (orig_read < n) {
                    memset(orig + orig_read, 0, n - orig_read);
                }
                file_txn_log_write(fctx, ch, pos, orig, n);
                free(orig);
            }
        }

        unsigned char *buf = (unsigned char *)calloc(1, n);
        if (!buf) {
            err->code = 7; err->message = "Out of memory"; return res;
        }
        memset(buf, val, n);
        fseek(fp, pos, SEEK_SET);
        int written = (int)fwrite(buf, 1, n, fp);
        fflush(fp);

        if (fp) fseek(fp, saved_pos, SEEK_SET);
        free(buf);

        res.type = VAL_NUMBER;
        res.as.number = (double)written;
    }
    else if (strcmp(uname, "MKI$") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "MKI$ expects numeric argument"; return res;
        }
        short val = (short)args[0].as.number;
        char buf[2];
        memcpy(buf, &val, 2);
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), buf, 2);
    }
    else if (strcmp(uname, "MKS$") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "MKS$ expects numeric argument"; return res;
        }
        float val = (float)args[0].as.number;
        char buf[4];
        memcpy(buf, &val, 4);
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), buf, 4);
    }
    else if (strcmp(uname, "MKD$") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "MKD$ expects numeric argument"; return res;
        }
        double val = args[0].as.number;
        char buf[8];
        memcpy(buf, &val, 8);
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), buf, 8);
    }
    else if (strcmp(uname, "CVI") == 0) {
        if (arg_count != 1 || args[0].type != VAL_STRING) {
            err->code = 13; err->message = "CVI expects string argument"; return res;
        }
        if (args[0].as.string && str_len(args[0].as.string) >= 2) {
            short val;
            memcpy(&val, str_data(args[0].as.string), 2);
            res.type = VAL_NUMBER;
            res.as.number = (double)val;
        } else {
            err->code = 5; err->message = "Illegal function call: CVI string too short"; return res;
        }
    }
    else if (strcmp(uname, "CVS") == 0) {
        if (arg_count != 1 || args[0].type != VAL_STRING) {
            err->code = 13; err->message = "CVS expects string argument"; return res;
        }
        if (args[0].as.string && str_len(args[0].as.string) >= 4) {
            float val;
            memcpy(&val, str_data(args[0].as.string), 4);
            res.type = VAL_NUMBER;
            res.as.number = (double)val;
        } else {
            err->code = 5; err->message = "Illegal function call: CVS string too short"; return res;
        }
    }
    else if (strcmp(uname, "CVD") == 0) {
        if (arg_count != 1 || args[0].type != VAL_STRING) {
            err->code = 13; err->message = "CVD expects string argument"; return res;
        }
        if (args[0].as.string && str_len(args[0].as.string) >= 8) {
        double val;
            memcpy(&val, str_data(args[0].as.string), 8);
            res.type = VAL_NUMBER;
            res.as.number = val;
        } else {
            err->code = 5; err->message = "Illegal function call: CVD string too short"; return res;
        }
    }
    else if (strncmp(uname, "USR", 3) == 0) {
        int idx = 0;
        if (strlen(uname) == 4 && isdigit((unsigned char)uname[3])) {
            idx = uname[3] - '0';
        }
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "USR expects one numeric argument"; return res;
        }
        uintptr_t ptr = vm_get_usr_ptr(vm, idx);
        res.type = VAL_NUMBER;
        if (ptr != 0) {
            typedef double (*UsrFunc)(double);
            if (ptr > 0x10000) {
                UsrFunc func = (UsrFunc)(intptr_t)ptr;
                res.as.number = func(args[0].as.number);
            } else {
                res.as.number = (double)ptr + args[0].as.number;
            }
        } else {
            res.as.number = args[0].as.number;
        }
    }
    else if (strcmp(uname, "ERDEV") == 0) {
        res.type = VAL_NUMBER;
        res.as.number = 0.0;
    }
    else if (strcmp(uname, "ERDEV$") == 0) {
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), "DSK1    ", 8);
    }
    else if (strcmp(uname, "EXTERR") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "EXTERR expects one numeric argument (0 to 3)"; return res;
        }
        int mode = (int)args[0].as.number;
        res.type = VAL_NUMBER;
        if (mode == 0) res.as.number = 0.0;
        else if (mode == 1) res.as.number = 1.0;
        else if (mode == 2) res.as.number = 1.0;
        else if (mode == 3) res.as.number = 1.0;
        else res.as.number = 0.0;
    }
    else if (strcmp(uname, "HASH$") == 0) {
        if (arg_count != 2 || args[0].type != VAL_STRING || args[1].type != VAL_STRING) {
            err->code = 13; err->message = "HASH$ expects (algo$, data$)"; return res;
        }
        const char *algo = str_data(args[0].as.string);
        const char *data = str_data(args[1].as.string);
        char digest[128];
        hash_string(algo, data, digest, sizeof(digest));
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), digest, strlen(digest));
    }
    else if (strcmp(uname, "SALT$") == 0) {
        if (arg_count != 2 || args[0].type != VAL_STRING || args[1].type != VAL_STRING) {
            err->code = 13; err->message = "SALT$ expects (data$, salt$)"; return res;
        }
        const char *data = str_data(args[0].as.string);
        const char *salt = str_data(args[1].as.string);
        char combined[1024];
        snprintf(combined, sizeof(combined), "%s:%s", salt, data);
        char digest[128];
        hash_string("SHA256", combined, digest, sizeof(digest));
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), digest, strlen(digest));
    }
    else if (strcmp(uname, "AUDITCRACK") == 0 || strcmp(uname, "AUDITCRACK$") == 0) {
        if (arg_count < 2 || args[0].type != VAL_STRING || args[1].type != VAL_STRING) {
            err->code = 13; err->message = "AUDITCRACK expects (hash$, wordlist_file$ [, algo$])"; return res;
        }
        const char *target_hash = str_data(args[0].as.string);
        const char *wordlist_path = str_data(args[1].as.string);
        const char *algo = (arg_count >= 3 && args[2].type == VAL_STRING) ? str_data(args[2].as.string) : "MD5";
        
        char found_word[256] = {0};
        FILE *fp = fopen(wordlist_path, "r");
        if (fp) {
            char line[256];
            while (fgets(line, sizeof(line), fp)) {
                size_t len = strlen(line);
                while (len > 0 && (line[len-1] == '\r' || line[len-1] == '\n')) line[--len] = '\0';
                char digest[128];
                hash_string(algo, line, digest, sizeof(digest));
                if (strcasecmp(digest, target_hash) == 0) {
                    snprintf(found_word, sizeof(found_word), "%s", line);
                    break;
                }
            }
            fclose(fp);
        }
        if (uname[strlen(uname)-1] == '$') {
            res.type = VAL_STRING;
            res.as.string = str_create(vm_get_str(vm), found_word, strlen(found_word));
        } else {
            res.type = VAL_NUMBER;
            res.as.number = (found_word[0] != '\0') ? 1.0 : 0.0;
        }
    }
    else if (strcmp(uname, "SANDBOXAUDIT") == 0) {
        res.type = VAL_NUMBER;
        res.as.number = 0.0;
    }
    else if (strcmp(uname, "VMCHECK") == 0) {
        res.type = VAL_NUMBER;
        res.as.number = 0.0;
    }
    else if (strcmp(uname, "NETHOST$") == 0) {
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), "localhost", 9);
    }
    else if (strcmp(uname, "NETIP$") == 0) {
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), "127.0.0.1", 9);
    }
    else if (strcmp(uname, "DEVICECOUNT") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "DEVICECOUNT expects no arguments"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = (double)vdev_count(vm_get_vdev(vm));
    }
    else if (strcmp(uname, "DEVICE$") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "DEVICE$ expects one numeric argument"; return res;
        }
        int idx = (int)args[0].as.number;
        int count = vdev_count(vm_get_vdev(vm));
        if (idx < 1 || idx > count) {
            err->code = 5; err->message = "Device index out of range"; return res;
        }
        VDev *dev = vdev_get_by_index(vm_get_vdev(vm), idx - 1);
        res.type = VAL_STRING;
        if (dev) {
            res.as.string = str_create(vm_get_str(vm), dev->name, (int)strlen(dev->name));
        } else {
            res.as.string = str_create(vm_get_str(vm), "", 0);
        }
    }
    else if (strcmp(uname, "DEVICECLASS$") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "DEVICECLASS$ expects one numeric argument"; return res;
        }
        int idx = (int)args[0].as.number;
        int count = vdev_count(vm_get_vdev(vm));
        if (idx < 1 || idx > count) {
            err->code = 5; err->message = "Device index out of range"; return res;
        }
        VDev *dev = vdev_get_by_index(vm_get_vdev(vm), idx - 1);
        res.type = VAL_STRING;
        if (dev) {
            const char *cls_name = vdev_class_name(dev->dev_class);
            res.as.string = str_create(vm_get_str(vm), cls_name, (int)strlen(cls_name));
        } else {
            res.as.string = str_create(vm_get_str(vm), "UNKNOWN", 7);
        }
    }
    else if (strcmp(uname, "DEVICEINFO$") == 0) {
        if (arg_count != 2 || args[0].type == VAL_STRING || args[1].type != VAL_STRING) {
            if (arg_count == 2) {
                if (args[0].type == VAL_STRING) { str_release(vm_get_str(vm), args[0].as.string); args[0].as.string = NULL; }
                if (args[1].type == VAL_STRING) { str_release(vm_get_str(vm), args[1].as.string); args[1].as.string = NULL; }
            } else if (arg_count == 1 && args[0].type == VAL_STRING) {
                str_release(vm_get_str(vm), args[0].as.string);
                args[0].as.string = NULL;
            }
            err->code = 13; err->message = "DEVICEINFO$ expects a numeric index and a string key"; return res;
        }
        int idx = (int)args[0].as.number;
        BppStringRef key_sr = args[1].as.string;
        const char *key = str_data(key_sr);

        int count = vdev_count(vm_get_vdev(vm));
        if (idx < 1 || idx > count) {
            err->code = 5; err->message = "Device index out of range";
            str_release(vm_get_str(vm), key_sr);
            return res;
        }

        VDev *dev = vdev_get_by_index(vm_get_vdev(vm), idx - 1);
        res.type = VAL_STRING;
        const char *val_str = "";
        if (dev) {
            val_str = vdev_info(dev, key);
        }
        if (!val_str) val_str = "";
        res.as.string = str_create(vm_get_str(vm), val_str, (int)strlen(val_str));
        str_release(vm_get_str(vm), key_sr);
    }
    else if (strcmp(uname, "POLL") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "POLL expects one numeric channel argument"; return res;
        }
        int channel = (int)args[0].as.number;
        FileContext *fctx = vm_get_file(vm);
        if (!file_is_open(fctx, channel)) {
            err->code = 52; err->message = "Bad file number"; return res;
        }
        VDev *dev = file_get_vdev(fctx, channel);
        res.type = VAL_NUMBER;
        if (dev) {
            res.as.number = (vdev_poll(dev) > 0) ? 1.0 : 0.0;
        } else {
            res.as.number = 1.0;
        }
    }
#if SUPPORT_NET
    else if (strcmp(uname, "NSTATUS") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "NSTATUS expects one numeric channel argument"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = (double)vnet_status(vm_get_vnet(vm), (int)args[0].as.number);
    }
    else if (strcmp(uname, "NCONNECTED") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "NCONNECTED expects one numeric channel argument"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = vnet_connected(vm_get_vnet(vm), (int)args[0].as.number) ? -1.0 : 0.0;
    }
    else if (strcmp(uname, "NHTTPSTATUS") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "NHTTPSTATUS expects one numeric channel argument"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = (double)vnet_http_status(vm_get_vnet(vm), (int)args[0].as.number);
    }
    else if (strcmp(uname, "HTTP_GET$") == 0) {
        if (arg_count != 1 || args[0].type != VAL_STRING) {
            err->code = 13; err->message = "HTTP_GET$ expects one string URL argument"; return res;
        }
        const char *url = str_data(args[0].as.string);
        VDev n_dev = fujinet_create_n_dev(vm);
        if (n_dev.dev_open(&n_dev, url, 1) < 0) {
            err->code = 57; err->message = "HTTP_GET$ failed to open URL";
            return res;
        }
        char *body_buf = (char *)calloc(1, 4096);
        if (!body_buf) {
            n_dev.dev_close(&n_dev);
            err->code = 7; err->message = "Out of memory";
            return res;
        }
        size_t body_cap = 4096;
        size_t body_len = 0;
            char chunk[1024];
            while (true) {
                int read_bytes = n_dev.dev_read(&n_dev, chunk, sizeof(chunk));
                if (read_bytes <= 0) {
                    break;
                }
                if (body_len + read_bytes >= body_cap) {
                    body_cap *= 2;
                    char *new_buf = realloc(body_buf, body_cap);
                    if (!new_buf) {
                        break;
                    }
                    body_buf = new_buf;
                }
                memcpy(body_buf + body_len, chunk, read_bytes);
                body_len += read_bytes;
            }
        n_dev.dev_close(&n_dev);
        str_release(vm_get_str(vm), args[0].as.string);
        args[0].as.string = NULL;
        res.type = VAL_STRING;
        if (body_buf) {
            res.as.string = str_create(vm_get_str(vm), body_buf, body_len);
            free(body_buf);
        } else {
            res.as.string = str_create(vm_get_str(vm), "", 0);
        }
    }
#endif
    else if (strcmp(uname, "INP") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "INP expects one numeric port argument"; return res;
        }
        /* Sandbox Check */
        if (security_check(SECOP_SYSTEM, 0) != 0) {
            err->code = 70; err->message = "Permission denied: Port I/O is restricted"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = (double)vdev_bus_in((int)args[0].as.number);
    }
#if SUPPORT_BIOS
    else if (strcmp(uname, "MEMMAP$") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "MEMMAP$ expects no arguments"; return res;
        }
        MockBiosModel model = vdev_bus_get_model();
        const char *m_name = mock_bios_model_to_string(model);
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), m_name, strlen(m_name));
    }
#endif
    else if (strcmp(uname, "PEEK") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "PEEK expects one numeric address argument"; return res;
        }
        /* Sandbox Check */
        if (security_check(SECOP_MEM_READ, 0) != 0) {
            err->code = 70; err->message = "Permission denied: PEEK is restricted"; return res;
        }
        uint8_t val = 0;
#ifndef BASIC_LITE_BUILD
        if (vmem_peek(vm_get_vmem(vm), (uint16_t)args[0].as.number, &val) == 0) {
#endif
            bool intercepted = false;
            val = vdev_bus_peek((unsigned long)(long)args[0].as.number, &intercepted);
#ifndef BASIC_LITE_BUILD
        }
#endif
        res.type = VAL_NUMBER;
        res.as.number = (double)val;
    }
    else if (strcmp(uname, "MAP") == 0 || strcmp(uname, "MAP_NEW") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "MAP expects no arguments"; return res;
        }
        BppMap *m = map_create();
        if (!m) {
            err->code = 14; err->message = "Out of memory creating map"; return res;
        }
        res.type = VAL_MAP;
        res.as.map = m;
    }
    else if (strcmp(uname, "MAP_SET") == 0) {
        if (arg_count != 3) {
            err->code = 13; err->message = "MAP_SET expects three arguments (map, key$, value)"; return res;
        }
        if (args[0].type != VAL_MAP) {
            err->code = 13; err->message = "First argument to MAP_SET must be a MAP";
            if (args[1].type == VAL_STRING) { str_release(vm_get_str(vm), args[1].as.string); args[1].as.string = NULL; }
            if (args[2].type == VAL_STRING) { str_release(vm_get_str(vm), args[2].as.string); args[2].as.string = NULL; }
            if (args[2].type == VAL_MAP) { map_release(vm_get_str(vm), args[2].as.map); args[2].as.map = NULL; }
            return res;
        }
        if (args[1].type != VAL_STRING) {
            err->code = 13; err->message = "Second argument to MAP_SET must be a string key";
            map_release(vm_get_str(vm), args[0].as.map);
            args[0].as.map = NULL;
            if (args[2].type == VAL_STRING) { str_release(vm_get_str(vm), args[2].as.string); args[2].as.string = NULL; }
            if (args[2].type == VAL_MAP) { map_release(vm_get_str(vm), args[2].as.map); args[2].as.map = NULL; }
            return res;
        }
        const char *k = str_data(args[1].as.string);
        bool set_ok = map_set(vm_get_str(vm), args[0].as.map, k, args[2]);
        if (!set_ok) {
            err->code = 14; err->message = "Failed to set map key";
        }
        map_release(vm_get_str(vm), args[0].as.map);
        args[0].as.map = NULL;
        str_release(vm_get_str(vm), args[1].as.string);
        args[1].as.string = NULL;
        if (args[2].type == VAL_STRING) { str_release(vm_get_str(vm), args[2].as.string); args[2].as.string = NULL; }
        if (args[2].type == VAL_MAP) { map_release(vm_get_str(vm), args[2].as.map); args[2].as.map = NULL; }

        res.type = VAL_NUMBER;
        res.as.number = set_ok ? 1.0 : 0.0;
    }
    else if (strcmp(uname, "MAP_GET") == 0 || strcmp(uname, "MAP_GET$") == 0) {
        bool expect_str = (strcmp(uname, "MAP_GET$") == 0);
        if (arg_count != 2) {
            err->code = 13; err->message = "MAP_GET expects two arguments (map, key$)"; return res;
        }
        if (args[0].type != VAL_MAP) {
            err->code = 13; err->message = "First argument to MAP_GET must be a MAP";
            if (args[1].type == VAL_STRING) { str_release(vm_get_str(vm), args[1].as.string); args[1].as.string = NULL; }
            return res;
        }
        if (args[1].type != VAL_STRING) {
            err->code = 13; err->message = "Second argument to MAP_GET must be a string key";
            map_release(vm_get_str(vm), args[0].as.map);
            args[0].as.map = NULL;
            return res;
        }
        const char *k = str_data(args[1].as.string);
        BValue val;
        if (map_get(args[0].as.map, k, &val)) {
            res = val;
            if (res.type == VAL_STRING && res.as.string) {
                str_add_ref(res.as.string);
            } else if (res.type == VAL_MAP && res.as.map) {
                map_add_ref(res.as.map);
            }
        } else {
            if (expect_str) {
                res.type = VAL_STRING;
                res.as.string = str_create(vm_get_str(vm), "", 0);
            } else {
                res.type = VAL_NUMBER;
                res.as.number = 0.0;
            }
        }
        map_release(vm_get_str(vm), args[0].as.map);
        args[0].as.map = NULL;
        str_release(vm_get_str(vm), args[1].as.string);
        args[1].as.string = NULL;

        if (expect_str && res.type != VAL_STRING) {
            char coerce_buf[64];
            if (res.type == VAL_NUMBER) {
                num_format_display(coerce_buf, sizeof(coerce_buf), res.as.number, false, false);
            } else {
                strcpy(coerce_buf, "");
            }
            res.type = VAL_STRING;
            res.as.string = str_create(vm_get_str(vm), coerce_buf, strlen(coerce_buf));
        }
    }
    else if (strcmp(uname, "MAP_REMOVE") == 0) {
        if (arg_count != 2) {
            err->code = 13; err->message = "MAP_REMOVE expects two arguments (map, key$)"; return res;
        }
        if (args[0].type != VAL_MAP) {
            err->code = 13; err->message = "First argument to MAP_REMOVE must be a MAP";
            if (args[1].type == VAL_STRING) { str_release(vm_get_str(vm), args[1].as.string); args[1].as.string = NULL; }
            return res;
        }
        if (args[1].type != VAL_STRING) {
            err->code = 13; err->message = "Second argument to MAP_REMOVE must be a string key";
            map_release(vm_get_str(vm), args[0].as.map);
            args[0].as.map = NULL;
            return res;
        }
        const char *k = str_data(args[1].as.string);
        bool rem_ok = map_remove(vm_get_str(vm), args[0].as.map, k);
        map_release(vm_get_str(vm), args[0].as.map);
        args[0].as.map = NULL;
        str_release(vm_get_str(vm), args[1].as.string);
        args[1].as.string = NULL;
        res.type = VAL_NUMBER;
        res.as.number = rem_ok ? 1.0 : 0.0;
    }
    else if (strcmp(uname, "MAP_COUNT") == 0) {
        if (arg_count != 1 || args[0].type != VAL_MAP) {
            err->code = 13; err->message = "MAP_COUNT expects one MAP argument";
            if (arg_count == 1 && args[0].type == VAL_STRING) { str_release(vm_get_str(vm), args[0].as.string); args[0].as.string = NULL; }
            return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = (double)map_count(args[0].as.map);
        map_release(vm_get_str(vm), args[0].as.map);
    }
    else if (strcmp(uname, "MAP_KEY$") == 0) {
        if (arg_count != 2) {
            err->code = 13; err->message = "MAP_KEY$ expects two arguments (map, index)"; return res;
        }
        if (args[0].type != VAL_MAP) {
            err->code = 13; err->message = "First argument to MAP_KEY$ must be a MAP";
            return res;
        }
        int idx = (int)args[1].as.number;
        const char *k = map_key(args[0].as.map, idx);
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), k ? k : "", k ? strlen(k) : 0);
        map_release(vm_get_str(vm), args[0].as.map);
    }
    else if (strcmp(uname, "MAP_HAS") == 0) {
        if (arg_count != 2) {
            err->code = 13; err->message = "MAP_HAS expects two arguments (map, key$)"; return res;
        }
        if (args[0].type != VAL_MAP) {
            err->code = 13; err->message = "First argument to MAP_HAS must be a MAP";
            if (args[1].type == VAL_STRING) { str_release(vm_get_str(vm), args[1].as.string); args[1].as.string = NULL; }
            return res;
        }
        if (args[1].type != VAL_STRING) {
            err->code = 13; err->message = "Second argument to MAP_HAS must be a string key";
            map_release(vm_get_str(vm), args[0].as.map);
            args[0].as.map = NULL;
            return res;
        }
        const char *k = str_data(args[1].as.string);
        bool has_ok = map_has(args[0].as.map, k);
        map_release(vm_get_str(vm), args[0].as.map);
        args[0].as.map = NULL;
        str_release(vm_get_str(vm), args[1].as.string);
        args[1].as.string = NULL;
        res.type = VAL_NUMBER;
        res.as.number = has_ok ? 1.0 : 0.0;
    }
    else if (strcmp(uname, "JSON_PARSE") == 0) {
        if (arg_count != 1 || args[0].type != VAL_STRING) {
            err->code = 13; err->message = "JSON_PARSE expects one string argument";
            return res;
        }
        const char *json = str_data(args[0].as.string);
        BppMap *m = map_parse_json(vm_get_str(vm), json);
        str_release(vm_get_str(vm), args[0].as.string);
        res.type = VAL_MAP;
        res.as.map = m;
    }
    else if (strcmp(uname, "JSON_STRINGIFY$") == 0) {
        if (arg_count != 1 || args[0].type != VAL_MAP) {
            err->code = 13; err->message = "JSON_STRINGIFY$ expects one MAP argument";
            return res;
        }
        char *json = map_stringify_json(args[0].as.map);
        map_release(vm_get_str(vm), args[0].as.map);
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), json ? json : "", json ? strlen(json) : 0);
        free(json);
    }
    else if (strcmp(uname, "XML_PARSE") == 0) {
        if (arg_count != 1 || args[0].type != VAL_STRING) {
            err->code = 13; err->message = "XML_PARSE expects one string argument";
            return res;
        }
        const char *xml = str_data(args[0].as.string);
        BppMap *m = map_parse_xml(vm_get_str(vm), xml);
        str_release(vm_get_str(vm), args[0].as.string);
        res.type = VAL_MAP;
        res.as.map = m;
    }
    else if (strcmp(uname, "XML_STRINGIFY$") == 0) {
        if (arg_count != 1 || args[0].type != VAL_MAP) {
            err->code = 13; err->message = "XML_STRINGIFY$ expects one MAP argument";
            return res;
        }
        char *xml = map_stringify_xml(args[0].as.map);
        map_release(vm_get_str(vm), args[0].as.map);
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), xml ? xml : "", xml ? strlen(xml) : 0);
        free(xml);
    }
    else if (strcmp(uname, "YAML_PARSE") == 0) {
        if (arg_count != 1 || args[0].type != VAL_STRING) {
            err->code = 13; err->message = "YAML_PARSE expects one string argument";
            return res;
        }
        const char *yaml = str_data(args[0].as.string);
        BppMap *m = map_parse_yaml(vm_get_str(vm), yaml);
        str_release(vm_get_str(vm), args[0].as.string);
        res.type = VAL_MAP;
        res.as.map = m;
    }
    else if (strcmp(uname, "YAML_STRINGIFY$") == 0) {
        if (arg_count != 1 || args[0].type != VAL_MAP) {
            err->code = 13; err->message = "YAML_STRINGIFY$ expects one MAP argument";
            return res;
        }
        char *yaml = map_stringify_yaml(args[0].as.map);
        map_release(vm_get_str(vm), args[0].as.map);
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), yaml ? yaml : "", yaml ? strlen(yaml) : 0);
        free(yaml);
    }
    else if (strcmp(uname, "INI_PARSE") == 0) {
        if (arg_count != 1 || args[0].type != VAL_STRING) {
            err->code = 13; err->message = "INI_PARSE expects one string argument";
            return res;
        }
        const char *ini = str_data(args[0].as.string);
        BppMap *m = map_parse_ini(vm_get_str(vm), ini);
        str_release(vm_get_str(vm), args[0].as.string);
        res.type = VAL_MAP;
        res.as.map = m;
    }
    else if (strcmp(uname, "INI_STRINGIFY$") == 0) {
        if (arg_count != 1 || args[0].type != VAL_MAP) {
            err->code = 13; err->message = "INI_STRINGIFY$ expects one MAP argument";
            return res;
        }
        char *ini = map_stringify_ini(args[0].as.map);
        map_release(vm_get_str(vm), args[0].as.map);
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), ini ? ini : "", ini ? strlen(ini) : 0);
        free(ini);
    }
    else if (strcmp(uname, "DIALECT_LOAD") == 0) {
        if (arg_count != 1 && arg_count != 2) {
            err->code = 13; err->message = "DIALECT_LOAD expects 1 or 2 arguments (filepath$ [, format$])";
            return res;
        }
        if (args[0].type != VAL_STRING) {
            err->code = 13; err->message = "DIALECT_LOAD expects string representation of dialect spec or file path";
            if (arg_count == 2 && args[1].type == VAL_STRING) {
                str_release(vm_get_str(vm), args[1].as.string);
            }
            return res;
        }
        if (arg_count == 2 && args[1].type != VAL_STRING) {
            str_release(vm_get_str(vm), args[0].as.string);
            err->code = 13; err->message = "DIALECT_LOAD expects string format (JSON, XML, YAML, or INI) as the second argument";
            return res;
        }
        
        const char *spec_str = str_data(args[0].as.string);
        char *file_content = NULL;
        const char *format = NULL;
        
        if (arg_count == 2) {
            format = str_data(args[1].as.string);
            FILE *temp_fp = fopen(spec_str, "r");
            if (temp_fp) {
                fclose(temp_fp);
                file_content = eval_read_file_to_string(spec_str);
            }
        } else {
            FILE *temp_fp = fopen(spec_str, "r");
            if (!temp_fp) {
                str_release(vm_get_str(vm), args[0].as.string);
                err->code = 5; err->message = "Dialect spec file not found or invalid format";
                return res;
            }
            fclose(temp_fp);
            file_content = eval_read_file_to_string(spec_str);
            if (!file_content) {
                str_release(vm_get_str(vm), args[0].as.string);
                err->code = 5; err->message = "Failed to read dialect spec file";
                return res;
            }
            
            const char *ext = strrchr(spec_str, '.');
            if (ext) {
                if (strcasecmp(ext, ".json") == 0) {
                    format = "JSON";
                } else if (strcasecmp(ext, ".ini") == 0) {
                    format = "INI";
                } else if (strcasecmp(ext, ".xml") == 0) {
                    format = "XML";
                } else if (strcasecmp(ext, ".yaml") == 0 || strcasecmp(ext, ".yml") == 0) {
                    format = "YAML";
                }
            }
            if (!format) {
                free(file_content);
                str_release(vm_get_str(vm), args[0].as.string);
                err->code = 5; err->message = "Could not infer dialect format from file extension (expected .json, .ini, .xml, .yaml)";
                return res;
            }
        }
        
        const char *parse_source = file_content ? file_content : spec_str;
        BppMap *map = NULL;
        if (strcasecmp(format, "JSON") == 0) {
            map = map_parse_json(vm_get_str(vm), parse_source);
        } else if (strcasecmp(format, "XML") == 0) {
            map = map_parse_xml(vm_get_str(vm), parse_source);
        } else if (strcasecmp(format, "YAML") == 0) {
            map = map_parse_yaml(vm_get_str(vm), parse_source);
        } else if (strcasecmp(format, "INI") == 0) {
            map = map_parse_ini(vm_get_str(vm), parse_source);
        } else {
            err->code = 5; err->message = "Unsupported spec format (expected JSON, XML, YAML, or INI)";
        }
        
        if (file_content) free(file_content);
        str_release(vm_get_str(vm), args[0].as.string);
        if (arg_count == 2) {
            str_release(vm_get_str(vm), args[1].as.string);
        }
        
        if (err->code == 0) {
            if (!map) {
                err->code = 5; err->message = "Failed to parse dialect spec content";
            } else {
                res.type = VAL_MAP;
                res.as.map = map;
            }
        }
    }
    else if (strcmp(uname, "DIALECT_VALIDATE") == 0) {
        if (arg_count != 1 || args[0].type != VAL_MAP) {
            err->code = 13; err->message = "DIALECT_VALIDATE expects one MAP argument";
            return res;
        }
        char val_err[512] = "";
        bool ok = dialect_validate_map(vm, args[0].as.map, val_err, sizeof(val_err));
        map_release(vm_get_str(vm), args[0].as.map);
        args[0].as.map = NULL;
        if (!ok) {
            err->code = 5;
            static char err_msg_buf[512];
            strncpy(err_msg_buf, val_err, sizeof(err_msg_buf) - 1);
            err_msg_buf[sizeof(err_msg_buf) - 1] = '\0';
            err->message = err_msg_buf;
            return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = 1.0;
    }
    else if (strcmp(uname, "DIALECT_REGISTER") == 0) {
        if (arg_count != 1 || args[0].type != VAL_MAP) {
            err->code = 13; err->message = "DIALECT_REGISTER expects one MAP argument";
            return res;
        }
        BppDialect *d = dialect_create();
        char val_err[512] = "";
        if (!d) {
            err->code = 14; err->message = "Out of memory allocating dialect";
            map_release(vm_get_str(vm), args[0].as.map);
            args[0].as.map = NULL;
            return res;
        }
        if (!dialect_load_from_map(vm, args[0].as.map, d, val_err, sizeof(val_err))) {
            dialect_free(d);
            map_release(vm_get_str(vm), args[0].as.map);
            args[0].as.map = NULL;
            err->code = 5;
            static char err_msg_buf2[512];
            strncpy(err_msg_buf2, val_err, sizeof(err_msg_buf2) - 1);
            err_msg_buf2[sizeof(err_msg_buf2) - 1] = '\0';
            err->message = err_msg_buf2;
            return res;
        }
        map_release(vm_get_str(vm), args[0].as.map);
        vm_set_active_dialect(vm, d);
        res.type = VAL_NUMBER;
        res.as.number = 1.0;
    }
    else if (strcmp(uname, "DIALECT_DOC$") == 0) {
        if (arg_count != 1 || args[0].type != VAL_MAP) {
            err->code = 13; err->message = "DIALECT_DOC$ expects one MAP argument";
            return res;
        }
        BppDialect *d = dialect_create();
        char val_err[512] = "";
        if (!d) {
            err->code = 14; err->message = "Out of memory allocating dialect";
            map_release(vm_get_str(vm), args[0].as.map);
            args[0].as.map = NULL;
            return res;
        }
        if (!dialect_load_from_map(vm, args[0].as.map, d, val_err, sizeof(val_err))) {
            dialect_free(d);
            map_release(vm_get_str(vm), args[0].as.map);
            args[0].as.map = NULL;
            err->code = 5;
            static char err_msg_buf3[512];
            strncpy(err_msg_buf3, val_err, sizeof(err_msg_buf3) - 1);
            err_msg_buf3[sizeof(err_msg_buf3) - 1] = '\0';
            err->message = err_msg_buf3;
            return res;
        }
        map_release(vm_get_str(vm), args[0].as.map);
        
        char *docs = dialect_generate_docs(vm, d);
        dialect_free(d);
        
        if (!docs) {
            err->code = 14; err->message = "Failed to generate dialect documentation";
        } else {
            res.type = VAL_STRING;
            res.as.string = str_create(vm_get_str(vm), docs, strlen(docs));
            free(docs);
        }
    }
    else {
        const FunctionEntry *entry = funcreg_find_by_name(uname);
        if (entry) {
            if (arg_count < entry->min_args) {
                err->code = 13;
                err->message = "Too few arguments for function";
                return res;
            }
            if (arg_count > entry->max_args) {
                err->code = 13;
                err->message = "Too many arguments for function";
                return res;
            }

            /* Check safety classification level gates and keyword restrictions */
            bool blocked = false;
            BppSecLevel current_lvl = security_get_level();

            if (entry->safety == FSAFE_SYSTEM) {
                if (current_lvl >= SEC_SAFE) blocked = true;
            } else if (entry->safety == FSAFE_IO) {
                if (current_lvl >= SEC_STANDARD) blocked = true;
            } else if (entry->safety == FSAFE_STATE) {
                if (current_lvl >= SEC_PARANOID) blocked = true;
            }

            if (entry->keyword != KW_NONE && security_is_keyword_restricted((int)entry->keyword)) {
                blocked = true;
            }

            if (blocked) {
                err->code = 70; /* Permission Denied */
                err->message = "Function blocked by active security level or runtime restriction";
                return res;
            }

            res = entry->handler(args, arg_count, vm);
        } else {
            err->code = 2;
            err->message = "Undefined function called";
        }
    }
 
    return res;
}

