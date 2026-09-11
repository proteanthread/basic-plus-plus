// FILENAME: esp32_hal.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore, libengine, libkernel
// NEEDS: platform, memory
// Declares the Hardware Abstraction Layer for ESP32 and microcontroller peripherals.
//
// ---- Includes ----

#ifndef ESP32_HAL_H
#define ESP32_HAL_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// ---- Pin Modes ----
#define PIN_INPUT          0
#define PIN_OUTPUT         1
#define PIN_INPUT_PULLUP   2
#define PIN_INPUT_PULLDOWN 3

// ---- Public Functions ----
void esp32_hal_init(void);
void esp32_hal_set_pin_mode(int pin, int mode);
void esp32_hal_digital_write(int pin, int value);
int  esp32_hal_digital_read(int pin);
int  esp32_hal_analog_read(int pin);
void esp32_hal_dac_write(int pin, int value);
void esp32_hal_pwm_write(int pin, double freq, double duty);
void esp32_hal_servo_write(int pin, double angle);
bool esp32_hal_i2c_write(uint8_t addr, uint8_t reg, uint8_t val);
int  esp32_hal_i2c_read(uint8_t addr, uint8_t reg);
bool esp32_hal_spi_transfer(int cs_pin, const uint8_t *tx_buf, uint8_t *rx_buf, size_t len);

void esp32_hal_delay_ms(uint32_t ms);
void esp32_hal_delay_us(uint32_t us);
uint64_t esp32_hal_ticks_ms(void);
uint64_t esp32_hal_ticks_us(void);

#endif // ESP32_HAL_H
