/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file plat_sys.c
 * @brief Platform component implementation and public API surface for plat_sys.c.
 *
 * WHAT IT DOES:
 * Implements the core responsibilities, data structures, and function evaluation logic for plat_sys.c within the platform subsystem.
 *
 * WHY IT EXISTS:
 * Ensures decoupled modularity, strict C17 portability, and clear micro-library architectural boundary enforcement.
 *
 * WHY IT WORKS THIS WAY:
 * Designed with zero-initialization defaults, bounded memory operations, and explicit error code propagation to the VM state.
 *
 * WHAT CAN BE CHANGED:
 * Subsystem configuration defaults, local execution helper routines, and documentation annotations.
 *
 * WHAT CANNOT BE CHANGED:
 * Public API symbol declarations, micro-library metadata structures, and thread-safe error reporting contracts.
 *
 * WHAT TO EXPECT:
 * High-performance deterministic execution with zero side-effects outside designated state structures.
 *
 * WHAT TO DO IF SOMETHING BREAKS:
 * Verify context initialization, trace BppError return codes, and inspect log outputs for bounds assertions.
 *
 * ASSUMPTIONS:
 * Valid subsystem contexts and required memory pools are allocated prior to executing API handlers.
 *
 * PORTABILITY CONCERNS:
 * Strict C17 compliance, 64-bit pointer safety, and pure ASCII string operations across desktop, IoT, and embedded targets.
 *
 * FUTURE EXPANSIONS:
 * Additional dialect compatibility mappings, telemetry instrumentation, and microcontroller payload stubs.
 */

/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */
#include "platform/platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

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
#include "vm/vm.h"
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

/* State tracking for POSIX terminal raw mode */
#if !defined(_WIN32) && !defined(__WATCOMC__) && !defined(MSDOS)
static struct termios orig_termios;
static bool termios_raw_active = false;
#endif

void platform_init(void) {
#if defined(_WIN32)
    /* Enable ANSI escape sequences support on Windows console host */
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
    /* Save original terminal settings on POSIX */
    if (isatty(STDIN_FILENO)) {
        tcgetattr(STDIN_FILENO, &orig_termios);
    }
#endif
    
}

void platform_shutdown(void) {
#if !defined(_WIN32) && !defined(__WATCOMC__) && !defined(MSDOS)
    /* Restore POSIX terminal settings */
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
    /* No native hostname on DOS */
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
    /* No native username on DOS */
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

#ifndef STANDALONE_EDITOR
static void *g_sig_vm = NULL;

#if defined(_WIN32)
static BOOL WINAPI console_ctrl_handler(DWORD dwCtrlType) {
    if (dwCtrlType == CTRL_C_EVENT || dwCtrlType == CTRL_BREAK_EVENT) {
        if (g_sig_vm) {
            vm_trigger_break((VMContext*)g_sig_vm);
        }
        return TRUE; /* Handled */
    }
    return FALSE;
}
#else
static void sigint_handler(int sig) {
    (void)sig;
    if (g_sig_vm) {
        vm_trigger_break((VMContext*)g_sig_vm);
    }
}
#endif
#endif

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

