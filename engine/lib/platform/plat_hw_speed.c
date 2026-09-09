// FILENAME: plat_hw_speed.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libhardware, libengine
// Platform hardware speed and clock probing implementation.

#include "platform/plat_hw_speed.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if defined(_WIN32)
    #include <windows.h>
    #if defined(_MSC_VER) || defined(__INTEL_COMPILER)
        #include <intrin.h>
    #endif
#elif defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
    #include <unistd.h>
    #include <sys/time.h>
    #if defined(__APPLE__)
        #include <sys/types.h>
        #include <sys/sysctl.h>
    #endif
    #if defined(__x86_64__) || defined(__i386__)
        #include <x86intrin.h>
    #endif
#endif

// Static cached host clock rates
static double s_cpu_hz = 0.0;
static double s_bus_hz = 100000000.0;   // Default 100 MHz reference bus clock
static double s_mem_hz = 3200000000.0;  // Default 3200 MT/s DDR
static double s_gpu_hz = 1500000000.0;  // Default 1500 MHz GPU core
static bool s_calibrated = false;

uint64_t plat_hw_get_cycle_count(void) {
#if defined(_MSC_VER) && (defined(_M_IX86) || defined(_M_X64))
    return (uint64_t)__rdtsc();
#elif (defined(__GNUC__) || defined(__clang__)) && (defined(__i386__) || defined(__x86_64__))
    return (uint64_t)__builtin_ia32_rdtsc();
#elif defined(_WIN32)
    LARGE_INTEGER count;
    if (QueryPerformanceCounter(&count)) {
        return (uint64_t)count.QuadPart;
    }
    return (uint64_t)GetTickCount();
#elif defined(CLOCK_MONOTONIC_RAW) || defined(CLOCK_MONOTONIC)
    struct timespec ts;
    #if defined(CLOCK_MONOTONIC_RAW)
    if (clock_gettime(CLOCK_MONOTONIC_RAW, &ts) == 0)
    #else
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0)
    #endif
    {
        return ((uint64_t)ts.tv_sec * 1000000000ULL) + (uint64_t)ts.tv_nsec;
    }
    return 0;
#else
    return (uint64_t)clock();
#endif
}

static double probe_system_cpu_hz(void) {
#if defined(_WIN32)
    // Method 1: Check Windows registry for CPU ~MHz
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                      "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
                      0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        DWORD mhz = 0;
        DWORD size = sizeof(mhz);
        if (RegQueryValueExA(hKey, "~MHz", NULL, NULL, (LPBYTE)&mhz, &size) == ERROR_SUCCESS && mhz > 0) {
            RegCloseKey(hKey);
            return (double)mhz * 1000000.0;
        }
        RegCloseKey(hKey);
    }

    // Method 2: Calibrate using QueryPerformanceCounter and RDTSC
    LARGE_INTEGER qpc_freq, t0, t1;
    if (QueryPerformanceFrequency(&qpc_freq) && qpc_freq.QuadPart > 0) {
        uint64_t r0 = plat_hw_get_cycle_count();
        QueryPerformanceCounter(&t0);
        // Spin for ~5 milliseconds
        LONGLONG wait_ticks = (qpc_freq.QuadPart * 5) / 1000;
        do {
            QueryPerformanceCounter(&t1);
        } while ((t1.QuadPart - t0.QuadPart) < wait_ticks);
        uint64_t r1 = plat_hw_get_cycle_count();

        double elapsed_sec = (double)(t1.QuadPart - t0.QuadPart) / (double)qpc_freq.QuadPart;
        if (elapsed_sec > 0.001 && r1 > r0) {
            double measured_hz = (double)(r1 - r0) / elapsed_sec;
            if (measured_hz >= 1000000.0 && measured_hz <= 100000000000.0) {
                return measured_hz;
            }
        }
    }
    return 3000000000.0; // 3.0 GHz sensible x86_64 baseline

#elif defined(__APPLE__)
    uint64_t freq = 0;
    size_t size = sizeof(freq);
    if (sysctlbyname("hw.cpufrequency", &freq, &size, NULL, 0) == 0 && freq > 0) {
        return (double)freq;
    }
    return 2400000000.0;

#elif defined(__linux__)
    // Try reading scaling_cur_freq or cpuinfo_max_freq
    FILE *fp = fopen("/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq", "r");
    if (!fp) fp = fopen("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq", "r");
    if (fp) {
        long khz = 0;
        if (fscanf(fp, "%ld", &khz) == 1 && khz > 0) {
            fclose(fp);
            return (double)khz * 1000.0;
        }
        fclose(fp);
    }

    // Fallback: parse /proc/cpuinfo for cpu MHz
    fp = fopen("/proc/cpuinfo", "r");
    if (fp) {
        char line[256];
        while (fgets(line, sizeof(line), fp)) {
            if (strncmp(line, "cpu MHz", 7) == 0) {
                char *colon = strchr(line, ':');
                if (colon) {
                    double mhz = atof(colon + 1);
                    if (mhz > 0.0) {
                        fclose(fp);
                        return mhz * 1000000.0;
                    }
                }
            }
        }
        fclose(fp);
    }
    return 2500000000.0;

#else
    // Generic fallback
    return 2000000000.0;
#endif
}

void plat_hw_calibrate(void) {
    if (s_calibrated) return;
    s_cpu_hz = probe_system_cpu_hz();
    s_calibrated = true;
}

double plat_hw_get_cpu_hz(void) {
    if (!s_calibrated || s_cpu_hz <= 0.0) {
        plat_hw_calibrate();
    }
    return s_cpu_hz;
}

double plat_hw_get_bus_hz(void) {
    return s_bus_hz;
}

double plat_hw_get_mem_hz(void) {
    return s_mem_hz;
}

double plat_hw_get_gpu_hz(void) {
    return s_gpu_hz;
}

void plat_hw_set_cpu_hz(double hz) {
    if (hz > 0.0) {
        s_cpu_hz = hz;
        s_calibrated = true;
    }
}

void plat_hw_set_bus_hz(double hz) {
    if (hz > 0.0) s_bus_hz = hz;
}

void plat_hw_set_mem_hz(double hz) {
    if (hz > 0.0) s_mem_hz = hz;
}

void plat_hw_set_gpu_hz(double hz) {
    if (hz > 0.0) s_gpu_hz = hz;
}

void plat_hw_delay_us(uint64_t us) {
    if (us == 0) return;

#if defined(_WIN32)
    LARGE_INTEGER freq, start, now;
    if (!QueryPerformanceFrequency(&freq) || freq.QuadPart == 0) {
        if (us >= 1000) Sleep((DWORD)(us / 1000));
        return;
    }

    LONGLONG ticks_needed = (LONGLONG)((us * (uint64_t)freq.QuadPart) / 1000000ULL);
    QueryPerformanceCounter(&start);

    // If wait duration is long (> 15ms), sleep the bulk to yield CPU
    if (us >= 16000) {
        DWORD ms = (DWORD)((us - 2000) / 1000);
        Sleep(ms);
    }

    // Spin-wait the high precision remainder
    do {
        QueryPerformanceCounter(&now);
    } while ((now.QuadPart - start.QuadPart) < ticks_needed);

#elif defined(CLOCK_MONOTONIC)
    struct timespec start, now;
    clock_gettime(CLOCK_MONOTONIC, &start);
    uint64_t ns_needed = us * 1000ULL;

    if (us >= 20000) {
        struct timespec sleep_ts;
        sleep_ts.tv_sec = (time_t)((us - 5000) / 1000000ULL);
        sleep_ts.tv_nsec = (long)(((us - 5000) % 1000000ULL) * 1000ULL);
        nanosleep(&sleep_ts, NULL);
    }

    do {
        clock_gettime(CLOCK_MONOTONIC, &now);
        uint64_t elapsed_ns = (uint64_t)(now.tv_sec - start.tv_sec) * 1000000000ULL +
                              (uint64_t)(now.tv_nsec - start.tv_nsec);
        if (elapsed_ns >= ns_needed) break;
    } while (1);
#else
    // Fallback: simple sleep
    if (us >= 1000) {
        platform_sleep_ms((uint32_t)(us / 1000));
    }
#endif
}
