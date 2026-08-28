// FILENAME: strops.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot, libcore, libengine, libhardware, libkernel, libserver, libstandard
// NEEDS: libcore (ctype.h, ctype.c, memops.h, memops.c, strops.h)
// Freestanding string search and manipulation implementation.
//
// ---- Includes ----

#include "runtime/string/strops.h"
#include "runtime/string/memops.h"
#include "runtime/ctype/ctype.h"

size_t runtime_strlen(const char *s) {
    if (!s) {
        return 0;
    }
    const char *p = s;
    while (*p) {
        p++;
    }
    return (size_t)(p - s);
}

size_t runtime_strnlen(const char *s, size_t maxlen) {
    if (!s) {
        return 0;
    }
    size_t len = 0;
    while (len < maxlen && s[len]) {
        len++;
    }
    return len;
}

int runtime_strcmp(const char *s1, const char *s2) {
    if (s1 == s2) return 0;
    if (!s1) return -1;
    if (!s2) return 1;

    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return (int)(unsigned char)*s1 - (int)(unsigned char)*s2;
}

int runtime_strncmp(const char *s1, const char *s2, size_t n) {
    if (n == 0) return 0;
    if (s1 == s2) return 0;
    if (!s1) return -1;
    if (!s2) return 1;

    while (n > 1 && *s1 && (*s1 == *s2)) {
        s1++;
        s2++;
        n--;
    }
    return (int)(unsigned char)*s1 - (int)(unsigned char)*s2;
}

int runtime_strcasecmp(const char *s1, const char *s2) {
    if (s1 == s2) return 0;
    if (!s1) return -1;
    if (!s2) return 1;

    while (*s1 && (runtime_tolower((unsigned char)*s1) == runtime_tolower((unsigned char)*s2))) {
        s1++;
        s2++;
    }
    return (int)runtime_tolower((unsigned char)*s1) - (int)runtime_tolower((unsigned char)*s2);
}

int runtime_strncasecmp(const char *s1, const char *s2, size_t n) {
    if (n == 0) return 0;
    if (s1 == s2) return 0;
    if (!s1) return -1;
    if (!s2) return 1;

    while (n > 1 && *s1 && (runtime_tolower((unsigned char)*s1) == runtime_tolower((unsigned char)*s2))) {
        s1++;
        s2++;
        n--;
    }
    return (int)runtime_tolower((unsigned char)*s1) - (int)runtime_tolower((unsigned char)*s2);
}

char *runtime_strcpy(char *dest, const char *src) {
    if (!dest || !src) return dest;
    char *d = dest;
    while ((*d++ = *src++) != '\0') {
        // copy loop
    }
    return dest;
}

char *runtime_strncpy(char *dest, const char *src, size_t n) {
    if (!dest || !src || n == 0) return dest;
    size_t i;
    for (i = 0; i < n && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    for (; i < n; i++) {
        dest[i] = '\0';
    }
    return dest;
}

char *runtime_strcat(char *dest, const char *src) {
    if (!dest || !src) return dest;
    char *d = dest;
    while (*d) {
        d++;
    }
    while ((*d++ = *src++) != '\0') {
        // append loop
    }
    return dest;
}

char *runtime_strncat(char *dest, const char *src, size_t n) {
    if (!dest || !src || n == 0) return dest;
    char *d = dest;
    while (*d) {
        d++;
    }
    while (n > 0 && *src) {
        *d++ = *src++;
        n--;
    }
    *d = '\0';
    return dest;
}

char *runtime_strchr(const char *s, int c) {
    if (!s) return NULL;
    char ch = (char)c;
    while (*s) {
        if (*s == ch) {
            return (char *)s;
        }
        s++;
    }
    return (ch == '\0') ? (char *)s : NULL;
}

char *runtime_strrchr(const char *s, int c) {
    if (!s) return NULL;
    char ch = (char)c;
    const char *last = NULL;
    while (*s) {
        if (*s == ch) {
            last = s;
        }
        s++;
    }
    if (ch == '\0') return (char *)s;
    return (char *)last;
}

char *runtime_strstr(const char *haystack, const char *needle) {
    if (!haystack || !needle) return NULL;
    if (!*needle) return (char *)haystack;

    size_t nlen = runtime_strlen(needle);
    while (*haystack) {
        if (*haystack == *needle && runtime_strncmp(haystack, needle, nlen) == 0) {
            return (char *)haystack;
        }
        haystack++;
    }
    return NULL;
}

char *runtime_strcasestr(const char *haystack, const char *needle) {
    if (!haystack || !needle) return NULL;
    if (!*needle) return (char *)haystack;

    size_t nlen = runtime_strlen(needle);
    while (*haystack) {
        if (runtime_tolower((unsigned char)*haystack) == runtime_tolower((unsigned char)*needle) &&
            runtime_strncasecmp(haystack, needle, nlen) == 0) {
            return (char *)haystack;
        }
        haystack++;
    }
    return NULL;
}

char *runtime_strtok_r(char *str, const char *delim, char **saveptr) {
    if (!saveptr || (!str && !*saveptr)) return NULL;
    if (!delim) return NULL;

    char *s = str ? str : *saveptr;
    // Skip leading delimiters
    while (*s && runtime_strchr(delim, *s)) {
        s++;
    }
    if (*s == '\0') {
        *saveptr = s;
        return NULL;
    }

    char *token_start = s;
    // Scan until next delimiter or end of string
    while (*s && !runtime_strchr(delim, *s)) {
        s++;
    }

    if (*s != '\0') {
        *s = '\0';
        *saveptr = s + 1;
    } else {
        *saveptr = s;
    }

    return token_start;
}

size_t runtime_strspn(const char *s, const char *accept) {
    if (!s || !accept) return 0;
    size_t count = 0;
    while (*s && runtime_strchr(accept, *s)) {
        count++;
        s++;
    }
    return count;
}

size_t runtime_strcspn(const char *s, const char *reject) {
    if (!s || !reject) return 0;
    size_t count = 0;
    while (*s && !runtime_strchr(reject, *s)) {
        count++;
        s++;
    }
    return count;
}

char *runtime_strpbrk(const char *s, const char *accept) {
    if (!s || !accept) return NULL;
    while (*s) {
        if (runtime_strchr(accept, *s)) {
            return (char *)s;
        }
        s++;
    }
    return NULL;
}

size_t runtime_strlcpy(char *dst, const char *src, size_t siz) {
    if (!src) return 0;
    size_t srclen = runtime_strlen(src);
    if (!dst || siz == 0) return srclen;

    if (siz > 0) {
        size_t copylen = (srclen >= siz) ? (siz - 1) : srclen;
        runtime_memcpy(dst, src, copylen);
        dst[copylen] = '\0';
    }
    return srclen;
}

size_t runtime_strlcat(char *dst, const char *src, size_t siz) {
    if (!dst || !src) return 0;
    size_t dstlen = runtime_strlen(dst);
    size_t srclen = runtime_strlen(src);

    if (dstlen >= siz) {
        return siz + srclen;
    }

    size_t avail = siz - dstlen - 1;
    size_t copylen = (srclen > avail) ? avail : srclen;
    runtime_memcpy(dst + dstlen, src, copylen);
    dst[dstlen + copylen] = '\0';

    return dstlen + srclen;
}

char *runtime_strdup_custom(const char *s, void *(*alloc_fn)(size_t)) {
    if (!s || !alloc_fn) return NULL;
    size_t len = runtime_strlen(s);
    char *copy = (char *)alloc_fn(len + 1);
    if (copy) {
        runtime_memcpy(copy, s, len + 1);
    }
    return copy;
}
