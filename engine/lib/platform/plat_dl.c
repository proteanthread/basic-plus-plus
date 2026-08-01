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

