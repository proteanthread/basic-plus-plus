// FILENAME: func_baud.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (funcreg.h, language_descriptor.h)
// NEEDS: libengine (func_baud.h, vm.h)
// Implements the BAUD built-in function to query channel and console transmission rates.
//
// ---- Includes ----

#include "eval/functions/system/hardware/func_baud.h"
#include "runtime/language_descriptor.h"
#include "runtime/funcreg.h"
#include "vm/vm.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "platform/platform.h"

static const LangDesc g_baud_desc = {
    .name = "BAUD",
    .category = "System & Hardware",
    .syntax = "BAUD([channel%])",
    .description = "Returns active transmission baud rate in bps for specified channel (0 for console).",
    .error_summary = "Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SYSTEM,
    .type = FEATURE_FUNCTION
};

#define MAX_BAUD_CHANNELS 16
static double g_channel_baud_rates[MAX_BAUD_CHANNELS] = {
    115200.0, 115200.0, 115200.0, 115200.0,
    115200.0, 115200.0, 115200.0, 115200.0,
    115200.0, 115200.0, 115200.0, 115200.0,
    115200.0, 115200.0, 115200.0, 115200.0
};

static double g_apple_speed = 255.0;

double speed_get_apple_speed(void) {
    return g_apple_speed;
}

void speed_set_apple_speed(double speed) {
    if (speed < 0.0) speed = 0.0;
    if (speed > 255.0) speed = 255.0;
    g_apple_speed = speed;
}

double baud_get_channel_rate(int channel) {
    if (channel < 0 || channel >= MAX_BAUD_CHANNELS) {
        return g_channel_baud_rates[0];
    }
    return g_channel_baud_rates[channel];
}

void baud_set_channel_rate(int channel, double rate) {
    if (channel >= 0 && channel < MAX_BAUD_CHANNELS && rate >= 0.0) {
        g_channel_baud_rates[channel] = rate;
    }
}

uint32_t baud_get_char_delay_ms(int channel) {
    if (channel == 0 && g_apple_speed < 255.0) {
        double diff = 255.0 - g_apple_speed;
        if (diff <= 0.0) return 0;
        return (uint32_t)(diff * 1.0);
    }
    double rate = baud_get_channel_rate(channel);
    if (rate <= 0.0 || rate >= 115200.0) {
        return 0;
    }
    // Authentic framing calculation:
    // Baudot (45.45..75 baud): 7.5 bits per char
    if (rate >= 40.0 && rate <= 76.0) {
        double sec = 7.5 / rate;
        return (uint32_t)(sec * 1000.0 + 0.5);
    }
    // Teletype Model 33 / Bell 101 (110 baud): 11 bits per char
    if (rate >= 105.0 && rate <= 115.0) {
        double sec = 11.0 / rate;
        return (uint32_t)(sec * 1000.0 + 0.5);
    }
    // IBM 2741 Selectric (134.5 baud): 9 bits per char
    if (rate >= 130.0 && rate <= 139.0) {
        double sec = 9.0 / rate;
        return (uint32_t)(sec * 1000.0 + 0.5);
    }
    // Standard Modems & Serial (150..57600): 10 bits per char
    double sec = 10.0 / rate;
    uint32_t ms = (uint32_t)(sec * 1000.0 + 0.5);
    return ms;
}

void baud_pace_char(int channel) {
    uint32_t ms = baud_get_char_delay_ms(channel);
    if (ms > 0) {
        platform_sleep_ms(ms);
    }
}

BValue func_baud_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    (void)uname;
    (void)err;
    BValue res;
    res.type = VAL_NUMBER;
    int channel = 0;
    if (arg_count >= 1 && (args[0].type == VAL_INTEGER || args[0].type == VAL_NUMBER)) {
        channel = (int)args[0].as.number;
    }
    res.as.number = baud_get_channel_rate(channel);
    return res;
}

void func_baud_register(void) {
    lang_desc_register(&g_baud_desc);
}
