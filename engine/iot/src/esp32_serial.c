// FILENAME: esp32_serial.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (iot_main.c, nil_transport.c)
// NEEDS: libcore (esp32_serial.h, string.h)
// NEEDS: libengine (string.c)
// Provides USB-UART physical communication for ESP32 WROOM-32 hardware.
//
// ---- Includes ----

#include "esp32_serial.h"
#include "runtime/format/snprintf.h"
#include "runtime/memory/alloc.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

#if defined(_WIN32) || defined(_WIN64)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
static HANDLE s_serial_handle = INVALID_HANDLE_VALUE;
#else
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
static int s_serial_fd = -1;
#endif

static bool s_is_connected = false;
static char s_last_port[64] = {0};

bool esp32_serial_is_connected(void) {
    return s_is_connected;
}

bool esp32_serial_open(const char *port_name, uint32_t baud_rate) {
    if (!port_name || port_name[0] == '\0') return false;

#if defined(_WIN32) || defined(_WIN64)
    if (s_serial_handle != INVALID_HANDLE_VALUE) {
        CloseHandle(s_serial_handle);
        s_serial_handle = INVALID_HANDLE_VALUE;
    }

    char win_port[128];
    if (runtime_strncmp(port_name, "\\\\.\\", 4) != 0 && (runtime_strncmp(port_name, "COM", 3) == 0 || runtime_strncmp(port_name, "com", 3) == 0)) {
        runtime_snprintf(win_port, sizeof(win_port), "\\\\.\\%s", port_name);
    } else {
        runtime_snprintf(win_port, sizeof(win_port), "%s", port_name);
    }

    s_serial_handle = CreateFileA(win_port, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (s_serial_handle == INVALID_HANDLE_VALUE) {
        s_is_connected = false;
        return false;
    }

    DCB dcb;
    runtime_memset(&dcb, 0, sizeof(dcb));
    dcb.DCBlength = sizeof(dcb);
    if (GetCommState(s_serial_handle, &dcb)) {
        dcb.BaudRate = baud_rate ? baud_rate : CBR_115200;
        dcb.ByteSize = 8;
        dcb.Parity = NOPARITY;
        dcb.StopBits = ONESTOPBIT;
        dcb.fBinary = TRUE;
        dcb.fDtrControl = DTR_CONTROL_ENABLE;
        dcb.fRtsControl = RTS_CONTROL_ENABLE;
        SetCommState(s_serial_handle, &dcb);
    }

    COMMTIMEOUTS timeouts;
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 100;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 100;
    timeouts.WriteTotalTimeoutMultiplier = 10;
    SetCommTimeouts(s_serial_handle, &timeouts);

    s_is_connected = true;
    runtime_snprintf(s_last_port, sizeof(s_last_port), "%s", port_name);
    return true;
#else
    if (s_serial_fd >= 0) {
        close(s_serial_fd);
        s_serial_fd = -1;
    }

    s_serial_fd = open(port_name, O_RDWR | O_NOCTTY | O_NDELAY);
    if (s_serial_fd < 0) {
        s_is_connected = false;
        return false;
    }

    struct termios options;
    tcgetattr(s_serial_fd, &options);
    speed_t speed = B115200;
    if (baud_rate == 921600) speed = B921600;
    cfsetispeed(&options, speed);
    cfsetospeed(&options, speed);
    options.c_cflag |= (CLOCAL | CREAD | CS8);
    options.c_cflag &= ~(PARENB | CSTOPB | CSIZE);
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    options.c_iflag &= ~(IXON | IXOFF | IXANY);
    options.c_oflag &= ~OPOST;
    tcsetattr(s_serial_fd, TCSANOW, &options);

    s_is_connected = true;
    runtime_snprintf(s_last_port, sizeof(s_last_port), "%s", port_name);
    return true;
#endif
}

void esp32_serial_close(void) {
#if defined(_WIN32) || defined(_WIN64)
    if (s_serial_handle != INVALID_HANDLE_VALUE) {
        CloseHandle(s_serial_handle);
        s_serial_handle = INVALID_HANDLE_VALUE;
    }
#else
    if (s_serial_fd >= 0) {
        close(s_serial_fd);
        s_serial_fd = -1;
    }
#endif
    s_is_connected = false;
}

int esp32_serial_write(const void *buf, size_t len) {
    if (!s_is_connected || !buf || len == 0) return 0;
#if defined(_WIN32) || defined(_WIN64)
    DWORD written = 0;
    if (WriteFile(s_serial_handle, buf, (DWORD)len, &written, NULL)) {
        return (int)written;
    }
    return -1;
#else
    ssize_t res = write(s_serial_fd, buf, len);
    return (int)res;
#endif
}

int esp32_serial_read(void *buf, size_t max_len) {
    if (!s_is_connected || !buf || max_len == 0) return 0;
#if defined(_WIN32) || defined(_WIN64)
    DWORD bytes_read = 0;
    if (ReadFile(s_serial_handle, buf, (DWORD)max_len, &bytes_read, NULL)) {
        return (int)bytes_read;
    }
    return -1;
#else
    ssize_t res = read(s_serial_fd, buf, max_len);
    return (int)res;
#endif
}

const char *esp32_serial_autodetect(void) {
#if defined(_WIN32) || defined(_WIN64)
    static char detected[32];
    for (int i = 1; i <= 32; i++) {
        runtime_snprintf(detected, sizeof(detected), "COM%d", i);
        char test_name[64];
        runtime_snprintf(test_name, sizeof(test_name), "\\\\.\\COM%d", i);
        HANDLE h = CreateFileA(test_name, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
        if (h != INVALID_HANDLE_VALUE) {
            CloseHandle(h);
            return detected;
        }
    }
    return NULL;
#else
    static const char *const candidates[] = {
        "/dev/ttyUSB0", "/dev/ttyUSB1", "/dev/ttyACM0", "/dev/ttyACM1", NULL
    };
    for (int i = 0; candidates[i]; i++) {
        if (access(candidates[i], R_OK | W_OK) == 0) {
            return candidates[i];
        }
    }
    return NULL;
#endif
}
