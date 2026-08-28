// FILENAME: vdev_esp32.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot (common_boot.c)
// NEEDS: libcore, libengine, libkernel, libserver
// Implements the ESP32: virtual device driver for microcontroller peripheral streams.
//
// ---- Includes ----

#include "device/vdev_esp32.h"
#include "esp32_hal.h"
#include "iot_net.h"
#include "runtime/memory/alloc.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/ctype/ctype.h"
#include "runtime/format/snprintf.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    ESP32_DEV_GPIO,
    ESP32_DEV_ADC,
    ESP32_DEV_DAC,
    ESP32_DEV_PWM,
    ESP32_DEV_I2C,
    ESP32_DEV_WIFI,
    ESP32_DEV_GENERIC
} Esp32DevType;

typedef struct {
    Esp32DevType type;
    int pin;
    uint8_t i2c_addr;
    char read_buffer[256];
    size_t read_pos;
    size_t read_len;
} Esp32Channel;

static int vdev_esp32_dev_open(VDev *dev, const char *path, int mode) {
    (void)mode;
    if (!dev || !path) return -1;

    Esp32Channel *ch = (Esp32Channel *)calloc(1, sizeof(Esp32Channel));
    if (!ch) return -1;

    const char *p = path;
    if (strncmp(p, "ESP32:", 6) == 0 || strncmp(p, "esp32:", 6) == 0) {
        p += 6;
    }

    while (*p == '/') p++;

    if (runtime_strncasecmp(p, "GPIO/", 5) == 0) {
        ch->type = ESP32_DEV_GPIO;
        ch->pin = atoi(p + 5);
        esp32_hal_set_pin_mode(ch->pin, PIN_OUTPUT);
    } else if (runtime_strncasecmp(p, "ADC/", 4) == 0) {
        ch->type = ESP32_DEV_ADC;
        ch->pin = atoi(p + 4);
        esp32_hal_set_pin_mode(ch->pin, PIN_INPUT);
    } else if (runtime_strncasecmp(p, "DAC/", 4) == 0) {
        ch->type = ESP32_DEV_DAC;
        ch->pin = atoi(p + 4);
    } else if (runtime_strncasecmp(p, "PWM/", 4) == 0) {
        ch->type = ESP32_DEV_PWM;
        ch->pin = atoi(p + 4);
    } else if (runtime_strncasecmp(p, "I2C/", 4) == 0) {
        ch->type = ESP32_DEV_I2C;
        ch->i2c_addr = (uint8_t)strtol(p + 4, NULL, 0);
    } else if (runtime_strncasecmp(p, "WIFI", 4) == 0) {
        ch->type = ESP32_DEV_WIFI;
    } else {
        ch->type = ESP32_DEV_GENERIC;
    }

    dev->priv = ch;
    return 0;
}

static int vdev_esp32_dev_close(VDev *dev) {
    if (dev && dev->priv) {
        free(dev->priv);
        dev->priv = NULL;
    }
    return 0;
}

static int vdev_esp32_dev_read(VDev *dev, void *buffer, int len) {
    if (!dev || !dev->priv || !buffer || len <= 0) return -1;
    Esp32Channel *ch = (Esp32Channel *)dev->priv;

    if (ch->read_pos >= ch->read_len) {
        if (ch->type == ESP32_DEV_GPIO) {
            int val = esp32_hal_digital_read(ch->pin);
            snprintf(ch->read_buffer, sizeof(ch->read_buffer), "%d\n", val);
        } else if (ch->type == ESP32_DEV_ADC) {
            int val = esp32_hal_analog_read(ch->pin);
            snprintf(ch->read_buffer, sizeof(ch->read_buffer), "%d\n", val);
        } else if (ch->type == ESP32_DEV_WIFI) {
            snprintf(ch->read_buffer, sizeof(ch->read_buffer), "%s\n", iot_wifi_get_ip());
        } else {
            snprintf(ch->read_buffer, sizeof(ch->read_buffer), "0\n");
        }
        ch->read_len = strlen(ch->read_buffer);
        ch->read_pos = 0;
    }

    size_t avail = ch->read_len - ch->read_pos;
    size_t to_copy = ((size_t)len < avail) ? (size_t)len : avail;
    memcpy(buffer, ch->read_buffer + ch->read_pos, to_copy);
    ch->read_pos += to_copy;
    return (int)to_copy;
}

static int vdev_esp32_dev_write(VDev *dev, const void *buffer, int len) {
    if (!dev || !dev->priv || !buffer || len <= 0) return -1;
    Esp32Channel *ch = (Esp32Channel *)dev->priv;
    char tmp[256];
    size_t copy_len = ((size_t)len < sizeof(tmp) - 1) ? (size_t)len : sizeof(tmp) - 1;
    memcpy(tmp, buffer, copy_len);
    tmp[copy_len] = '\0';

    if (ch->type == ESP32_DEV_GPIO) {
        int val = atoi(tmp);
        esp32_hal_digital_write(ch->pin, val);
    } else if (ch->type == ESP32_DEV_DAC) {
        int val = atoi(tmp);
        esp32_hal_dac_write(ch->pin, val);
    } else if (ch->type == ESP32_DEV_PWM) {
        double duty = atof(tmp);
        esp32_hal_pwm_write(ch->pin, 5000.0, duty);
    } else if (ch->type == ESP32_DEV_I2C) {
        uint8_t val = (uint8_t)atoi(tmp);
        esp32_hal_i2c_write(ch->i2c_addr, 0, val);
    }
    return len;
}

static int vdev_esp32_getc(VDev *dev) {
    unsigned char ch = 0;
    if (vdev_esp32_dev_read(dev, &ch, 1) == 1) {
        return (int)ch;
    }
    return -1;
}

static int vdev_esp32_putc(VDev *dev, int c) {
    char ch = (char)c;
    if (vdev_esp32_dev_write(dev, &ch, 1) == 1) {
        return c;
    }
    return -1;
}

static int vdev_esp32_puts(VDev *dev, const char *s) {
    if (!s) return -1;
    return vdev_esp32_dev_write(dev, s, (int)strlen(s));
}

VDev vdev_esp32_create(const char *name) {
    VDev dev;
    memset(&dev, 0, sizeof(dev));
    dev.name = name ? name : "ESP32:";
    dev.dev_class = VDCLASS_CUSTOM;
    dev.dev_caps = VDCAP_RW | VDCAP_STREAM;
    dev.dev_description = "ESP32 Microcontroller Peripheral Stream";
    dev.ops.getc = vdev_esp32_getc;
    dev.ops.putc = vdev_esp32_putc;
    dev.ops.puts = vdev_esp32_puts;
    dev.dev_open = vdev_esp32_dev_open;
    dev.dev_close = vdev_esp32_dev_close;
    dev.dev_read = vdev_esp32_dev_read;
    dev.dev_write = vdev_esp32_dev_write;
    return dev;
}
