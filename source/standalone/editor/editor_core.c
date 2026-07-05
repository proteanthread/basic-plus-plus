#include "editor_core.h"
#include <stdlib.h>
#include <string.h>

/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: editor_core.c
 * Subsystem: Standalone Buffered Program Editor
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Manages the stored program lines (insertion, deletion, renumbering,
 *    and searches) in a sorted array format.
 *
 * 2. WHAT TO EXPECT:
 *    Binary search based lookups. Verbatim storage of line strings.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Line bounds constraints.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Array sorting invariants.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    Ensure memory is allocated correctly prior to program insertion.
 * ===================================================================== */

int editor_core_find_insert_pos(const ProgramStore *store, double line_number)
{
    int low = 0;
    int high = store->count - 1;
    int mid;

    while (low <= high) {
        mid = low + (high - low) / 2;
        if (store->lines[mid].line_number == line_number) {
            return mid;  /* exact match */
        } else if (store->lines[mid].line_number < line_number) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }

    return low;  /* insertion point */
}

int editor_core_insert(ProgramStore *store, double line_number, const char *full_text, int *err_sorry)
{
    int pos;
    int i;
    char *new_str;

    if (err_sorry) *err_sorry = 0;

    pos = editor_core_find_insert_pos(store, line_number);

    /* Check if this is a replacement of an existing line */
    if (pos < store->count &&
        store->lines[pos].line_number == line_number) {
        /* Replace existing line text */
        char *old_txt = store->lines[pos].text;
        new_str = (char *)malloc(strlen(full_text) + 1);
        if (!new_str) {
            if (err_sorry) *err_sorry = 1;
            return -1;
        }
        strcpy(new_str, full_text);

        if (old_txt != NULL) {
            if (store->bulk_buffer == NULL ||
                old_txt < store->bulk_buffer ||
                old_txt >= store->bulk_buffer + store->bulk_size) {
                free(old_txt);
            }
        }
        store->lines[pos].text = new_str;
        return 0;
    }

    /* Inserting a new line -- check capacity */
    if (store->count >= store->capacity) {
        if (err_sorry) *err_sorry = 1;
        return -1;
    }

    new_str = (char *)malloc(strlen(full_text) + 1);
    if (!new_str) {
        if (err_sorry) *err_sorry = 1;
        return -1;
    }
    strcpy(new_str, full_text);

    /* Shift lines from pos..count-1 up by one position. */
    for (i = store->count; i > pos; i--) {
        store->lines[i] = store->lines[i - 1];
    }

    /* Insert the new line at the correct sorted position */
    store->lines[pos].line_number = line_number;
    store->lines[pos].text = new_str;
    store->count++;

    return 0;
}

int editor_core_insert_pointer(ProgramStore *store, double line_number, char *text_ptr, int *err_sorry)
{
    int pos;
    int i;

    if (err_sorry) *err_sorry = 0;

    pos = editor_core_find_insert_pos(store, line_number);

    /* Check if this is a replacement of an existing line */
    if (pos < store->count &&
        store->lines[pos].line_number == line_number) {
        char *old_txt = store->lines[pos].text;
        if (old_txt != NULL) {
            if (store->bulk_buffer == NULL ||
                old_txt < store->bulk_buffer ||
                old_txt >= store->bulk_buffer + store->bulk_size) {
                free(old_txt);
            }
        }
        store->lines[pos].text = text_ptr;
        return 0;
    }

    /* Inserting a new line -- check capacity */
    if (store->count >= store->capacity) {
        if (err_sorry) *err_sorry = 1;
        return -1;
    }

    /* Shift lines from pos..count-1 up by one position. */
    for (i = store->count; i > pos; i--) {
        store->lines[i] = store->lines[i - 1];
    }

    /* Insert the new line */
    store->lines[pos].line_number = line_number;
    store->lines[pos].text = text_ptr;
    store->count++;

    return 0;
}

int editor_core_delete(ProgramStore *store, double line_number)
{
    int pos;
    int i;

    pos = editor_core_find_insert_pos(store, line_number);

    /* Verify we found an exact match */
    if (pos >= store->count ||
        store->lines[pos].line_number != line_number) {
        return -1;  /* line not found (silent, not an error) */
    }

    char *txt = store->lines[pos].text;
    if (txt != NULL) {
        if (store->bulk_buffer == NULL ||
            txt < store->bulk_buffer ||
            txt >= store->bulk_buffer + store->bulk_size) {
            free(txt);
        }
    }

    /* Shift lines down to fill the gap */
    for (i = pos; i < store->count - 1; i++) {
        store->lines[i] = store->lines[i + 1];
    }

    store->count--;
    return 0;
}

int editor_core_find(const ProgramStore *store, double line_number)
{
    int pos = editor_core_find_insert_pos(store, line_number);

    if (pos < store->count &&
        store->lines[pos].line_number == line_number) {
        return pos;
    }

    return -1;
}

int editor_core_find_next(const ProgramStore *store, double line_number)
{
    int pos = editor_core_find_insert_pos(store, line_number);

    if (pos < store->count) {
        return pos;
    }

    return -1;  /* no line at or after this number */
}

void editor_core_clear(ProgramStore *store)
{
    int i;
    for (i = 0; i < store->count; i++) {
        char *txt = store->lines[i].text;
        if (txt != NULL) {
            if (store->bulk_buffer == NULL ||
                txt < store->bulk_buffer ||
                txt >= store->bulk_buffer + store->bulk_size) {
                free(txt);
            }
            store->lines[i].text = NULL;
        }
    }
    if (store->bulk_buffer != NULL) {
        free(store->bulk_buffer);
        store->bulk_buffer = NULL;
    }
    store->bulk_size = 0;
    store->count = 0;
}
