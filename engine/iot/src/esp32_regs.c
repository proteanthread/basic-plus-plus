// FILENAME: esp32_regs.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (iot_main.c)
// NEEDED BY: libengine (peek.c, poke.c)
// NEEDS: libcore (esp32_hal.h, esp32_hal.c, esp32_regs.h, string.h)
// NEEDS: libengine (string.c)
// Implements ESP32 hardware register bank memory mapping and PEEK/POKE interception.
//
// ---- Includes ----

#include "esp32_regs.h"
#include "esp32_hal.h"
#include "runtime/format/snprintf.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

// ---- Simulated Register Memory Space (64 KB peripheral block) ----
static uint32_t s_sim_gpio_out = 0;
static uint32_t s_sim_gpio_enable = 0;
static uint32_t s_sim_gpio_in = 0;
static uint32_t s_sim_rtc_cntl = 0;
static uint32_t s_sim_dport = 0;

void esp32_regs_init(void) {
    s_sim_gpio_out = 0;
    s_sim_gpio_enable = 0;
    s_sim_gpio_in = 0;
    s_sim_rtc_cntl = 0;
    s_sim_dport = 0;
}

bool esp32_is_hardware_addr(uint32_t addr) {
    return (addr >= 0x3FF00000 && addr <= 0x3FFFFFFF);
}

uint32_t esp32_reg_read32(uint32_t addr) {
    switch (addr) {
        case ESP32_GPIO_OUT_REG:
            return s_sim_gpio_out;
        case ESP32_GPIO_ENABLE_REG:
            return s_sim_gpio_enable;
        case ESP32_GPIO_IN_REG: {
            uint32_t in_val = 0;
            for (int i = 0; i < 32; i++) {
                if (esp32_hal_digital_read(i)) {
                    in_val |= (1U << i);
                }
            }
            return in_val;
        }
        case ESP32_RTC_CNTL_BASE:
            return s_sim_rtc_cntl;
        case ESP32_DR_REG_DPORT_BASE:
            return s_sim_dport;
        default:
            return 0;
    }
}

void esp32_reg_write32(uint32_t addr, uint32_t value) {
    switch (addr) {
        case ESP32_GPIO_OUT_REG:
            s_sim_gpio_out = value;
            for (int i = 0; i < 32; i++) {
                esp32_hal_digital_write(i, (value & (1U << i)) ? 1 : 0);
            }
            break;
        case ESP32_GPIO_OUT_W1TS_REG:
            s_sim_gpio_out |= value;
            for (int i = 0; i < 32; i++) {
                if (value & (1U << i)) {
                    esp32_hal_digital_write(i, 1);
                }
            }
            break;
        case ESP32_GPIO_OUT_W1TC_REG:
            s_sim_gpio_out &= ~value;
            for (int i = 0; i < 32; i++) {
                if (value & (1U << i)) {
                    esp32_hal_digital_write(i, 0);
                }
            }
            break;
        case ESP32_GPIO_ENABLE_REG:
            s_sim_gpio_enable = value;
            for (int i = 0; i < 32; i++) {
                esp32_hal_set_pin_mode(i, (value & (1U << i)) ? PIN_OUTPUT : PIN_INPUT);
            }
            break;
        case ESP32_RTC_CNTL_BASE:
            s_sim_rtc_cntl = value;
            break;
        case ESP32_DR_REG_DPORT_BASE:
            s_sim_dport = value;
            break;
        default:
            break;
    }
}

uint8_t esp32_reg_read8(uint32_t addr) {
    uint32_t aligned_addr = addr & ~3U;
    uint32_t offset = addr & 3U;
    uint32_t val32 = esp32_reg_read32(aligned_addr);
    return (uint8_t)((val32 >> (offset * 8)) & 0xFF);
}

void esp32_reg_write8(uint32_t addr, uint8_t value) {
    uint32_t aligned_addr = addr & ~3U;
    uint32_t offset = addr & 3U;
    uint32_t val32 = esp32_reg_read32(aligned_addr);
    uint32_t mask = 0xFFU << (offset * 8);
    val32 = (val32 & ~mask) | (((uint32_t)value) << (offset * 8));
    esp32_reg_write32(aligned_addr, val32);
}
