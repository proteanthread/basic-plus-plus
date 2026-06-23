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
 *    - Physical host serial communication layer backing BASIC COM channels.
 *    - Direct Win32/POSIX COM port interfaces and serial event tracking.
 * 
 * 4. WHAT TO DO IF SOMETHING BREAKS:
 *    - Check variable tables, default variable type states, and stack pointers.
 *    - Cross-reference token layouts with original detokenization specifications.
 *    - Run diagnostic verification script to identify isolated error line numbers.
 * -----------------------------------------------------------------------------
 */
#include "gw_serial.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#endif

struct GW_SerialPort {
#ifdef _WIN32
    HANDLE handle;
#else
    int fd;
#endif
};

#ifndef _WIN32
static speed_t get_baud_constant(int baud) {
    switch (baud) {
        case 50: return B50;
        case 75: return B75;
        case 110: return B110;
        case 134: return B134;
        case 150: return B150;
        case 200: return B200;
        case 300: return B300;
        case 600: return B600;
        case 1200: return B1200;
        case 1800: return B1800;
        case 2400: return B2400;
        case 4800: return B4800;
        case 9600: return B9600;
        case 19200: return B19200;
        case 38400: return B38400;
        case 57600: return B57600;
        case 115200: return B115200;
        default: return B9600;
    }
}
#endif

GW_SerialPort *gw_serial_open(const char *port_name, int baud, char parity, int data_bits, int stop_bits) {
    GW_SerialPort *port = (GW_SerialPort *)malloc(sizeof(GW_SerialPort));
    if (!port) return NULL;

#ifdef _WIN32
    // Setup port name formatting (\\\\.\\COM1) for numbers > 9
    char formatted_name[64];
    if (strncmp(port_name, "COM", 3) == 0 && strlen(port_name) > 4) {
        snprintf(formatted_name, sizeof(formatted_name), "\\\\.\\%s", port_name);
    } else {
        snprintf(formatted_name, sizeof(formatted_name), "%s", port_name);
    }

    port->handle = CreateFileA(formatted_name, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (port->handle == INVALID_HANDLE_VALUE) {
        free(port);
        return NULL;
    }

    DCB dcb;
    memset(&dcb, 0, sizeof(dcb));
    dcb.DCBlength = sizeof(dcb);
    if (!GetCommState(port->handle, &dcb)) {
        CloseHandle(port->handle);
        free(port);
        return NULL;
    }

    dcb.BaudRate = baud;
    dcb.ByteSize = data_bits;
    
    if (parity == 'E') {
        dcb.Parity = EVENPARITY;
        dcb.fParity = TRUE;
    } else if (parity == 'O') {
        dcb.Parity = ODDPARITY;
        dcb.fParity = TRUE;
    } else {
        dcb.Parity = NOPARITY;
        dcb.fParity = FALSE;
    }

    dcb.StopBits = (stop_bits == 2) ? TWOSTOPBITS : ONESTOPBIT;

    if (!SetCommState(port->handle, &dcb)) {
        CloseHandle(port->handle);
        free(port);
        return NULL;
    }

    COMMTIMEOUTS timeouts;
    timeouts.ReadIntervalTimeout = MAXDWORD;
    timeouts.ReadTotalTimeoutMultiplier = 0;
    timeouts.ReadTotalTimeoutConstant = 0;
    timeouts.WriteTotalTimeoutMultiplier = 0;
    timeouts.WriteTotalTimeoutConstant = 0;
    SetCommTimeouts(port->handle, &timeouts);
#else
    port->fd = open(port_name, O_RDWR | O_NOCTTY | O_NDELAY);
    if (port->fd == -1) {
        free(port);
        return NULL;
    }

    // Set non-blocking
    fcntl(port->fd, F_SETFL, O_NDELAY);

    struct termios options;
    tcgetattr(port->fd, &options);

    speed_t speed = get_baud_constant(baud);
    cfsetispeed(&options, speed);
    cfsetospeed(&options, speed);

    options.c_cflag |= (CLOCAL | CREAD);
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= (data_bits == 7) ? CS7 : CS8;

    if (parity == 'E') {
        options.c_cflag |= PARENB;
        options.c_cflag &= ~PARODD;
    } else if (parity == 'O') {
        options.c_cflag |= PARENB;
        options.c_cflag |= PARODD;
    } else {
        options.c_cflag &= ~PARENB;
    }

    if (stop_bits == 2) {
        options.c_cflag |= CSTOPB;
    } else {
        options.c_cflag &= ~CSTOPB;
    }

    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    options.c_oflag &= ~OPOST;
    options.c_iflag &= ~(IXON | IXOFF | IXANY);

    tcsetattr(port->fd, TCSANOW, &options);
#endif

    return port;
}

void gw_serial_close(GW_SerialPort *port) {
    if (!port) return;
#ifdef _WIN32
    CloseHandle(port->handle);
#else
    close(port->fd);
#endif
    free(port);
}

int gw_serial_write(GW_SerialPort *port, const uint8_t *buf, int len) {
    if (!port || !buf) return -1;
#ifdef _WIN32
    DWORD bytes_written = 0;
    if (WriteFile(port->handle, buf, len, &bytes_written, NULL)) {
        return (int)bytes_written;
    }
    return -1;
#else
    return write(port->fd, buf, len);
#endif
}

int gw_serial_read(GW_SerialPort *port, uint8_t *buf, int len) {
    if (!port || !buf) return -1;
#ifdef _WIN32
    DWORD bytes_read = 0;
    if (ReadFile(port->handle, buf, len, &bytes_read, NULL)) {
        return (int)bytes_read;
    }
    return -1;
#else
    return read(port->fd, buf, len);
#endif
}

int gw_serial_bytes_available(GW_SerialPort *port) {
    if (!port) return 0;
#ifdef _WIN32
    DWORD errors;
    COMSTAT stat;
    if (ClearCommError(port->handle, &errors, &stat)) {
        return (int)stat.cbInQue;
    }
    return 0;
#else
    int bytes = 0;
    if (ioctl(port->fd, FIONREAD, &bytes) >= 0) {
        return bytes;
    }
    return 0;
#endif
}
