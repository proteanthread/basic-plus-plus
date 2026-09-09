// FILENAME: plat_serial.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libdevice (dev_serial.c)
// NEEDS: libplatform (plat_serial.h)
// Platform serial communications implementation for Win32, POSIX, and freestanding IoT.
//
// ---- Includes ----

#include "platform/plat_serial.h"
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>

struct PlatformSerialPort {
    HANDLE handle;
    int    port_num;
    PlatformSerialConfig config;
};

static PlatformSerialPort s_win_serial_ports[8];
static bool s_win_port_in_use[8] = {false};

PlatformSerialPort *platform_serial_open(int port_num, const PlatformSerialConfig *config) {
    char port_name[32];
    snprintf(port_name, sizeof(port_name), "\\\\.\\COM%d", port_num);

    HANDLE h = CreateFileA(port_name, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (h == INVALID_HANDLE_VALUE) {
        // Fallback to simulated in-memory port if physical hardware port is not present
        return NULL;
    }

    int slot = -1;
    for (int i = 0; i < 8; i++) {
        if (!s_win_port_in_use[i]) {
            slot = i;
            break;
        }
    }
    if (slot == -1) {
        CloseHandle(h);
        return NULL;
    }
    s_win_port_in_use[slot] = true;
    PlatformSerialPort *p = &s_win_serial_ports[slot];
    memset(p, 0, sizeof(*p));
    p->handle = h;
    p->port_num = port_num;
    if (config) {
        p->config = *config;
        platform_serial_set_config(p, config);
    }

    COMMTIMEOUTS timeouts;
    memset(&timeouts, 0, sizeof(timeouts));
    timeouts.ReadIntervalTimeout = MAXDWORD;
    timeouts.ReadTotalTimeoutMultiplier = 0;
    timeouts.ReadTotalTimeoutConstant = 0;
    timeouts.WriteTotalTimeoutMultiplier = 0;
    timeouts.WriteTotalTimeoutConstant = 100;
    SetCommTimeouts(h, &timeouts);

    return p;
}

int platform_serial_close(PlatformSerialPort *port) {
    if (!port) return -1;
    if (port->handle != INVALID_HANDLE_VALUE) {
        CloseHandle(port->handle);
        port->handle = INVALID_HANDLE_VALUE;
    }
    for (int i = 0; i < 8; i++) {
        if (port == &s_win_serial_ports[i]) {
            s_win_port_in_use[i] = false;
            break;
        }
    }
    return 0;
}

int platform_serial_read(PlatformSerialPort *port, void *buf, int len) {
    if (!port || port->handle == INVALID_HANDLE_VALUE || !buf || len <= 0) return -1;
    DWORD bytes_read = 0;
    if (!ReadFile(port->handle, buf, (DWORD)len, &bytes_read, NULL)) {
        return -1;
    }
    return (int)bytes_read;
}

int platform_serial_write(PlatformSerialPort *port, const void *buf, int len) {
    if (!port || port->handle == INVALID_HANDLE_VALUE || !buf || len <= 0) return -1;
    DWORD bytes_written = 0;
    if (!WriteFile(port->handle, buf, (DWORD)len, &bytes_written, NULL)) {
        return -1;
    }
    return (int)bytes_written;
}

int platform_serial_rx_available(PlatformSerialPort *port) {
    if (!port || port->handle == INVALID_HANDLE_VALUE) return 0;
    COMSTAT stat;
    DWORD errors = 0;
    if (ClearCommError(port->handle, &errors, &stat)) {
        return (int)stat.cbInQue;
    }
    return 0;
}

int platform_serial_tx_free(PlatformSerialPort *port) {
    if (!port || port->handle == INVALID_HANDLE_VALUE) return 4096;
    COMSTAT stat;
    DWORD errors = 0;
    if (ClearCommError(port->handle, &errors, &stat)) {
        return (int)(4096 - stat.cbOutQue);
    }
    return 4096;
}

int platform_serial_set_config(PlatformSerialPort *port, const PlatformSerialConfig *config) {
    if (!port || port->handle == INVALID_HANDLE_VALUE || !config) return -1;
    DCB dcb;
    memset(&dcb, 0, sizeof(dcb));
    dcb.DCBlength = sizeof(dcb);
    if (!GetCommState(port->handle, &dcb)) return -1;

    dcb.BaudRate = (DWORD)(config->baud_rate ? config->baud_rate : CBR_9600);
    dcb.ByteSize = (BYTE)(config->data_bits ? config->data_bits : 8);
    switch (config->parity) {
        case 'E': case 'e': dcb.Parity = (BYTE)EVENPARITY; break;
        case 'O': case 'o': dcb.Parity = (BYTE)ODDPARITY; break;
        case 'M': case 'm': dcb.Parity = (BYTE)MARKPARITY; break;
        case 'S': case 's': dcb.Parity = (BYTE)SPACEPARITY; break;
        default:            dcb.Parity = (BYTE)NOPARITY; break;
    }
    dcb.StopBits = (BYTE)((config->stop_bits == 2) ? TWOSTOPBITS : ONESTOPBIT);
    dcb.fOutxCtsFlow = config->hardware_flow ? TRUE : FALSE;
    dcb.fRtsControl = config->hardware_flow ? RTS_CONTROL_HANDSHAKE : RTS_CONTROL_ENABLE;
    dcb.fOutX = config->software_flow ? TRUE : FALSE;
    dcb.fInX = config->software_flow ? TRUE : FALSE;

    if (!SetCommState(port->handle, &dcb)) return -1;
    port->config = *config;
    return 0;
}

#elif defined(__linux__) || defined(__APPLE__) || defined(__unix__)
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <stdio.h>

struct PlatformSerialPort {
    int fd;
    int port_num;
    PlatformSerialConfig config;
};

static PlatformSerialPort s_posix_serial_ports[8];
static bool s_posix_port_in_use[8] = {false};

PlatformSerialPort *platform_serial_open(int port_num, const PlatformSerialConfig *config) {
    char port_name[64];
    snprintf(port_name, sizeof(port_name), "/dev/ttyS%d", port_num - 1);
    int fd = open(port_name, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0) {
        snprintf(port_name, sizeof(port_name), "/dev/ttyUSB%d", port_num - 1);
        fd = open(port_name, O_RDWR | O_NOCTTY | O_NONBLOCK);
    }
    if (fd < 0) return NULL;

    int slot = -1;
    for (int i = 0; i < 8; i++) {
        if (!s_posix_port_in_use[i]) {
            slot = i;
            break;
        }
    }
    if (slot == -1) { close(fd); return NULL; }
    s_posix_port_in_use[slot] = true;
    PlatformSerialPort *p = &s_posix_serial_ports[slot];
    memset(p, 0, sizeof(*p));
    p->fd = fd;
    p->port_num = port_num;
    if (config) {
        p->config = *config;
        platform_serial_set_config(p, config);
    }
    return p;
}

int platform_serial_close(PlatformSerialPort *port) {
    if (!port) return -1;
    if (port->fd >= 0) {
        close(port->fd);
        port->fd = -1;
    }
    for (int i = 0; i < 8; i++) {
        if (port == &s_posix_serial_ports[i]) {
            s_posix_port_in_use[i] = false;
            break;
        }
    }
    return 0;
}

int platform_serial_read(PlatformSerialPort *port, void *buf, int len) {
    if (!port || port->fd < 0 || !buf || len <= 0) return -1;
    ssize_t res = read(port->fd, buf, (size_t)len);
    return (int)res;
}

int platform_serial_write(PlatformSerialPort *port, const void *buf, int len) {
    if (!port || port->fd < 0 || !buf || len <= 0) return -1;
    ssize_t res = write(port->fd, buf, (size_t)len);
    return (int)res;
}

int platform_serial_rx_available(PlatformSerialPort *port) {
    if (!port || port->fd < 0) return 0;
    int bytes = 0;
    if (ioctl(port->fd, FIONREAD, &bytes) == 0) {
        return bytes;
    }
    return 0;
}

int platform_serial_tx_free(PlatformSerialPort *port) {
    if (!port || port->fd < 0) return 4096;
    return 4096;
}

int platform_serial_set_config(PlatformSerialPort *port, const PlatformSerialConfig *config) {
    if (!port || port->fd < 0 || !config) return -1;
    struct termios tty;
    if (tcgetattr(port->fd, &tty) != 0) return -1;

    speed_t speed = B9600;
    switch (config->baud_rate) {
        case 300: speed = B300; break;
        case 1200: speed = B1200; break;
        case 2400: speed = B2400; break;
        case 4800: speed = B4800; break;
        case 9600: speed = B9600; break;
        case 19200: speed = B19200; break;
        case 38400: speed = B38400; break;
        case 57600: speed = B57600; break;
        case 115200: speed = B115200; break;
        default: speed = B9600; break;
    }
    cfsetospeed(&tty, speed);
    cfsetispeed(&tty, speed);

    tty.c_cflag &= ~CSIZE;
    switch (config->data_bits) {
        case 5: tty.c_cflag |= CS5; break;
        case 6: tty.c_cflag |= CS6; break;
        case 7: tty.c_cflag |= CS7; break;
        default: tty.c_cflag |= CS8; break;
    }

    if (config->parity == 'E' || config->parity == 'e') {
        tty.c_cflag |= PARENB;
        tty.c_cflag &= ~PARODD;
    } else if (config->parity == 'O' || config->parity == 'o') {
        tty.c_cflag |= (PARENB | PARODD);
    } else {
        tty.c_cflag &= ~PARENB;
    }

    if (config->stop_bits == 2) {
        tty.c_cflag |= CSTOPB;
    } else {
        tty.c_cflag &= ~CSTOPB;
    }

    tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_oflag &= ~OPOST;

    if (tcsetattr(port->fd, TCSANOW, &tty) != 0) return -1;
    port->config = *config;
    return 0;
}

#else
// Freestanding IoT / MCU Stub (Zero OS libc calls)
struct PlatformSerialPort {
    int port_num;
    PlatformSerialConfig config;
    uint8_t ring[256];
    int head;
    int tail;
};

static PlatformSerialPort s_bare_serial_ports[8];
static bool s_bare_port_in_use[8] = {false};

PlatformSerialPort *platform_serial_open(int port_num, const PlatformSerialConfig *config) {
    int slot = -1;
    for (int i = 0; i < 8; i++) {
        if (!s_bare_port_in_use[i]) {
            slot = i;
            break;
        }
    }
    if (slot == -1) return NULL;
    s_bare_port_in_use[slot] = true;
    PlatformSerialPort *p = &s_bare_serial_ports[slot];
    memset(p, 0, sizeof(*p));
    p->port_num = port_num;
    if (config) p->config = *config;
    return p;
}

int platform_serial_close(PlatformSerialPort *port) {
    if (port) {
        for (int i = 0; i < 8; i++) {
            if (port == &s_bare_serial_ports[i]) {
                s_bare_port_in_use[i] = false;
                break;
            }
        }
    }
    return 0;
}

int platform_serial_read(PlatformSerialPort *port, void *buf, int len) {
    if (!port || !buf || len <= 0) return 0;
    int count = 0;
    uint8_t *dst = (uint8_t *)buf;
    while (count < len && port->head != port->tail) {
        dst[count++] = port->ring[port->tail];
        port->tail = (port->tail + 1) % 256;
    }
    return count;
}

int platform_serial_write(PlatformSerialPort *port, const void *buf, int len) {
    if (!port || !buf || len <= 0) return 0;
    int count = 0;
    const uint8_t *src = (const uint8_t *)buf;
    while (count < len) {
        int next = (port->head + 1) % 256;
        if (next == port->tail) break; // full
        port->ring[port->head] = src[count++];
        port->head = next;
    }
    return count;
}

int platform_serial_rx_available(PlatformSerialPort *port) {
    if (!port) return 0;
    if (port->head >= port->tail) return port->head - port->tail;
    return 256 - (port->tail - port->head);
}

int platform_serial_tx_free(PlatformSerialPort *port) {
    if (!port) return 255;
    return 255 - platform_serial_rx_available(port);
}

int platform_serial_set_config(PlatformSerialPort *port, const PlatformSerialConfig *config) {
    if (!port || !config) return -1;
    port->config = *config;
    return 0;
}
#endif
