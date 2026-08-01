/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */
#include "bpp_platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#if defined(_WIN32)
#include <sys/timeb.h>
#else
#include <sys/time.h>
#endif
#include <signal.h>
#ifndef STANDALONE_EDITOR
#include "bpp_vm.h"
#endif

#if defined(_WIN32)
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <windows.h>
    #include <conio.h>
    #include <direct.h>
    #include <io.h>
#elif defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
    #include <unistd.h>
    #include <termios.h>
    #include <sys/select.h>
    #include <sys/ioctl.h>
    #include <sys/stat.h>
    #include <sys/socket.h>
    #include <sys/types.h>
    #include <netdb.h>
    #include <arpa/inet.h>
    #include <dirent.h>
    #include <fcntl.h>
    #include <dlfcn.h>
    #include <errno.h>
    #include <pthread.h>
    #ifndef STANDALONE_EDITOR
        #include <ncurses.h>
    #endif
#elif defined(__WATCOMC__) || defined(MSDOS)
    #include <dos.h>
    #include <conio.h>
    #include <direct.h>
#endif

static double g_boot_time = -1.0;

static double platform_get_epoch_time(void) {
    double current = 0.0;
    time_t t = time(NULL);
#if defined(_WIN32)
    struct _timeb tb;
    _ftime_s(&tb);
    current = (double)t + tb.millitm / 1000.0;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    current = (double)t + tv.tv_usec / 1000000.0;
#endif
    return current;
}

void platform_sleep_ms(uint32_t ms) {
#if defined(_WIN32)
    Sleep(ms);
#elif defined(__WATCOMC__) || defined(MSDOS)
    /* DOS delay loop using clock ticks */
    delay(ms);
#else
    usleep(ms * 1000);
#endif
}

struct tm *platform_localtime(const time_t *timep, struct tm *result) {
    if (!timep || !result) return NULL;
#if defined(_WIN32)
    if (localtime_s(result, timep) == 0) {
        return result;
    }
    return NULL;
#else
    return localtime_r(timep, result);
#endif
}

struct tm *platform_gmtime(const time_t *timep, struct tm *result) {
    if (!timep || !result) return NULL;
#if defined(_WIN32)
    if (gmtime_s(result, timep) == 0) {
        return result;
    }
    return NULL;
#else
    return gmtime_r(timep, result);
#endif
}


double platform_get_timer(void) {
#if defined(_WIN32)
    struct _timeb tb;
    _ftime_s(&tb);
    time_t t = tb.time;
    struct tm lt_buf;
    struct tm *lt = platform_localtime(&t, &lt_buf);
    double sec = 0.0;
    if (lt) {
        sec = lt->tm_hour * 3600.0 + lt->tm_min * 60.0 + lt->tm_sec;
    }
    sec += tb.millitm / 1000.0;
    return sec;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    time_t t = tv.tv_sec;
    struct tm lt_buf;
    struct tm *lt = platform_localtime(&t, &lt_buf);
    double sec = 0.0;
    if (lt) {
        sec = lt->tm_hour * 3600.0 + lt->tm_min * 60.0 + lt->tm_sec;
    }
    sec += tv.tv_usec / 1000000.0;
    return sec;
#endif
}

double platform_get_uptime(void) {
    double current = platform_get_epoch_time();
    if (g_boot_time < 0.0) {
        g_boot_time = current;
    }
    return current - g_boot_time;
}

