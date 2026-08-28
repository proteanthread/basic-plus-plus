// FILENAME: strops.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot, libcore, libengine, libhardware, libkernel, libserver, libstandard
// NEEDS: platform, memory
// Freestanding string manipulation and comparison routines.
//
// ---- Includes ----

#ifndef RUNTIME_STRING_STROPS_H
#define RUNTIME_STRING_STROPS_H

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// @brief Calculates the length of string s, excluding the terminating null byte.
size_t runtime_strlen(const char *s);

// @brief Calculates the length of string s, up to a maximum of maxlen bytes.
size_t runtime_strnlen(const char *s, size_t maxlen);

// @brief Compares two strings s1 and s2.
int runtime_strcmp(const char *s1, const char *s2);

// @brief Compares at most n bytes of two strings s1 and s2.
int runtime_strncmp(const char *s1, const char *s2, size_t n);

// @brief Compares two strings ignoring case.
int runtime_strcasecmp(const char *s1, const char *s2);

// @brief Compares at most n bytes of two strings ignoring case.
int runtime_strncasecmp(const char *s1, const char *s2, size_t n);

// @brief Copies string src to dest, including terminating null byte.
char *runtime_strcpy(char *dest, const char *src);

// @brief Copies at most n bytes of string src to dest. Null-pads if src length < n.
char *runtime_strncpy(char *dest, const char *src, size_t n);

// @brief Appends src to dest, overwriting terminating null byte at end of dest.
char *runtime_strcat(char *dest, const char *src);

// @brief Appends at most n bytes from src to dest.
char *runtime_strncat(char *dest, const char *src, size_t n);

// @brief Locates the first occurrence of c in string s.
char *runtime_strchr(const char *s, int c);

// @brief Locates the last occurrence of c in string s.
char *runtime_strrchr(const char *s, int c);

// @brief Finds the first occurrence of the substring needle in the string haystack.
char *runtime_strstr(const char *haystack, const char *needle);

// @brief Case-insensitive substring search.
char *runtime_strcasestr(const char *haystack, const char *needle);

// @brief Re-entrant thread-safe string tokenizer.
char *runtime_strtok_r(char *str, const char *delim, char **saveptr);

// @brief Calculates the length of the initial segment of s consisting of characters in accept.
size_t runtime_strspn(const char *s, const char *accept);

// @brief Calculates the length of the initial segment of s consisting of characters NOT in reject.
size_t runtime_strcspn(const char *s, const char *reject);

// @brief Locates the first occurrence in s of any character in accept.
char *runtime_strpbrk(const char *s, const char *accept);

// @brief Safe bounded string copy that always guarantees null-termination. Returns length of src.
size_t runtime_strlcpy(char *dst, const char *src, size_t siz);

// @brief Safe bounded string cat that always guarantees null-termination.
size_t runtime_strlcat(char *dst, const char *src, size_t siz);

// @brief Duplicates string s using allocator fn (or runtime_malloc).
char *runtime_strdup_custom(const char *s, void *(*alloc_fn)(size_t));

#ifdef __cplusplus
}
#endif

#endif // RUNTIME_STRING_STROPS_H
