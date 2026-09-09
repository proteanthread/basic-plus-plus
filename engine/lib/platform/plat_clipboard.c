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
static char g_fallback_clipboard[65536];
static bool g_has_fallback_clipboard = false;
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
    static char s_clip_result[65536];
    size_t len = strlen(pszText);
    if (len >= sizeof(s_clip_result)) len = sizeof(s_clip_result) - 1;
    memcpy(s_clip_result, pszText, len);
    s_clip_result[len] = '\0';
    GlobalUnlock(hData);
    CloseClipboard();
    return s_clip_result;
#else
    if (g_has_fallback_clipboard) {
        static char s_clip_result[65536];
        size_t len = strlen(g_fallback_clipboard);
        if (len >= sizeof(s_clip_result)) len = sizeof(s_clip_result) - 1;
        memcpy(s_clip_result, g_fallback_clipboard, len);
        s_clip_result[len] = '\0';
        return s_clip_result;
    }
    return NULL;
#endif
}

void platform_clipboard_free(char *text) {
    // Model 1 static buffer: no free required
    (void)text;
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
    if (!text) {
        g_has_fallback_clipboard = false;
        g_fallback_clipboard[0] = '\0';
        return;
    }
    size_t len = strlen(text);
    if (len >= sizeof(g_fallback_clipboard)) len = sizeof(g_fallback_clipboard) - 1;
    memcpy(g_fallback_clipboard, text, len);
    g_fallback_clipboard[len] = '\0';
    g_has_fallback_clipboard = true;
#endif
}

