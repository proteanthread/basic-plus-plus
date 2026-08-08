/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file plat_dl.c
 * @brief Platform component implementation and public API surface for plat_dl.c.
 *
 * WHAT IT DOES:
 * Implements the core responsibilities, data structures, and function evaluation logic for plat_dl.c within the platform subsystem.
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

void *platform_load_library(const char *path) {
#if defined(_WIN32)
    return LoadLibraryA(path);
#elif defined(__linux__) || defined(__APPLE__) || defined(__unix__)
    return dlopen(path, RTLD_LAZY);
#else
    (void)path;
    return NULL;
#endif
}

void *platform_get_proc_address(void *library_handle, const char *name) {
    if (!library_handle) return NULL;
#if defined(_WIN32)
    return (void *)GetProcAddress((HMODULE)library_handle, name);
#elif defined(__linux__) || defined(__APPLE__) || defined(__unix__)
    return dlsym(library_handle, name);
#else
    (void)name;
    return NULL;
#endif
}

void platform_free_library(void *library_handle) {
    if (!library_handle) return;
#if defined(_WIN32)
    FreeLibrary((HMODULE)library_handle);
#elif defined(__linux__) || defined(__APPLE__) || defined(__unix__)
    dlclose(library_handle);
#endif
}

const char *platform_library_last_error(void) {
#if defined(_WIN32)
    return "LoadLibrary failed";
#elif defined(__linux__) || defined(__APPLE__) || defined(__unix__)
    return dlerror();
#else
    return "Not supported";
#endif
}

