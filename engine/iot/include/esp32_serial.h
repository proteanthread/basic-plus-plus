// FILENAME: esp32_serial.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (esp32_serial.c, iot_main.c, nil_transport.c)
// NEEDS: platform, memory
// Provides USB-UART serial bridge communication for physical ESP32 WROOM-32 boards.
//
// ---- Includes ----

#ifndef ESP32_SERIAL_H
#define ESP32_SERIAL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// @brief Checks if a physical ESP32 hardware device is connected.
bool esp32_serial_is_connected(void);

// @brief Opens physical serial port (e.g. "COM3" or "/dev/ttyUSB0") at given baud rate.
bool esp32_serial_open(const char *port_name, uint32_t baud_rate);

// @brief Closes the physical serial port.
void esp32_serial_close(void);

// @brief Sends raw buffer to physical ESP32 device.
int esp32_serial_write(const void *buf, size_t len);

// @brief Reads available bytes from physical ESP32 device.
int esp32_serial_read(void *buf, size_t max_len);

// @brief Probes and auto-detects available serial ports.
const char *esp32_serial_autodetect(void);

#ifdef __cplusplus
}
#endif

#endif // ESP32_SERIAL_H
