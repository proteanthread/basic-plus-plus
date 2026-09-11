// FILENAME: esp32_regs.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (esp32_regs.c, iot_main.c)
// NEEDED BY: libengine (peek.c, poke.c)
// NEEDS: platform, memory
// Declares memory-mapped register bank emulation and silicon translation for ESP32.
//
// ---- Includes ----

#ifndef ESP32_REGS_H
#define ESP32_REGS_H

#include <stdint.h>
#include <stdbool.h>

// ---- ESP32 Register Base Constants from Technical Reference Manual ----
#define ESP32_DR_REG_DPORT_BASE       0x3FF00000
#define ESP32_GPIO_BASE               0x3FF44000
#define ESP32_GPIO_OUT_REG            0x3FF44000
#define ESP32_GPIO_OUT_W1TS_REG       0x3FF44004
#define ESP32_GPIO_OUT_W1TC_REG       0x3FF44008
#define ESP32_GPIO_ENABLE_REG         0x3FF44020
#define ESP32_GPIO_IN_REG             0x3FF4403C
#define ESP32_RTC_CNTL_BASE           0x3FF48000
#define ESP32_TIMG0_BASE              0x3FF5F000
#define ESP32_UART0_BASE              0x3FF40000

// ---- Public Functions ----
void esp32_regs_init(void);
uint32_t esp32_reg_read32(uint32_t addr);
void esp32_reg_write32(uint32_t addr, uint32_t value);
uint8_t esp32_reg_read8(uint32_t addr);
void esp32_reg_write8(uint32_t addr, uint8_t value);
bool esp32_is_hardware_addr(uint32_t addr);

#endif // ESP32_REGS_H
