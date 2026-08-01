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

/* Threading and Mutex Abstractions */
void platform_mutex_init(BppMutex *mutex) {
    if (!mutex) return;
#if defined(_WIN32)
    mutex->lock = (void *)calloc(1, sizeof(CRITICAL_SECTION));
    if (mutex->lock) {
        InitializeCriticalSection((CRITICAL_SECTION *)mutex->lock);
    }
#elif defined(__linux__) || defined(__APPLE__) || defined(__unix__)
    mutex->lock = (void *)calloc(1, sizeof(pthread_mutex_t));
    if (mutex->lock) {
        pthread_mutex_init((pthread_mutex_t *)mutex->lock, NULL);
    }
#else
    mutex->lock = NULL;
#endif
}

void platform_mutex_lock(BppMutex *mutex) {
    if (!mutex || !mutex->lock) return;
#if defined(_WIN32)
    EnterCriticalSection((CRITICAL_SECTION *)mutex->lock);
#elif defined(__linux__) || defined(__APPLE__) || defined(__unix__)
    pthread_mutex_lock((pthread_mutex_t *)mutex->lock);
#endif
}

void platform_mutex_unlock(BppMutex *mutex) {
    if (!mutex || !mutex->lock) return;
#if defined(_WIN32)
    LeaveCriticalSection((CRITICAL_SECTION *)mutex->lock);
#elif defined(__linux__) || defined(__APPLE__) || defined(__unix__)
    pthread_mutex_unlock((pthread_mutex_t *)mutex->lock);
#endif
}

void platform_mutex_destroy(BppMutex *mutex) {
    if (!mutex || !mutex->lock) return;
#if defined(_WIN32)
    DeleteCriticalSection((CRITICAL_SECTION *)mutex->lock);
#elif defined(__linux__) || defined(__APPLE__) || defined(__unix__)
    pthread_mutex_destroy((pthread_mutex_t *)mutex->lock);
#endif
    free(mutex->lock);
    mutex->lock = NULL;
}

#if defined(_WIN32)
static DWORD WINAPI win32_thread_adapter(LPVOID lpParam) {
    struct {
        void *(*start_routine)(void *);
        void *arg;
    } *args = lpParam;
    void *(*routine)(void *) = args->start_routine;
    void *arg = args->arg;
    free(args);
    routine(arg);
    return 0;
}
#endif

int platform_thread_create(BppThread *thread, void *(*start_routine)(void *), void *arg) {
    if (!thread) return -1;
#if defined(_WIN32)
    struct {
        void *(*start_routine)(void *);
        void *arg;
    } *args = (void *)calloc(1, sizeof(*args));
    if (!args) return -1;
    args->start_routine = start_routine;
    args->arg = arg;
    thread->handle = CreateThread(NULL, 0, win32_thread_adapter, args, 0, &thread->id);
    if (!thread->handle) {
        free(args);
        return -1;
    }
    thread->has_thread = 1;
    return 0;
#elif defined(__linux__) || defined(__APPLE__) || defined(__unix__)
    pthread_t thread_id;
    int rc = pthread_create(&thread_id, NULL, start_routine, arg);
    if (rc == 0) {
        thread->handle = (void *)calloc(1, sizeof(pthread_t));
        if (thread->handle) {
            *(pthread_t *)thread->handle = thread_id;
        }
        thread->has_thread = 1;
        return 0;
    }
    return -1;
#else
    (void)start_routine; (void)arg;
    return -1;
#endif
}

int platform_thread_join(BppThread *thread) {
    if (!thread || !thread->has_thread) return -1;
#if defined(_WIN32)
    if (thread->handle) {
        WaitForSingleObject(thread->handle, INFINITE);
        CloseHandle(thread->handle);
        thread->handle = NULL;
    }
    thread->has_thread = 0;
    return 0;
#elif defined(__linux__) || defined(__APPLE__) || defined(__unix__)
    if (thread->handle) {
        pthread_join(*(pthread_t *)thread->handle, NULL);
        free(thread->handle);
        thread->handle = NULL;
    }
    thread->has_thread = 0;
    return 0;
#else
    return -1;
#endif
}

