// FILENAME: plat_clipboard.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (select.c, string.h)
// NEEDS: libengine (select.h, string.c, time.h, time.c, vm.h)
// NEEDS: libkernel (types.h)
// NEEDS: libplatform (platform.h)
// Provides cross-platform OS abstraction primitives for plat_clipboard.
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

#if !defined(_WIN32)
static char *g_fallback_clipboard = NULL;
#endif

char *platform_clipboard_get(void) {
#if defined(_WIN32)
    if (!OpenClipboard(NULL)) return NULL;
    HANDLE hData = GetClipboardData(CF_TEXT);
    if (!hData) {
        CloseClipboard();
        return NULL;
    }
    char *pszText = (char *)GlobalLock(hData);
    if (!pszText) {
        CloseClipboard();
        return NULL;
    }
    char *result = (char *)calloc(1, strlen(pszText) + 1);
    if (result) {
        strcpy(result, pszText);
    }
    GlobalUnlock(hData);
    CloseClipboard();
    return result;
#else
    if (g_fallback_clipboard) {
        char *result = (char *)calloc(1, strlen(g_fallback_clipboard) + 1);
        if (result) {
            strcpy(result, g_fallback_clipboard);
        }
        return result;
    }
    return NULL;
#endif
}

void platform_clipboard_set(const char *text) {
#if defined(_WIN32)
    if (!text) return;
    if (!OpenClipboard(NULL)) return;
    EmptyClipboard();
    size_t len = strlen(text);
    HGLOBAL hGlob = GlobalAlloc(GMEM_MOVEABLE, len + 1);
    if (!hGlob) {
        CloseClipboard();
        return;
    }
    char *pBuf = (char *)GlobalLock(hGlob);
    if (pBuf) {
        memcpy(pBuf, text, len + 1);
        GlobalUnlock(hGlob);
        SetClipboardData(CF_TEXT, hGlob);
    } else {
        GlobalFree(hGlob);
    }
    CloseClipboard();
#else
    if (g_fallback_clipboard) {
        free(g_fallback_clipboard);
        g_fallback_clipboard = NULL;
    }
    if (text) {
        g_fallback_clipboard = (char *)calloc(1, strlen(text) + 1);
        if (g_fallback_clipboard) {
            strcpy(g_fallback_clipboard, text);
        }
    }
#endif
}

