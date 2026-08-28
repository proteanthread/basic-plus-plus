// FILENAME: qsort.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (memops.h, memops.c, qsort.h)
// Freestanding non-recursive quicksort and binary search implementation.
//
// ---- Includes ----

#include "runtime/sort/qsort.h"
#include "runtime/string/memops.h"
#include <stdint.h>

#define STACK_DEPTH 64

static void swap_bytes(uint8_t *a, uint8_t *b, size_t size) {
    uint8_t tmp[256];
    while (size > 0) {
        size_t block = (size < sizeof(tmp)) ? size : sizeof(tmp);
        runtime_memcpy(tmp, a, block);
        runtime_memcpy(a, b, block);
        runtime_memcpy(b, tmp, block);
        a += block;
        b += block;
        size -= block;
    }
}

// Insertion sort for small partitions (<= 16 elements)
static void insertion_sort(uint8_t *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *)) {
    uint8_t tmp[256];
    uint8_t *scratch = tmp; // assumes element <= 256

    for (size_t i = 1; i < nmemb; ++i) {
        runtime_memcpy(scratch, base + i * size, size);
        size_t j = i;
        while (j > 0 && compar(base + (j - 1) * size, scratch) > 0) {
            runtime_memcpy(base + j * size, base + (j - 1) * size, size);
            j--;
        }
        runtime_memcpy(base + j * size, scratch, size);
    }
}

void runtime_qsort(void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *)) {
    if (!base || nmemb <= 1 || size == 0 || !compar) {
        return;
    }

    if (nmemb <= 16) {
        insertion_sort((uint8_t *)base, nmemb, size, compar);
        return;
    }

    // Iterative QuickSort using an explicit stack
    struct {
        size_t left;
        size_t right;
    } stack[STACK_DEPTH];

    int top = 0;
    stack[top].left = 0;
    stack[top].right = nmemb - 1;

    uint8_t *b = (uint8_t *)base;

    while (top >= 0) {
        size_t left = stack[top].left;
        size_t right = stack[top].right;
        top--;

        if (right - left <= 16) {
            insertion_sort(b + left * size, right - left + 1, size, compar);
            continue;
        }

        // Median-of-three pivot selection
        size_t mid = left + (right - left) / 2;
        if (compar(b + left * size, b + mid * size) > 0) swap_bytes(b + left * size, b + mid * size, size);
        if (compar(b + left * size, b + right * size) > 0) swap_bytes(b + left * size, b + right * size, size);
        if (compar(b + mid * size, b + right * size) > 0) swap_bytes(b + mid * size, b + right * size, size);

        // Place pivot at right - 1
        swap_bytes(b + mid * size, b + (right - 1) * size, size);
        const uint8_t *pivot = b + (right - 1) * size;

        size_t i = left;
        size_t j = right - 1;

        for (;;) {
            while (compar(b + (++i) * size, pivot) < 0) {}
            while (compar(b + (--j) * size, pivot) > 0) {}
            if (i >= j) break;
            swap_bytes(b + i * size, b + j * size, size);
        }

        // Restore pivot
        swap_bytes(b + i * size, b + (right - 1) * size, size);

        // Push smaller partition first to minimize stack depth
        if (top + 2 < STACK_DEPTH) {
            if (i - left > right - i) {
                if (left < i - 1) {
                    stack[++top].left = left;
                    stack[top].right = i - 1;
                }
                if (i + 1 < right) {
                    stack[++top].left = i + 1;
                    stack[top].right = right;
                }
            } else {
                if (i + 1 < right) {
                    stack[++top].left = i + 1;
                    stack[top].right = right;
                }
                if (left < i - 1) {
                    stack[++top].left = left;
                    stack[top].right = i - 1;
                }
            }
        }
    }
}

void *runtime_bsearch(const void *key, const void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *)) {
    if (!key || !base || nmemb == 0 || size == 0 || !compar) {
        return NULL;
    }

    const uint8_t *b = (const uint8_t *)base;
    size_t low = 0;
    size_t high = nmemb;

    while (low < high) {
        size_t mid = low + (high - low) / 2;
        const void *mid_elem = b + mid * size;
        int cmp = compar(key, mid_elem);
        if (cmp == 0) {
            return (void *)mid_elem;
        } else if (cmp < 0) {
            high = mid;
        } else {
            low = mid + 1;
        }
    }

    return NULL;
}
