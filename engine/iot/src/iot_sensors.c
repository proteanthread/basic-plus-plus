// FILENAME: iot_sensors.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (iot_main.c)
// NEEDED BY: libengine (func_hall.c, func_touch.c, stmt_dht.c, stmt_neopixel.c)
// NEEDS: libcore (iot_sensors.h, string.h)
// NEEDS: libengine (string.c)
// Implements sensor and actuator drivers for NeoPixel, DHT, Touch, and Hall sensors.
//
// ---- Includes ----

#include "iot_sensors.h"
#include "runtime/format/snprintf.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

#define MAX_NEOPIXELS 256
static uint8_t s_pixels[40][MAX_NEOPIXELS][3] = {{{0}}};

void iot_sensors_init(void) {
    runtime_memset(s_pixels, 0, sizeof(s_pixels));
}

void iot_neopixel_set(int pin, int index, uint8_t r, uint8_t g, uint8_t b) {
    if (pin >= 0 && pin < 40 && index >= 0 && index < MAX_NEOPIXELS) {
        s_pixels[pin][index][0] = r;
        s_pixels[pin][index][1] = g;
        s_pixels[pin][index][2] = b;
    }
}

void iot_neopixel_show(int pin) {
    (void)pin;
}

void iot_neopixel_clear(int pin) {
    if (pin >= 0 && pin < 40) {
        runtime_memset(s_pixels[pin], 0, sizeof(s_pixels[pin]));
    }
}

bool iot_dht_read(int pin, double *temp_out, double *hum_out) {
    (void)pin;
    if (temp_out) *temp_out = 24.5;
    if (hum_out) *hum_out = 45.0;
    return true;
}

int iot_touch_read(int pin) {
    (void)pin;
    return 32;
}

int iot_hall_read(void) {
    return 128;
}
