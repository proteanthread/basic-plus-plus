#ifndef STDLIB_DIALECT_H
#define STDLIB_DIALECT_H

#include "dialect.h"
#include "value.h"

 // Dialect-Overridable API Layer
 // Defines customizable behaviors for formatting, numeric conversions, 
//
// HOW TO EXTEND:
//   See the corresponding .c implementation file for
//   detailed extension and customization instructions.
//
// TROUBLESHOOTING:
//   If you get 'undeclared identifier' errors after adding
//   new functions, make sure the declaration is added here
//   AND the definition exists in the .c file.
 // string concatenations, and I/O prompts based on the current dialect.

typedef struct DialectOverrides {
    BValue (*format_print)(BValue *val, void *rt);
    BValue (*prompt_input)(void *rt);
    BValue (*concat_strings)(BValue *a, BValue *b, void *rt);
    BValue (*num_to_string)(BValue *val, void *rt);
    
    // Fine-Tuned API Extensions
    void (*format_error)(int error_code, void *rt);
    BValue (*format_number)(double val, void *rt);
    void (*initialize_variable)(BValue *var, void *rt);
    
    // 1. Connection & State Hooks
    int (*file_open)(const char *filename, int mode, int channel, void *rt);
    int (*file_close)(int channel, void *rt);
    int (*file_eof)(int channel, void *rt);

    // 2. Sequential / Stream Hooks
    int (*file_print)(int channel, BValue *val, void *rt);
    int (*file_input)(int channel, BValue *var, void *rt);
    int (*file_line_input)(int channel, BValue *var, void *rt);

    // 3. Byte / Character Hooks
    int (*file_get_byte)(int channel, void *rt);
    int (*file_put_byte)(int channel, int byte_val, void *rt);
    int (*file_read_bytes)(int channel, int num_bytes, char *buffer, void *rt);

    // 4. Random Access & Record Hooks
    int (*file_get_record)(int channel, int record_num, void *rt);
    int (*file_put_record)(int channel, int record_num, void *rt);
    int (*file_field_alloc)(int channel, int record_len, void *rt);

    // 5. Position, Query & Device Hooks
    int (*file_seek)(int channel, int position, void *rt);
    int (*file_loc)(int channel, void *rt);
    int (*file_lof)(int channel, void *rt);
    int (*file_ioctl)(int channel, int command, int aux1, int aux2, void *rt);
} DialectOverrides;

void stdlib_dialect_init(void);
void stdlib_dialect_set_overrides(DialectId dialect, DialectOverrides *overrides);

// Commands for Explicit Profile Loading
void stdlib_dialect_load_profile(const char *profile_name);

// API Dispatchers
void stdlib_dialect_format_error(int error_code, void *rt);
BValue stdlib_dialect_format_number(double val, void *rt);
void stdlib_dialect_initialize_variable(BValue *var, void *rt);

int stdlib_dialect_file_open(const char *filename, int mode, int channel, void *rt);
int stdlib_dialect_file_close(int channel, void *rt);
int stdlib_dialect_file_eof(int channel, void *rt);

int stdlib_dialect_file_print(int channel, BValue *val, void *rt);
int stdlib_dialect_file_input(int channel, BValue *var, void *rt);
int stdlib_dialect_file_line_input(int channel, BValue *var, void *rt);

int stdlib_dialect_file_get_byte(int channel, void *rt);
int stdlib_dialect_file_put_byte(int channel, int byte_val, void *rt);
int stdlib_dialect_file_read_bytes(int channel, int num_bytes, char *buffer, void *rt);

int stdlib_dialect_file_get_record(int channel, int record_num, void *rt);
int stdlib_dialect_file_put_record(int channel, int record_num, void *rt);
int stdlib_dialect_file_field_alloc(int channel, int record_len, void *rt);

int stdlib_dialect_file_seek(int channel, int position, void *rt);
int stdlib_dialect_file_loc(int channel, void *rt);
int stdlib_dialect_file_lof(int channel, void *rt);
int stdlib_dialect_file_ioctl(int channel, int command, int aux1, int aux2, void *rt);

#endif
