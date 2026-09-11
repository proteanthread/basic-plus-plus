// FILENAME: iot_sensors.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (iot_main.c, iot_sensors.c)
// NEEDED BY: libengine (func_hall.c, func_touch.c, stmt_dht.c, stmt_neopixel.c)
// NEEDS: platform, memory
// Declares sensor and actuator driver interfaces for NeoPixel, DHT, Touch, and Hall.
//
// ---- Includes ----

#ifndef IOT_SENSORS_H
#define IOT_SENSORS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

void iot_sensors_init(void);

// ---- NeoPixel / WS2812 RGB LED ----
void iot_neopixel_set(int pin, int index, uint8_t r, uint8_t g, uint8_t b);
void iot_neopixel_show(int pin);
void iot_neopixel_clear(int pin);

// ---- DHT11 / DHT22 Sensor ----
bool iot_dht_read(int pin, double *temp_out, double *hum_out);

// ---- Capacitive Touch ----
int iot_touch_read(int pin);

// ---- Hall Effect Sensor ----
int iot_hall_read(void);

#endif // IOT_SENSORS_H
