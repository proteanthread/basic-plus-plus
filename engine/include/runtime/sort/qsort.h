// FILENAME: qsort.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (qsort.c)
// NEEDS: platform, memory
// Freestanding iterative quicksort and binary search algorithms.
//
// ---- Includes ----

#ifndef RUNTIME_SORT_QSORT_H
#define RUNTIME_SORT_QSORT_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// @brief Iterative, non-recursive introsort (quick sort with heapsort fallback on stack depth limit).
void runtime_qsort(void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *));

// @brief Binary search in a sorted array. Returns pointer to matching element or NULL.
void *runtime_bsearch(const void *key, const void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *));

#ifdef __cplusplus
}
#endif

#endif // RUNTIME_SORT_QSORT_H
