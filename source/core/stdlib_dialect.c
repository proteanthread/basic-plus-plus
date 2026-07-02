/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: stdlib_dialect.c
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Core interpreter engine infrastructure, memory pool allocator, error model, values, platform, security gating, and boot configurations.
 *
 * 2. WHAT TO EXPECT:
 *    Fixed memory footprint utilizing compile-time pool allocators (defined in config.h). Avoids malloc/free at runtime.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Diagnostic logs, specific error message phrasing, platform detection strings, security sandbox policy matrices.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    BValue tagged union structure fields, core memory allocator logic, security capability ratings.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    Check config.h pool sizes (e.g. increase PROGRAM_MEMORY_SIZE). If security level is ratcheted, check security level enforcement policies.
 * ===================================================================== */

 // ---
 // BASIC++ Interpreter - stdlib_dialect.c
 // ---
 //
 // Dialect-Overridable API Layer.
 // Allows mapping and overriding behaviors dynamically.
 //
//
// HOW TO EXTEND:
//   See the preamble comments in related files for
//   customization and extension instructions.
//
// TROUBLESHOOTING:
//   Check error_occurred() after operations that can fail.
//   Use error_raise(ERR_xxx, line_num) for error reporting.
 // ---

#include <string.h>
#include <stdio.h>
#include "stdlib_dialect.h"
#include "config.h"
#include "../console.h"

static DialectOverrides active_overrides;

void stdlib_dialect_init(void) {
    memset(&active_overrides, 0, sizeof(active_overrides));
}

void stdlib_dialect_set_overrides(DialectId dialect, DialectOverrides *overrides) {
    // If the dialect matches the currently active dialect, install these overrides
    if (dialect_get_config()->id == dialect && overrides != NULL) {
        memcpy(&active_overrides, overrides, sizeof(DialectOverrides));
    }
}

void stdlib_dialect_load_profile(const char *profile_name) {
    // Explicitly load a dialect profile. 
       // This provides the hybrid ability to load profiles from .LIB 
       //        or invoke internal initialization routines. 
    if (strcmp(profile_name, "GWBASIC") == 0) {
        // Future: Could load a .LIB or call a specific init
        dialect_init(DIALECT_GW_BASIC);
    } else if (strcmp(profile_name, "QBASIC") == 0) {
        dialect_init(DIALECT_QBASIC);
    }
    // etc...
}

// API Dispatchers
void stdlib_dialect_format_error(int error_code, void *rt) {
    if (active_overrides.format_error) {
        active_overrides.format_error(error_code, rt);
    } else {
        // Default formatting if no dialect overrides it
        printf("Error: %d\n", error_code);
    }
}

BValue stdlib_dialect_format_number(double val, void *rt) {
    if (active_overrides.format_number) {
        return active_overrides.format_number(val, rt);
    }
    // Default: just float formatting
    return bval_float(val);
}

void stdlib_dialect_initialize_variable(BValue *var, void *rt) {
    if (active_overrides.initialize_variable) {
        active_overrides.initialize_variable(var, rt);
    } else {
        // Default MS-BASIC behavior: zero/empty string
        *var = bval_int(0);
    }
}

int stdlib_dialect_file_open(const char *filename, int mode, int channel, void *rt) {
    if (active_overrides.file_open) return active_overrides.file_open(filename, mode, channel, rt);
    return -1; 
}

int stdlib_dialect_file_close(int channel, void *rt) {
    if (active_overrides.file_close) return active_overrides.file_close(channel, rt);
    return -1;
}

int stdlib_dialect_file_eof(int channel, void *rt) {
    if (active_overrides.file_eof) return active_overrides.file_eof(channel, rt);
    return -1;
}

int stdlib_dialect_file_print(int channel, BValue *val, void *rt) {
    if (active_overrides.file_print) return active_overrides.file_print(channel, val, rt);
    return -1;
}

int stdlib_dialect_file_input(int channel, BValue *var, void *rt) {
    if (active_overrides.file_input) return active_overrides.file_input(channel, var, rt);
    return -1;
}

int stdlib_dialect_file_line_input(int channel, BValue *var, void *rt) {
    if (active_overrides.file_line_input) return active_overrides.file_line_input(channel, var, rt);
    return -1;
}

int stdlib_dialect_file_get_byte(int channel, void *rt) {
    if (active_overrides.file_get_byte) return active_overrides.file_get_byte(channel, rt);
    return -1;
}

int stdlib_dialect_file_put_byte(int channel, int byte_val, void *rt) {
    if (active_overrides.file_put_byte) return active_overrides.file_put_byte(channel, byte_val, rt);
    return -1;
}

int stdlib_dialect_file_read_bytes(int channel, int num_bytes, char *buffer, void *rt) {
    if (active_overrides.file_read_bytes) return active_overrides.file_read_bytes(channel, num_bytes, buffer, rt);
    return -1;
}

int stdlib_dialect_file_get_record(int channel, int record_num, void *rt) {
    if (active_overrides.file_get_record) return active_overrides.file_get_record(channel, record_num, rt);
    return -1;
}

int stdlib_dialect_file_put_record(int channel, int record_num, void *rt) {
    if (active_overrides.file_put_record) return active_overrides.file_put_record(channel, record_num, rt);
    return -1;
}

int stdlib_dialect_file_field_alloc(int channel, int record_len, void *rt) {
    if (active_overrides.file_field_alloc) return active_overrides.file_field_alloc(channel, record_len, rt);
    return -1;
}

int stdlib_dialect_file_seek(int channel, int position, void *rt) {
    if (active_overrides.file_seek) return active_overrides.file_seek(channel, position, rt);
    return -1;
}

int stdlib_dialect_file_loc(int channel, void *rt) {
    if (active_overrides.file_loc) return active_overrides.file_loc(channel, rt);
    return -1;
}

int stdlib_dialect_file_lof(int channel, void *rt) {
    if (active_overrides.file_lof) return active_overrides.file_lof(channel, rt);
    return -1;
}

int stdlib_dialect_file_ioctl(int channel, int command, int aux1, int aux2, void *rt) {
    if (active_overrides.file_ioctl) return active_overrides.file_ioctl(channel, command, aux1, aux2, rt);
    return -1;
}
