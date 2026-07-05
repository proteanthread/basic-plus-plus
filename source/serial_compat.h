/**
 * Original GW-BASIC Interpreter Port (C17)
 * 
 * -----------------------------------------------------------------------------
 * MAINTENANCE & EXTENSION GUIDELINES
 * -----------------------------------------------------------------------------
 * 1. WHAT CAN BE CHANGED:
 *    - Console sizing, output formatting, or ANSI color/escape sequences.
 *    - Logic inside statement handlers to optimize standard BASIC behaviors.
 *    - Math functions (tuning logic for trigonometric or random values).
 * 
 * 2. WHAT CANNOT BE CHANGED:
 *    - Keyword/token byte mapping tables (essential for loading tokenized BAS binaries).
 *    - Segmented memory layout simulation structures.
 *    - Core mathematical parsing precedence chain (eval descent hierarchy).
 * 
 * 3. EXPECTED BEHAVIOR:
 *    - Interface definitions for physical host serial configuration and operations.
 *    - Serial hardware connection configurations and channel data buffers.
 * 
 * 4. WHAT TO DO IF SOMETHING BREAKS:
 *    - Check variable tables, default variable type states, and stack pointers.
 *    - Cross-reference token layouts with original detokenization specifications.
 *    - Run diagnostic verification script to identify isolated error line numbers.
 * -----------------------------------------------------------------------------
 */
#ifndef GW_SERIAL_H
#define GW_SERIAL_H

#include <stdint.h>

typedef struct GW_SerialPort GW_SerialPort;

// Open a serial port
//   port_name - e.g., "COM1" (Windows) or "/dev/ttyS0" (Linux)
//   baud      - e.g., 9600, 115200
//   parity    - 'N' (none), 'E' (even), 'O' (odd)
//   data_bits - 7 or 8
//   stop_bits - 1 or 2
// Returns pointer to GW_SerialPort, or NULL on error.
GW_SerialPort *gw_serial_open(const char *port_name, int baud, char parity, int data_bits, int stop_bits);

// Close the serial port
void gw_serial_close(GW_SerialPort *port);

// Write to the serial port, returns number of bytes written, or -1 on error
int gw_serial_write(GW_SerialPort *port, const uint8_t *buf, int len);

// Read from the serial port, returns number of bytes read, or -1 on error
int gw_serial_read(GW_SerialPort *port, uint8_t *buf, int len);

// Check number of bytes currently buffered and ready to read
int gw_serial_bytes_available(GW_SerialPort *port);

#endif // GW_SERIAL_H
