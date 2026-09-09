// FILENAME: dev_serial.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot, libkernel, libengine
// NEEDS: libkernel (vdev.h)
// Hardware and virtual serial device driver (COM1..COM8, R1..R8, AUX).
//
// ---- Includes ----

#ifndef DEVICE_DEV_SERIAL_H
#define DEVICE_DEV_SERIAL_H

#include "device/vdev.h"

#ifdef __cplusplus
extern "C" {
#endif

// Create a serial VDev for port_num (1..8) with device prefix (e.g. "COM1:", "R1:", "AUX:")
VDev dev_serial_create(const char *name, int port_num);

// Parse QBASIC-style COM parameters string: "9600,N,8,1,BIN,CS0,DS0,CD0"
bool dev_serial_parse_params(const char *params, int *out_baud, char *out_parity, int *out_databits, int *out_stopbits);

// Query serial hardware buffer depth
int dev_serial_loc(VDev *dev); // Bytes waiting in RX FIFO
int dev_serial_lof(VDev *dev); // Free bytes in TX FIFO
int dev_serial_eof(VDev *dev); // True (1) if RX FIFO is empty, 0 otherwise

#ifdef __cplusplus
}
#endif

#endif // DEVICE_DEV_SERIAL_H
