/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: editor_core.h
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

#ifndef STANDALONE_EDITOR_CORE_H
#define STANDALONE_EDITOR_CORE_H

#include <stddef.h>

typedef struct ProgramLine {
    double line_number;
    char *text;
} ProgramLine;

typedef struct ProgramStore {
    ProgramLine *lines;
    int count;
    int capacity;
    char *bulk_buffer;
    size_t bulk_size;
} ProgramStore;

int editor_core_find_insert_pos(const ProgramStore *store, double line_number);
int editor_core_insert(ProgramStore *store, double line_number, const char *full_text, int *err_sorry);
int editor_core_insert_pointer(ProgramStore *store, double line_number, char *text_ptr, int *err_sorry);
int editor_core_delete(ProgramStore *store, double line_number);
int editor_core_find(const ProgramStore *store, double line_number);
int editor_core_find_next(const ProgramStore *store, double line_number);
void editor_core_clear(ProgramStore *store);

#endif /* STANDALONE_EDITOR_CORE_H */
