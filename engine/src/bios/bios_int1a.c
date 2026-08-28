// FILENAME: bios_int1a.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (calendar.h, calendar.c, hal.h)
// NEEDS: libengine (bios.h, bios.c)
// Implements virtual BIOS interrupt and hardware emulation for bios_int1a.
//
// ---- Includes ----

#include "bios/bios.h"
#include "runtime/time/calendar.h"
#include "hal/hal.h"

#ifndef BIOS_DISABLE_INT1A

static uint8_t to_bcd(int val) {
    return (uint8_t)(((val / 10) << 4) | (val % 10));
}

bool bios_int1a_handler(BiosContext* ctx, uint8_t int_num, BiosRegs* regs, void* user_data) {
    (void)user_data;
    if (!ctx || !regs || int_num != 0x1A) return false;

    uint8_t ah = (uint8_t)(regs->ax >> 8);
    BiosDataArea* bda = bios_get_bda(ctx);

    switch (ah) {
        case 0x00: // Read System Timer Ticks
            if (bda) {
                regs->cx = (uint16_t)(bda->timer_ticks >> 16);
                regs->dx = (uint16_t)(bda->timer_ticks & 0xFFFF);
                regs->ax = (uint16_t)bda->timer_overflow;
                bda->timer_overflow = 0; // Reset overflow flag on read
            }
            return true;

        case 0x01: // Set System Timer Ticks
            if (bda) {
                bda->timer_ticks = ((uint32_t)regs->cx << 16) | regs->dx;
                bda->timer_overflow = 0;
            }
            return true;

        case 0x02: // Read RTC Time
            {
                int64_t now = hal_get()->time.now_epoch_seconds ? hal_get()->time.now_epoch_seconds() : 0;
                RuntimeCalendarTime cal;
                runtime_time_epoch_to_calendar(now, &cal);
                regs->cx = (uint16_t)((to_bcd(cal.hour) << 8) | to_bcd(cal.min));
                regs->dx = (uint16_t)((to_bcd(cal.sec) << 8) | (cal.isdst > 0 ? 0x01 : 0x00));
                regs->flags &= ~0x0001; // Clear Carry (Clock Running)
            }
            return true;

        case 0x04: // Read RTC Date
            {
                int64_t now = hal_get()->time.now_epoch_seconds ? hal_get()->time.now_epoch_seconds() : 0;
                RuntimeCalendarTime cal;
                runtime_time_epoch_to_calendar(now, &cal);
                int year = cal.year;
                regs->cx = (uint16_t)((to_bcd(year / 100) << 8) | to_bcd(year % 100));
                regs->dx = (uint16_t)((to_bcd(cal.mon + 1) << 8) | to_bcd(cal.mday));
                regs->flags &= ~0x0001;
            }
            return true;


        default:
            break;
    }

    return false;
}

#endif // BIOS_DISABLE_INT1A
