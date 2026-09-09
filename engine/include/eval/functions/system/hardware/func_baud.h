// FILENAME: func_baud.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libkernel (types.h, errors.h)
// Header for BAUD built-in function in BASIC++.
//
// ---- Includes ----

#ifndef FUNC_BAUD_H
#define FUNC_BAUD_H

#include "types/types.h"
#include "types/errors.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VMContext VMContext;

BValue func_baud_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
void func_baud_register(void);
double baud_get_channel_rate(int channel);
void baud_set_channel_rate(int channel, double rate);
double speed_get_apple_speed(void);
void speed_set_apple_speed(double speed);
uint32_t baud_get_char_delay_ms(int channel);
void baud_pace_char(int channel);

#ifdef __cplusplus
}
#endif

#endif // FUNC_BAUD_H
