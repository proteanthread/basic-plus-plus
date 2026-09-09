// FILENAME: plat_thread.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (select.c, string.h)
// NEEDS: libengine (select.h, string.c, time.h, time.c, vm.h)
// NEEDS: libkernel (types.h)
// NEEDS: libplatform (platform.h)
// Provides cross-platform OS abstraction primitives for plat_thread.
//
// ---- Includes ----

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

// Threading and Mutex Abstractions (Model 1 & 3: Pool Allocator)
#define MAX_PLATFORM_MUTEXES 16

#if defined(_WIN32)
static CRITICAL_SECTION s_cs_pool[MAX_PLATFORM_MUTEXES];
#elif defined(__linux__) || defined(__APPLE__) || defined(__unix__)
static pthread_mutex_t s_pm_pool[MAX_PLATFORM_MUTEXES];
#endif
static int s_mutex_used[MAX_PLATFORM_MUTEXES];

void platform_mutex_init(BppMutex *mutex) {
    if (!mutex) return;
    mutex->lock = NULL;
    for (int i = 0; i < MAX_PLATFORM_MUTEXES; i++) {
        if (!s_mutex_used[i]) {
            s_mutex_used[i] = 1;
#if defined(_WIN32)
            InitializeCriticalSection(&s_cs_pool[i]);
            mutex->lock = (void *)&s_cs_pool[i];
#elif defined(__linux__) || defined(__APPLE__) || defined(__unix__)
            pthread_mutex_init(&s_pm_pool[i], NULL);
            mutex->lock = (void *)&s_pm_pool[i];
#endif
            return;
        }
    }
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
    for (int i = 0; i < MAX_PLATFORM_MUTEXES; i++) {
        if (&s_cs_pool[i] == (CRITICAL_SECTION *)mutex->lock) {
            s_mutex_used[i] = 0;
            break;
        }
    }
#elif defined(__linux__) || defined(__APPLE__) || defined(__unix__)
    pthread_mutex_destroy((pthread_mutex_t *)mutex->lock);
    for (int i = 0; i < MAX_PLATFORM_MUTEXES; i++) {
        if (&s_pm_pool[i] == (pthread_mutex_t *)mutex->lock) {
            s_mutex_used[i] = 0;
            break;
        }
    }
#endif
    mutex->lock = NULL;
}

#define MAX_PLATFORM_THREADS 8

typedef struct {
    void *(*start_routine)(void *);
    void *arg;
    int in_use;
#if defined(__linux__) || defined(__APPLE__) || defined(__unix__)
    pthread_t pth;
#endif
} PlatThreadSlot;

static PlatThreadSlot s_thread_pool[MAX_PLATFORM_THREADS];

#if defined(_WIN32)
static DWORD WINAPI win32_thread_adapter(LPVOID lpParam) {
    PlatThreadSlot *slot = (PlatThreadSlot *)lpParam;
    if (!slot) return 0;
    void *(*routine)(void *) = slot->start_routine;
    void *arg = slot->arg;
    slot->in_use = 0;
    if (routine) {
        routine(arg);
    }
    return 0;
}
#endif

int platform_thread_create(BppThread *thread, void *(*start_routine)(void *), void *arg) {
    if (!thread) return -1;
    thread->handle = NULL;
    thread->has_thread = 0;

    int slot_idx = -1;
    for (int i = 0; i < MAX_PLATFORM_THREADS; i++) {
        if (!s_thread_pool[i].in_use) {
            s_thread_pool[i].in_use = 1;
            slot_idx = i;
            break;
        }
    }
    if (slot_idx < 0) return -1;

    s_thread_pool[slot_idx].start_routine = start_routine;
    s_thread_pool[slot_idx].arg = arg;

#if defined(_WIN32)
    thread->handle = CreateThread(NULL, 0, win32_thread_adapter, &s_thread_pool[slot_idx], 0, &thread->id);
    if (!thread->handle) {
        s_thread_pool[slot_idx].in_use = 0;
        return -1;
    }
    thread->has_thread = 1;
    return 0;
#elif defined(__linux__) || defined(__APPLE__) || defined(__unix__)
    int rc = pthread_create(&s_thread_pool[slot_idx].pth, NULL, start_routine, arg);
    if (rc == 0) {
        thread->handle = (void *)&s_thread_pool[slot_idx].pth;
        thread->has_thread = 1;
        return 0;
    }
    s_thread_pool[slot_idx].in_use = 0;
    return -1;
#else
    s_thread_pool[slot_idx].in_use = 0;
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
        pthread_t *pth = (pthread_t *)thread->handle;
        pthread_join(*pth, NULL);
        for (int i = 0; i < MAX_PLATFORM_THREADS; i++) {
            if (&s_thread_pool[i].pth == pth) {
                s_thread_pool[i].in_use = 0;
                break;
            }
        }
        thread->handle = NULL;
    }
    thread->has_thread = 0;
    return 0;
#else
    return -1;
#endif
}

