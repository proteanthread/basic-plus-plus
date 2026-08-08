/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file plat_regex.c
 * @brief Platform component implementation and public API surface for plat_regex.c.
 *
 * WHAT IT DOES:
 * Implements the core responsibilities, data structures, and function evaluation logic for plat_regex.c within the platform subsystem.
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

/* Tiny Regex Engine and Helper Functions */
static int match_char(char c, char pattern_char) {
    if (pattern_char == '.') return 1;
    return (c == pattern_char);
}

static int match_pattern(const char *pattern, const char *text, const char **match_end) {
    if (pattern[0] == '\0') {
        *match_end = text;
        return 1;
    }
    if (pattern[0] == '$' && pattern[1] == '\0') {
        if (*text == '\0') {
            *match_end = text;
            return 1;
        }
        return 0;
    }
    if (pattern[1] == '*') {
        while (*text != '\0' && match_char(*text, pattern[0])) {
            if (match_pattern(pattern + 2, text, match_end)) {
                return 1;
            }
            text++;
        }
        return match_pattern(pattern + 2, text, match_end);
    }
    if (pattern[1] == '+') {
        if (*text == '\0' || !match_char(*text, pattern[0])) return 0;
        text++;
        while (*text != '\0' && match_char(*text, pattern[0])) {
            if (match_pattern(pattern + 2, text, match_end)) {
                return 1;
            }
            text++;
        }
        return match_pattern(pattern + 2, text, match_end);
    }
    if (pattern[1] == '?') {
        if (*text != '\0' && match_char(*text, pattern[0])) {
            if (match_pattern(pattern + 2, text + 1, match_end)) return 1;
        }
        return match_pattern(pattern + 2, text, match_end);
    }
    if (*text != '\0' && match_char(*text, pattern[0])) {
        return match_pattern(pattern + 1, text + 1, match_end);
    }
    return 0;
}

static int platform_regex_search(const char *text, const char *pattern, int *match_length) {
    if (!text || !pattern) return -1;
    if (pattern[0] == '^') {
        const char *match_end = NULL;
        if (match_pattern(pattern + 1, text, &match_end)) {
            if (match_length) *match_length = (int)(match_end - text);
            return 0;
        }
        return -1;
    }
    for (int i = 0; text[i] != '\0'; i++) {
        const char *match_end = NULL;
        if (match_pattern(pattern, text + i, &match_end)) {
            if (match_length) *match_length = (int)(match_end - (text + i));
            return i;
        }
    }
    return -1;
}

int platform_regex_match(const char *text, const char *pattern) {
    int match_len = 0;
    return platform_regex_search(text, pattern, &match_len) >= 0;
}

char *platform_regex_replace(const char *text, const char *pattern, const char *replacement) {
    if (!text || !pattern || !replacement) return NULL;
    
    size_t out_cap = strlen(text) * 2 + strlen(replacement) + 256;
    char *out = (char *)calloc(1, out_cap);
    if (!out) return NULL;
    
    size_t out_len = 0;
    const char *p = text;
    
    while (*p != '\0') {
        int match_len = 0;
        int idx = platform_regex_search(p, pattern, &match_len);
        if (idx < 0) {
            size_t rem = strlen(p);
            if (out_len + rem >= out_cap) {
                out_cap = out_len + rem + 256;
                char *new_out = realloc(out, out_cap);
                if (!new_out) { free(out); return NULL; }
                out = new_out;
            }
            memcpy(out + out_len, p, rem);
            out_len += rem;
            break;
        }
        
        if (idx > 0) {
            if (out_len + idx >= out_cap) {
                out_cap = out_len + idx + 256;
                char *new_out = realloc(out, out_cap);
                if (!new_out) { free(out); return NULL; }
                out = new_out;
            }
            memcpy(out + out_len, p, idx);
            out_len += idx;
        }
        
        size_t rep_len = strlen(replacement);
        if (out_len + rep_len >= out_cap) {
            out_cap = out_len + rep_len + 256;
            char *new_out = realloc(out, out_cap);
            if (!new_out) { free(out); return NULL; }
            out = new_out;
        }
        memcpy(out + out_len, replacement, rep_len);
        out_len += rep_len;
        
        p += idx + match_len;
        
        if (match_len == 0) {
            if (*p != '\0') {
                if (out_len + 1 >= out_cap) {
                    out_cap += 256;
                    char *new_out = realloc(out, out_cap);
                    if (!new_out) { free(out); return NULL; }
                    out = new_out;
                }
                out[out_len++] = *p;
                p++;
            }
        }
    }
    
    out[out_len] = '\0';
    return out;
}

