// FILENAME: plat_serial.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libdevice (dev_serial.c, vdev.c)
// NEEDS: stdint.h, stdbool.h, stddef.h
// Platform-independent serial communications interface for Win32, POSIX, and bare-metal IoT.
//
// ---- Includes ----

#ifndef PLATFORM_PLAT_SERIAL_H
#define PLATFORM_PLAT_SERIAL_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "platform/platform.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct PlatformSerialPort PlatformSerialPort;

typedef struct {
    int  baud_rate;     // 300, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200
    int  data_bits;     // 5, 6, 7, 8 (default 8)
    char parity;        // 'N' (none), 'E' (even), 'O' (odd), 'M' (mark), 'S' (space)
    int  stop_bits;     // 1, 2 (default 1)
    bool hardware_flow; // CTS/RTS handshaking
    bool software_flow; // XON/XOFF flow control
} PlatformSerialConfig;

PlatformSerialPort *platform_serial_open(int port_num, const PlatformSerialConfig *config);
int                 platform_serial_close(PlatformSerialPort *port);
int                 platform_serial_read(PlatformSerialPort *port, void *buf, int len);
int                 platform_serial_write(PlatformSerialPort *port, const void *buf, int len);
int                 platform_serial_rx_available(PlatformSerialPort *port);
int                 platform_serial_tx_free(PlatformSerialPort *port);
int                 platform_serial_set_config(PlatformSerialPort *port, const PlatformSerialConfig *config);

#ifdef __cplusplus
}
#endif

#endif // PLATFORM_PLAT_SERIAL_H
