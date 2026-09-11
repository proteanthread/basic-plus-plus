// FILENAME: esp32_hal.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore, libengine, libkernel
// NEEDS: libcore (esp32_hal.h, string.h)
// NEEDS: libengine (string.c, time.h, time.c)
// Implements the Hardware Abstraction Layer for ESP32 silicon and host PC simulation.
//
// ---- Includes ----

#include "esp32_hal.h"
#include "runtime/format/snprintf.h"
#include "runtime/memory/alloc.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

#if defined(_WIN32)
#include <windows.h>
#else
#include <unistd.h>
#include <time.h>
#endif

// ---- State Variables ----
static uint8_t  s_sim_pin_modes[40] = {0};
static uint8_t  s_sim_pin_vals[40] = {0};
static uint16_t s_sim_pin_analog[40] = {0};
static double   s_sim_pwm_freq[40] = {0};
static double   s_sim_pwm_duty[40] = {0};
static double   s_sim_servo_angle[40] = {0};
static uint8_t  s_sim_i2c_regs[256][256] = {{0}};

void esp32_hal_init(void) {
    runtime_memset(s_sim_pin_modes, 0, sizeof(s_sim_pin_modes));
    runtime_memset(s_sim_pin_vals, 0, sizeof(s_sim_pin_vals));
    runtime_memset(s_sim_pin_analog, 0, sizeof(s_sim_pin_analog));
}

void esp32_hal_set_pin_mode(int pin, int mode) {
    if (pin >= 0 && pin < 40) {
        s_sim_pin_modes[pin] = (uint8_t)mode;
    }
}

void esp32_hal_digital_write(int pin, int value) {
    if (pin >= 0 && pin < 40) {
        s_sim_pin_vals[pin] = (uint8_t)(value ? 1 : 0);
    }
}

int esp32_hal_digital_read(int pin) {
    if (pin >= 0 && pin < 40) {
        return s_sim_pin_vals[pin];
    }
    return 0;
}

int esp32_hal_analog_read(int pin) {
    if (pin >= 0 && pin < 40) {
        if (s_sim_pin_analog[pin] == 0 && s_sim_pin_vals[pin] != 0) {
            return 4095;
        }
        return s_sim_pin_analog[pin];
    }
    return 0;
}

void esp32_hal_dac_write(int pin, int value) {
    if (pin >= 0 && pin < 40) {
        if (value < 0) value = 0;
        if (value > 255) value = 255;
        s_sim_pin_vals[pin] = (uint8_t)value;
    }
}

void esp32_hal_pwm_write(int pin, double freq, double duty) {
    if (pin >= 0 && pin < 40) {
        s_sim_pwm_freq[pin] = freq;
        s_sim_pwm_duty[pin] = duty;
    }
}

void esp32_hal_servo_write(int pin, double angle) {
    if (pin >= 0 && pin < 40) {
        if (angle < 0.0) angle = 0.0;
        if (angle > 180.0) angle = 180.0;
        s_sim_servo_angle[pin] = angle;
    }
}

bool esp32_hal_i2c_write(uint8_t addr, uint8_t reg, uint8_t val) {
    s_sim_i2c_regs[addr][reg] = val;
    return true;
}

int esp32_hal_i2c_read(uint8_t addr, uint8_t reg) {
    return s_sim_i2c_regs[addr][reg];
}

bool esp32_hal_spi_transfer(int cs_pin, const uint8_t *tx_buf, uint8_t *rx_buf, size_t len) {
    (void)cs_pin;
    if (!tx_buf || !rx_buf) return false;
    for (size_t i = 0; i < len; i++) {
        rx_buf[i] = tx_buf[i];
    }
    return true;
}

void esp32_hal_delay_ms(uint32_t ms) {
#if defined(_WIN32)
    Sleep(ms);
#else
    struct timespec ts;
    ts.tv_sec = (time_t)(ms / 1000);
    ts.tv_nsec = (long)((ms % 1000) * 1000000L);
    nanosleep(&ts, NULL);
#endif
}

void esp32_hal_delay_us(uint32_t us) {
#if defined(_WIN32)
    Sleep(us / 1000 > 0 ? us / 1000 : 1);
#else
    struct timespec ts;
    ts.tv_sec = (time_t)(us / 1000000L);
    ts.tv_nsec = (long)((us % 1000000L) * 1000L);
    nanosleep(&ts, NULL);
#endif
}

uint64_t esp32_hal_ticks_ms(void) {
#if defined(_WIN32)
    return (uint64_t)GetTickCount64();
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)(ts.tv_sec * 1000ULL + (uint64_t)(ts.tv_nsec / 1000000L));
#endif
}

uint64_t esp32_hal_ticks_us(void) {
#if defined(_WIN32)
    static LARGE_INTEGER freq;
    static int initialized = 0;
    if (!initialized) {
        QueryPerformanceFrequency(&freq);
        initialized = 1;
    }
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    return (uint64_t)((counter.QuadPart * 1000000ULL) / freq.QuadPart);
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)(ts.tv_sec * 1000000ULL + (uint64_t)(ts.tv_nsec / 1000L));
#endif
}
