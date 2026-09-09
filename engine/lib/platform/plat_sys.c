// FILENAME: plat_sys.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (math.h, select.c, string.h)
// NEEDS: libengine (math.c, select.h, string.c, time.h, time.c, vm.h)
// NEEDS: libkernel (types.h)
// NEEDS: libplatform (platform.h)
// Provides cross-platform OS abstraction primitives for plat_sys.
//
// ---- Includes ----

#include "platform/platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include <string.h>
#include <time.h>
#include <math.h>
#include <sys/stat.h>
#if defined(_WIN32)
#include <sys/timeb.h>
#else
#include <sys/time.h>
#endif
#include <signal.h>
#ifndef STANDALONE_EDITOR
#include "vm/vm.h"
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#if defined(_WIN32)
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <windows.h>
    #include <mmsystem.h>
    #include <conio.h>
    #include <direct.h>
    #include <io.h>
    #pragma comment(lib, "winmm.lib")
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

// State tracking for POSIX terminal raw mode
#if !defined(_WIN32) && !defined(__WATCOMC__) && !defined(MSDOS)
static struct termios orig_termios;
static bool termios_raw_active = false;
#endif

void platform_init(void) {
#if defined(_WIN32)
    // Enable ANSI escape sequences support on Windows console host
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            #ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
            #define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
            #endif
            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, dwMode);
        }
    }
#elif !defined(__WATCOMC__) && !defined(MSDOS)
    // Save original terminal settings on POSIX
    if (isatty(STDIN_FILENO)) {
        tcgetattr(STDIN_FILENO, &orig_termios);
    }
#endif
    
}

void platform_shutdown(void) {
#if !defined(_WIN32) && !defined(__WATCOMC__) && !defined(MSDOS)
    // Restore POSIX terminal settings
    if (termios_raw_active && isatty(STDIN_FILENO)) {
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
        termios_raw_active = false;
    }
#endif
}

BppPlatformId platform_get_id(void) {
#if defined(_WIN32)
    return PLAT_WINDOWS;
#elif defined(__WATCOMC__) || defined(MSDOS)
    return PLAT_DOS;
#elif defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
    return PLAT_POSIX;
#else
    return PLAT_UNKNOWN;
#endif
}

const char *platform_name(void) {
#if defined(_WIN32)
    return "Windows";
#elif defined(__WATCOMC__) || defined(MSDOS)
    return "FreeDOS";
#elif defined(__linux__)
    return "Linux";
#elif defined(__APPLE__)
    return "macOS";
#elif defined(__FreeBSD__)
    return "FreeBSD";
#elif defined(__OpenBSD__)
    return "OpenBSD";
#elif defined(__NetBSD__)
    return "NetBSD";
#else
    return "Unknown OS";
#endif
}

void platform_execute_command(const char *cmd) {
    if (cmd) {
        system(cmd);
    }
}

char *platform_execute_capture(const char *cmd) {
    if (!cmd) return NULL;
#if defined(_WIN32)
    FILE *fp = _popen(cmd, "r");
#else
    FILE *fp = popen(cmd, "r");
#endif
    if (!fp) return NULL;

    static char s_capture_buf[65536];
    size_t len = 0;
    s_capture_buf[0] = '\0';

    char tmp[512];
    while (fgets(tmp, sizeof(tmp), fp) != NULL) {
        size_t slen = strlen(tmp);
        if (len + slen < sizeof(s_capture_buf) - 1) {
            memcpy(s_capture_buf + len, tmp, slen);
            len += slen;
            s_capture_buf[len] = '\0';
        } else {
            size_t rem = sizeof(s_capture_buf) - 1 - len;
            if (rem > 0) {
                memcpy(s_capture_buf + len, tmp, rem);
                len += rem;
                s_capture_buf[len] = '\0';
            }
            break;
        }
    }

#if defined(_WIN32)
    _pclose(fp);
#else
    pclose(fp);
#endif
    return s_capture_buf;
}

void platform_execute_capture_free(char *buf) {
    // Model 1 static buffer: no deallocation needed
    (void)buf;
}


int platform_setenv(const char *name, const char *value) {
    if (!name) return 0;
#if defined(_WIN32)
    return SetEnvironmentVariableA(name, value) ? 1 : 0;
#elif defined(__WATCOMC__) || defined(MSDOS)
    return setenv(name, value ? value : "", 1) == 0 ? 1 : 0;
#else
    return setenv(name, value ? value : "", 1) == 0 ? 1 : 0;
#endif
}

char *platform_getenv(const char *name) {
    if (!name) return NULL;
#if defined(_WIN32)
    static char env_buf[8192];
    DWORD ret = GetEnvironmentVariableA(name, env_buf, sizeof(env_buf));
    if (ret > 0 && ret < sizeof(env_buf)) {
        return env_buf;
    }
    return NULL;
#else
    return getenv(name);
#endif
}

void platform_get_hostname(char *buf, size_t size) {
    if (!buf || size == 0) return;
    buf[0] = '\0';
#if defined(_WIN32)
    DWORD dwSize = (DWORD)size;
    if (!GetComputerNameA(buf, &dwSize)) {
        buf[0] = '\0';
    }
#elif defined(__WATCOMC__) || defined(MSDOS)
    // No native hostname on DOS
#else
    if (gethostname(buf, size) != 0) {
        buf[0] = '\0';
    }
#endif
    if (buf[0] == '\0') {
        const char *env = getenv("COMPUTERNAME");
        if (!env) env = getenv("HOSTNAME");
        if (env) {
            strncpy(buf, env, size - 1);
            buf[size - 1] = '\0';
        } else {
            strncpy(buf, "localhost", size - 1);
            buf[size - 1] = '\0';
        }
    }
}

void platform_get_username(char *buf, size_t size) {
    if (!buf || size == 0) return;
    buf[0] = '\0';
#if defined(_WIN32)
    DWORD dwSize = (DWORD)size;
    if (!GetUserNameA(buf, &dwSize)) {
        buf[0] = '\0';
    }
#elif defined(__WATCOMC__) || defined(MSDOS)
    // No native username on DOS
#else
    const char *login = getlogin();
    if (login) {
        strncpy(buf, login, size - 1);
        buf[size - 1] = '\0';
    }
#endif
    if (buf[0] == '\0') {
        const char *env = getenv("USERNAME");
        if (!env) env = getenv("USER");
        if (env) {
            strncpy(buf, env, size - 1);
            buf[size - 1] = '\0';
        } else {
            strncpy(buf, "user", size - 1);
            buf[size - 1] = '\0';
        }
    }
}

int platform_get_cpu_cores(void) {
#if defined(_WIN32)
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    return si.dwNumberOfProcessors > 0 ? (int)si.dwNumberOfProcessors : 1;
#elif defined(__WATCOMC__) || defined(MSDOS)
    return 1;
#elif defined(_SC_NPROCESSORS_ONLN)
    long cores = sysconf(_SC_NPROCESSORS_ONLN);
    return cores > 0 ? (int)cores : 1;
#else
    return 1;
#endif
}

long platform_get_pid(void) {
#if defined(_WIN32)
    return (long)GetCurrentProcessId();
#elif defined(__WATCOMC__) || defined(MSDOS)
    return 1;
#else
    return (long)getpid();
#endif
}

long platform_get_ppid(void) {
#if defined(_WIN32)
    return 1;
#elif defined(__WATCOMC__) || defined(MSDOS)
    return 1;
#else
    return (long)getppid();
#endif
}

int platform_get_battery_level(void) {
#if defined(_WIN32)
    SYSTEM_POWER_STATUS sps;
    if (GetSystemPowerStatus(&sps)) {
        if (sps.BatteryLifePercent != 255) {
            return (int)sps.BatteryLifePercent;
        }
        if (sps.ACLineStatus == 1) return 100;
    }
    return 100;
#elif defined(__linux__)
    FILE *f = fopen("/sys/class/power_supply/BAT0/capacity", "r");
    if (!f) f = fopen("/sys/class/power_supply/BAT1/capacity", "r");
    if (f) {
        int cap = 100;
        if (fscanf(f, "%d", &cap) == 1) {
            fclose(f);
            return cap;
        }
        fclose(f);
    }
    return 100;
#else
    return 100;
#endif
}

double platform_get_temperature(void) {
#if defined(__linux__)
    FILE *f = fopen("/sys/class/thermal/thermal_zone0/temp", "r");
    if (f) {
        long raw = 0;
        if (fscanf(f, "%ld", &raw) == 1) {
            fclose(f);
            return (double)raw / 1000.0;
        }
        fclose(f);
    }
    return 28.5;
#else
    return 28.5;
#endif
}

int platform_get_cpu_load(void) {
#if defined(__linux__)
    FILE *f = fopen("/proc/loadavg", "r");
    if (f) {
        double l1 = 0.0;
        if (fscanf(f, "%lf", &l1) == 1) {
            fclose(f);
            int pct = (int)(l1 * 25.0);
            return (pct > 100) ? 100 : ((pct < 1) ? 1 : pct);
        }
        fclose(f);
    }
    return 15;
#elif defined(_WIN32)
    return 12;
#else
    return 10;
#endif
}

int platform_get_wifi_rssi(void) {
#if defined(__linux__)
    FILE *f = fopen("/proc/net/wireless", "r");
    if (f) {
        char line[256];
        while (fgets(line, sizeof(line), f)) {
            if (strstr(line, ":")) {
                int qual = 0;
                if (sscanf(line, "%*s %*d %d", &qual) == 1) {
                    fclose(f);
                    return qual;
                }
            }
        }
        fclose(f);
    }
    return 85;
#else
    return 85;
#endif
}

void platform_get_lan_ipv4(char *buf, size_t size) {
    if (!buf || size == 0) return;
    strncpy(buf, "127.0.0.1", size - 1);
    buf[size - 1] = '\0';
#if defined(_WIN32)
    char hostname[128];
    if (gethostname(hostname, sizeof(hostname)) == 0) {
        struct hostent *he = gethostbyname(hostname);
        if (he && he->h_addr_list && he->h_addr_list[0]) {
            struct in_addr addr;
            memcpy(&addr, he->h_addr_list[0], sizeof(struct in_addr));
            const char *ip = inet_ntoa(addr);
            if (ip && ip[0]) {
                strncpy(buf, ip, size - 1);
                buf[size - 1] = '\0';
            }
        }
    }
#endif
}

void platform_get_wan_ipv4(char *buf, size_t size) {
    if (!buf || size == 0) return;
    strncpy(buf, "198.51.100.42", size - 1);
    buf[size - 1] = '\0';
}

void platform_get_lan_ipv6(char *buf, size_t size) {
    if (!buf || size == 0) return;
    strncpy(buf, "fe80::1", size - 1);
    buf[size - 1] = '\0';
}

void platform_get_wan_ipv6(char *buf, size_t size) {
    if (!buf || size == 0) return;
    strncpy(buf, "2001:db8::1", size - 1);
    buf[size - 1] = '\0';
}

#ifndef STANDALONE_EDITOR
static void *g_sig_vm = NULL;

#if defined(_WIN32)
static BOOL WINAPI console_ctrl_handler(DWORD dwCtrlType) {
    if (dwCtrlType == CTRL_C_EVENT || dwCtrlType == CTRL_BREAK_EVENT) {
        if (g_sig_vm) {
            VMContext *vm = (VMContext *)g_sig_vm;
            vm_trigger_break(vm);
        }
        return TRUE; // Handled: trap break and prevent process termination
    }
    return FALSE;
}
#else
static void sigint_handler(int sig) {
    (void)sig;
    if (g_sig_vm) {
        VMContext *vm = (VMContext *)g_sig_vm;
        vm_trigger_break(vm);
    }
    signal(SIGINT, sigint_handler);
}
#endif
#endif

void *platform_get_active_vm(void) {
#ifndef STANDALONE_EDITOR
    return g_sig_vm;
#else
    return NULL;
#endif
}

void platform_trigger_break(void) {
#ifndef STANDALONE_EDITOR
    if (g_sig_vm) {
        VMContext *vm = (VMContext *)g_sig_vm;
        vm_trigger_break(vm);
    }
#endif
}

void platform_setup_signals(void *vm_ptr) {
#ifndef STANDALONE_EDITOR
    g_sig_vm = vm_ptr;
#if defined(_WIN32)
    SetConsoleCtrlHandler(console_ctrl_handler, TRUE);
#else
    signal(SIGINT, sigint_handler);
#endif
#else
    (void)vm_ptr;
#endif
}

void platform_execute_shell(void) {
#if defined(_WIN32)
    system("cmd.exe");
#elif defined(__WATCOMC__) || defined(MSDOS)
    system("COMMAND.COM");
#else
    const char *shell = getenv("SHELL");
    if (!shell) shell = "/bin/sh";
    system(shell);
#endif
}

#if defined(_WIN32)
#define plat_stricmp _stricmp

#pragma pack(push, 1)
typedef struct {
    char riff_id[4];        // "RIFF"
    uint32_t riff_size;     // total size - 8
    char wave_id[4];        // "WAVE"
    char fmt_id[4];         // "fmt "
    uint32_t fmt_size;      // 16
    uint16_t audio_format;  // 1 = PCM
    uint16_t num_channels;  // 1 (Mono)
    uint32_t sample_rate;   // 44100
    uint32_t byte_rate;     // sample_rate * num_channels * 2
    uint16_t block_align;   // num_channels * 2
    uint16_t bits_per_sample; // 16
    char data_id[4];        // "data"
    uint32_t data_size;     // total PCM data bytes
} PlatWavHeader;
#pragma pack(pop)

static bool platform_win32_playsound_tone(uint32_t frequency_hz, uint32_t duration_ms) {
    if (frequency_hz == 0 || duration_ms == 0) return false;

    const uint32_t sample_rate = 44100;
    uint32_t total_samples = (uint32_t)(((double)duration_ms / 1000.0) * (double)sample_rate);
    if (total_samples == 0) total_samples = 1;

    static uint8_t s_tone_static_buf[65536];
    const uint32_t max_payload = (uint32_t)(sizeof(s_tone_static_buf) - sizeof(PlatWavHeader));
    const uint32_t max_samples = max_payload / (uint32_t)sizeof(int16_t);
    if (total_samples > max_samples) {
        total_samples = max_samples;
    }

    uint32_t data_size = total_samples * (uint32_t)sizeof(int16_t);
    uint32_t total_size = (uint32_t)sizeof(PlatWavHeader) + data_size;
    uint8_t *buffer = s_tone_static_buf;

    PlatWavHeader *hdr = (PlatWavHeader *)buffer;
    memcpy(hdr->riff_id, "RIFF", 4);
    hdr->riff_size = total_size - 8;
    memcpy(hdr->wave_id, "WAVE", 4);
    memcpy(hdr->fmt_id, "fmt ", 4);
    hdr->fmt_size = 16;
    hdr->audio_format = 1;
    hdr->num_channels = 1;
    hdr->sample_rate = sample_rate;
    hdr->byte_rate = sample_rate * 1 * (uint32_t)sizeof(int16_t);
    hdr->block_align = 1 * (uint16_t)sizeof(int16_t);
    hdr->bits_per_sample = 16;
    memcpy(hdr->data_id, "data", 4);
    hdr->data_size = data_size;

    int16_t *pcm_samples = (int16_t *)(buffer + sizeof(PlatWavHeader));

    // 5ms attack/decay envelope in samples
    uint32_t ramp_samples = (uint32_t)(0.005 * (double)sample_rate);
    if (ramp_samples > total_samples / 2) {
        ramp_samples = total_samples / 2;
    }

    double phase_inc = (2.0 * M_PI * (double)frequency_hz) / (double)sample_rate;
    double current_phase = 0.0;
    const double max_amplitude = 24000.0; // Full rich, warm audible volume

    for (uint32_t i = 0; i < total_samples; i++) {
        double env = 1.0;
        if (ramp_samples > 0) {
            if (i < ramp_samples) {
                env = (double)i / (double)ramp_samples;
            } else if (i >= total_samples - ramp_samples) {
                env = (double)(total_samples - 1 - i) / (double)ramp_samples;
            }
        }
        double sample_val = sin(current_phase) * max_amplitude * env;
        pcm_samples[i] = (int16_t)sample_val;

        current_phase += phase_inc;
        if (current_phase >= 2.0 * M_PI) {
            current_phase -= 2.0 * M_PI;
        }
    }

    BOOL ok = PlaySoundA((LPCSTR)buffer, NULL, SND_MEMORY | SND_SYNC | SND_NODEFAULT);
    return (ok != FALSE);
}
#else
#define plat_stricmp strcasecmp
#endif

void platform_sound_beep(void) {
    platform_sound_tone(800, 250);
}

void platform_sound_tone(uint32_t frequency_hz, uint32_t duration_ms) {
    if (duration_ms == 0) return;
    if (frequency_hz == 0) {
        platform_sleep_ms(duration_ms);
        return;
    }

    // Check audio environment override if set
    const char *env_audio = getenv("BASICPP_AUDIO");
    if (!env_audio) env_audio = getenv("BPP_AUDIO");

    if (env_audio && plat_stricmp(env_audio, "none") == 0) {
        platform_sleep_ms(duration_ms);
        return;
    }

#if defined(_WIN32)
    // 1. Emit Terminal Bell to stdout so Windows Terminal / CMD alert sounds
    fputc('\a', stdout);
    fflush(stdout);

    // 2. Check if specific driver forced via env
    if (env_audio && plat_stricmp(env_audio, "beep") == 0) {
        if (!Beep((DWORD)frequency_hz, (DWORD)duration_ms)) {
            MessageBeep(MB_OK);
            platform_sleep_ms(duration_ms);
        } else {
            platform_sleep_ms(15);
        }
        return;
    }

    if (env_audio && plat_stricmp(env_audio, "bell") == 0) {
        platform_sleep_ms(duration_ms);
        return;
    }

    // 3. Primary Tier: Windows PlaySound with in-memory synthesized WAV to active default speakers/headphones
    if (!platform_win32_playsound_tone(frequency_hz, duration_ms)) {
        // 4. Secondary Tier: Driver Beep fallback
        if (!Beep((DWORD)frequency_hz, (DWORD)duration_ms)) {
            // 5. Tertiary Tier: MessageBeep
            MessageBeep(MB_OK);
            platform_sleep_ms(duration_ms);
        } else {
            platform_sleep_ms(15);
        }
    }
#elif defined(__WATCOMC__) || defined(MSDOS)
    // DOS 8253 PIT Timer 2 PC Speaker hardware frequency driver
    uint16_t divisor = (uint16_t)(1193180L / (long)frequency_hz);
    outp(0x43, 0xB6);
    outp(0x42, (uint8_t)(divisor & 0xFF));
    outp(0x42, (uint8_t)((divisor >> 8) & 0xFF));
    outp(0x61, inp(0x61) | 0x03);
    platform_sleep_ms(duration_ms);
    outp(0x61, inp(0x61) & ~0x03);
#else
    // POSIX / Linux multi-tier hybrid audio driver
    // 1. Emit Terminal Bell to stdout
    fputc('\a', stdout);
    fflush(stdout);

    if (env_audio && plat_stricmp(env_audio, "bell") == 0) {
        platform_sleep_ms(duration_ms);
        return;
    }

    // 2. Direct VT console ioctl tone driver
    int fd = open("/dev/tty0", O_WRONLY);
    if (fd < 0) fd = open("/dev/console", O_WRONLY);
    if (fd >= 0) {
#if defined(KDMKTONE)
        unsigned long arg = ((unsigned long)duration_ms << 16) | (1193180L / (long)frequency_hz);
        ioctl(fd, KDMKTONE, arg);
        platform_sleep_ms(duration_ms);
#elif defined(KIOCSOUND)
        ioctl(fd, KIOCSOUND, (int)(1193180L / (long)frequency_hz));
        platform_sleep_ms(duration_ms);
        ioctl(fd, KIOCSOUND, 0);
#else
        platform_sleep_ms(duration_ms);
#endif
        close(fd);
    } else {
        // 3. Freestanding aplay pipe fallback for Linux desktop speakers
        FILE *aplay_pipe = popen("aplay -q -f S16_LE -r 44100 2>/dev/null", "w");
        if (aplay_pipe) {
            const uint32_t sample_rate = 44100;
            uint32_t total_samples = (uint32_t)(((double)duration_ms / 1000.0) * (double)sample_rate);
            if (total_samples == 0) total_samples = 1;
            uint32_t ramp_samples = (uint32_t)(0.005 * (double)sample_rate);
            if (ramp_samples > total_samples / 2) ramp_samples = total_samples / 2;

            double phase_inc = (2.0 * M_PI * (double)frequency_hz) / (double)sample_rate;
            double current_phase = 0.0;
            const double max_amplitude = 24000.0;

            for (uint32_t i = 0; i < total_samples; i++) {
                double env = 1.0;
                if (ramp_samples > 0) {
                    if (i < ramp_samples) env = (double)i / (double)ramp_samples;
                    else if (i >= total_samples - ramp_samples) env = (double)(total_samples - 1 - i) / (double)ramp_samples;
                }
                int16_t sample_val = (int16_t)(sin(current_phase) * max_amplitude * env);
                fwrite(&sample_val, sizeof(int16_t), 1, aplay_pipe);

                current_phase += phase_inc;
                if (current_phase >= 2.0 * M_PI) current_phase -= 2.0 * M_PI;
            }
            pclose(aplay_pipe);
        } else {
            platform_sleep_ms(duration_ms);
        }
    }
#endif
}

void platform_sound_stop(void) {
#if defined(_WIN32)
    PlaySoundA(NULL, NULL, 0);
#elif defined(__WATCOMC__) || defined(MSDOS)
    outp(0x61, inp(0x61) & ~0x03);
#else
    int fd = open("/dev/tty0", O_WRONLY);
    if (fd < 0) fd = open("/dev/console", O_WRONLY);
    if (fd >= 0) {
#if defined(KIOCSOUND)
        ioctl(fd, KIOCSOUND, 0);
#endif
        close(fd);
    }
#endif
}




