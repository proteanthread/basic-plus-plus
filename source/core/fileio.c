/**
 * Original GW-BASIC Interpreter Port (C17)
 * 
 * -----------------------------------------------------------------------------
 * MAINTENANCE & EXTENSION GUIDELINES
 * -----------------------------------------------------------------------------
 * 1. WHAT CAN BE CHANGED:
 *    - Console sizing, output formatting, or ANSI color/escape sequences.
 *    - Logic inside statement handlers to optimize standard BASIC behaviors.
 *    - Math functions (tuning logic for trigonometric or random values).
 * 
 * 2. WHAT CANNOT BE CHANGED:
 *    - Keyword/token byte mapping tables (essential for loading tokenized BAS binaries).
 *    - Segmented memory layout simulation structures.
 *    - Core mathematical parsing precedence chain (eval descent hierarchy).
 * 
 * 3. EXPECTED BEHAVIOR:
 *    - Logical device routing and file I/O operations (file, serial, printer, console channels).
 *    - Low-level disk file handles, record locking, and raw sector buffering.
 * 
 * 4. WHAT TO DO IF SOMETHING BREAKS:
 *    - Check variable tables, default variable type states, and stack pointers.
 *    - Cross-reference token layouts with original detokenization specifications.
 *    - Run diagnostic verification script to identify isolated error line numbers.
 * -----------------------------------------------------------------------------
 */
#include "fileio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int gw_file_open(GW_State *state, int channel, const char *path, int mode, int record_len) {
    if (!state || channel < 1 || channel > MAX_DEVICES) return -1;
    
    // Close existing if open
    gw_file_close(state, channel);
    
    const char *mode_str = "r";
    if (mode == 1) mode_str = "r";
    else if (mode == 2) mode_str = "w";
    else if (mode == 8) mode_str = "a";
    else if (mode == 4) mode_str = "r+b"; // Random binary
    
    FILE *f = fopen(path, mode_str);
    if (!f && mode == 4) {
        // Try creating random file if not exists
        f = fopen(path, "w+b");
    }
    
    if (!f) return -1;
    
    state->channels[channel].type = DEV_FILE;
    state->channels[channel].mode = mode;
    state->channels[channel].record_len = (record_len > 0) ? record_len : 128;
    state->channels[channel].handle = f;
    
    if (mode == 4) {
        state->channels[channel].record_buffer = (uint8_t *)calloc(1, state->channels[channel].record_len);
        state->channels[channel].record_buffer_size = state->channels[channel].record_len;
    }
    
    return 0;
}

void gw_file_close(GW_State *state, int channel) {
    if (!state || channel < 1 || channel > MAX_DEVICES) return;
    
    GW_FileChannel *ch = &state->channels[channel];
    if (ch->type == DEV_FILE && ch->handle) {
        fclose((FILE *)ch->handle);
        ch->handle = NULL;
    }
    
    if (ch->record_buffer) {
        free(ch->record_buffer);
        ch->record_buffer = NULL;
        ch->record_buffer_size = 0;
    }
    
    ch->type = DEV_NONE;
    ch->mode = 0;
}

void gw_file_print(GW_State *state, int channel, const char *str) {
    if (!state || channel < 1 || channel > MAX_DEVICES) return;
    GW_FileChannel *ch = &state->channels[channel];
    if (ch->type == DEV_FILE && ch->handle) {
        fprintf((FILE *)ch->handle, "%s", str);
    }
}

int gw_file_read_line(GW_State *state, int channel, char *buf, size_t max_len) {
    if (!state || channel < 1 || channel > MAX_DEVICES) return -1;
    GW_FileChannel *ch = &state->channels[channel];
    if (ch->type == DEV_FILE && ch->handle) {
        if (fgets(buf, max_len, (FILE *)ch->handle)) {
            size_t len = strlen(buf);
            while (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r')) {
                buf[--len] = '\0';
            }
            return 0;
        }
    }
    return -1;
}

void gw_file_field(GW_State *state, int channel, int offset, int field_len, GW_Var *var) {
    if (!state || channel < 1 || channel > MAX_DEVICES || !var) return;
    GW_FileChannel *ch = &state->channels[channel];
    if (ch->mode == 4 && ch->record_buffer) {
        // Point the string variable directly to a window in the record buffer
        if (offset + field_len <= (int)ch->record_buffer_size) {
            // GW-BASIC FIELD command maps strings to record buffers
            // Since we represent strings as dynamic pointer values, we can allocate
            // a custom shadow string referencing the buffer address.
            // For simplicity, we just bind it by setting the string value pointer:
            var->type = TYPE_STRING;
            if (var->str.ptr) free(var->str.ptr);
            
            var->str.ptr = (char *)malloc(field_len + 1);
            if (var->str.ptr) {
                memcpy(var->str.ptr, ch->record_buffer + offset, field_len);
                var->str.ptr[field_len] = '\0';
                var->str.len = field_len;
            }
        }
    }
}

void gw_file_get(GW_State *state, int channel, int record_num) {
    if (!state || channel < 1 || channel > MAX_DEVICES) return;
    GW_FileChannel *ch = &state->channels[channel];
    if (ch->type == DEV_FILE && ch->handle && ch->mode == 4 && ch->record_buffer) {
        FILE *f = (FILE *)ch->handle;
        fseek(f, (record_num - 1) * ch->record_len, SEEK_SET);
        size_t read_bytes = fread(ch->record_buffer, 1, ch->record_len, f);
        if (read_bytes < (size_t)ch->record_len) {
            memset(ch->record_buffer + read_bytes, 0, ch->record_len - read_bytes);
        }
    }
}

void gw_file_put(GW_State *state, int channel, int record_num) {
    if (!state || channel < 1 || channel > MAX_DEVICES) return;
    GW_FileChannel *ch = &state->channels[channel];
    if (ch->type == DEV_FILE && ch->handle && ch->mode == 4 && ch->record_buffer) {
        FILE *f = (FILE *)ch->handle;
        fseek(f, (record_num - 1) * ch->record_len, SEEK_SET);
        fwrite(ch->record_buffer, 1, ch->record_len, f);
        fflush(f);
    }
}
