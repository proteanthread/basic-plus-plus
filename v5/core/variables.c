/**
 * GW-BASIC / BASIC++ Core Engine (C17)
 * 
 * -----------------------------------------------------------------------------
 * MAINTENANCE & EXTENSION GUIDELINES
 * -----------------------------------------------------------------------------
 * 1. WHAT CAN BE CHANGED:
 *    - Logic inside statement handlers to optimize behaviors.
 *    - Diagnostic logs and specific error message phrasing.
 * 
 * 2. WHAT CANNOT BE CHANGED:
 *    - Keyword/token byte mapping tables and execution loop structure.
 *    - Memory pool layouts.
 * 
 * 3. EXPECTED BEHAVIOR:
 *    - Core interpreter execution honoring C17 standards.
 * 
 * 4. WHAT TO DO IF SOMETHING BREAKS:
 *    - Check variable tables, default variable type states, and stack pointers.
 *    - Verify execution pointer updates correctly. Trace memory crashes back to pool margins.
 * -----------------------------------------------------------------------------
 */
#include "variables.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

GW_Var *gw_var_get(GW_State *state, const char *name, bool create) {
    if (!state || !name || !*name) return NULL;
    
    // Normalize name (uppercase)
    char norm_name[40];
    int i;
    for (i = 0; name[i] && i < 39; i++) {
        norm_name[i] = toupper((unsigned char)name[i]);
    }
    norm_name[i] = '\0';
    
    // Scan list
    GW_Var *v = state->variables;
    while (v) {
        if (strcmp(v->name, norm_name) == 0) {
            return v;
        }
        v = v->next;
    }
    
    if (!create) return NULL;
    
    // Create new
    GW_Var *new_var = (GW_Var *)calloc(1, sizeof(GW_Var));
    if (!new_var) return NULL;
    
    strncpy(new_var->name, norm_name, 39);
    
    // Determine type from suffix or default type map
    size_t name_len = strlen(norm_name);
    char last = norm_name[name_len - 1];
    GW_Type t = TYPE_SINGLE;
    
    if (last == '%') t = TYPE_INTEGER;
    else if (last == '!') t = TYPE_SINGLE;
    else if (last == '#') t = TYPE_DOUBLE;
    else if (last == '$') t = TYPE_STRING;
    else {
        char first = norm_name[0];
        if (isalpha((unsigned char)first)) {
            t = state->default_types[first - 'A'];
        }
    }
    
    new_var->type = t;
    new_var->next = state->variables;
    state->variables = new_var;
    
    return new_var;
}

void gw_var_set_num(GW_Var *var, double value) {
    if (!var) return;
    switch (var->type) {
        case TYPE_INTEGER:
            var->i_val = (int16_t)value;
            break;
        case TYPE_SINGLE:
            var->s_val = (float)value;
            break;
        case TYPE_DOUBLE:
            var->d_val = value;
            break;
        default:
            break;
    }
}

double gw_var_get_num(GW_Var *var) {
    if (!var) return 0.0;
    switch (var->type) {
        case TYPE_INTEGER: return (double)var->i_val;
        case TYPE_SINGLE: return (double)var->s_val;
        case TYPE_DOUBLE: return var->d_val;
        default: return 0.0;
    }
}

void gw_var_set_str(GW_State *state, GW_Var *var, const char *str, size_t len) {
    if (!var || var->type != TYPE_STRING) return;
    
    // Simple allocation in state string space
    if (len > 255) len = 255;
    
    // For simplicity, allocate memory dynamically per string and free old
    // in variables lookup, or manage it via string descriptor
    if (var->str.ptr) {
        free(var->str.ptr);
    }
    
    if (len == 0) {
        var->str.ptr = NULL;
        var->str.len = 0;
        return;
    }
    
    var->str.ptr = (char *)malloc(len + 1);
    if (var->str.ptr) {
        memcpy(var->str.ptr, str, len);
        var->str.ptr[len] = '\0';
        var->str.len = (uint8_t)len;
    }
}

void gw_vars_clear(GW_State *state) {
    if (!state) return;
    
    GW_Var *v = state->variables;
    while (v) {
        GW_Var *next = v->next;
        if (v->type == TYPE_STRING && v->str.ptr) {
            free(v->str.ptr);
        }
        free(v);
        v = next;
    }
    state->variables = NULL;
    
    GW_Array *arr = state->arrays;
    while (arr) {
        GW_Array *next = arr->next;
        if (arr->data) {
            // If string array, free strings inside
            if (arr->type == TYPE_STRING) {
                int total_elements = 1;
                for (int i = 0; i < arr->num_dims; i++) {
                    total_elements *= arr->dims[i].size;
                }
                GW_String *strings = (GW_String *)arr->data;
                for (int i = 0; i < total_elements; i++) {
                    if (strings[i].ptr) free(strings[i].ptr);
                }
            }
            free(arr->data);
        }
        free(arr);
        arr = next;
    }
    state->arrays = NULL;
}

GW_Array *gw_arr_get(GW_State *state, const char *name, bool create) {
    if (!state || !name || !*name) return NULL;
    
    char norm_name[40];
    int i;
    for (i = 0; name[i] && i < 39; i++) {
        norm_name[i] = toupper((unsigned char)name[i]);
    }
    norm_name[i] = '\0';
    
    GW_Array *arr = state->arrays;
    while (arr) {
        if (strcmp(arr->name, norm_name) == 0) {
            return arr;
        }
        arr = arr->next;
    }
    
    if (!create) return NULL;
    
    // Create default 1-dimension array of size 11 (0 to 10 is default in BASIC)
    GW_Array *new_arr = (GW_Array *)calloc(1, sizeof(GW_Array));
    if (!new_arr) return NULL;
    
    strncpy(new_arr->name, norm_name, 39);
    
    size_t name_len = strlen(norm_name);
    char last = norm_name[name_len - 1];
    GW_Type t = TYPE_SINGLE;
    if (last == '%') t = TYPE_INTEGER;
    else if (last == '!') t = TYPE_SINGLE;
    else if (last == '#') t = TYPE_DOUBLE;
    else if (last == '$') t = TYPE_STRING;
    
    new_arr->type = t;
    new_arr->num_dims = 1;
    new_arr->dims[0].lbound = 0;
    new_arr->dims[0].ubound = 10;
    new_arr->dims[0].size = 11;
    
    size_t cell_size = sizeof(double);
    if (t == TYPE_INTEGER) cell_size = sizeof(int16_t);
    else if (t == TYPE_SINGLE) cell_size = sizeof(float);
    else if (t == TYPE_STRING) cell_size = sizeof(GW_String);
    
    new_arr->data = calloc(11, cell_size);
    new_arr->next = state->arrays;
    state->arrays = new_arr;
    
    return new_arr;
}

void *gw_arr_index_ptr(GW_Array *arr, int num_dims, int *indices) {
    if (!arr || num_dims != arr->num_dims) return NULL;
    
    int flat_idx = 0;
    int multiplier = 1;
    for (int i = 0; i < num_dims; i++) {
        int idx = indices[i];
        if (idx < arr->dims[i].lbound || idx > arr->dims[i].ubound) {
            return NULL; // Out of bounds
        }
        flat_idx += (idx - arr->dims[i].lbound) * multiplier;
        multiplier *= arr->dims[i].size;
    }
    
    size_t cell_size = sizeof(double);
    if (arr->type == TYPE_INTEGER) cell_size = sizeof(int16_t);
    else if (arr->type == TYPE_SINGLE) cell_size = sizeof(float);
    else if (arr->type == TYPE_STRING) cell_size = sizeof(GW_String);
    
    return (void *)((char *)arr->data + (flat_idx * cell_size));
}

GW_Array *gw_arr_create_dims(GW_State *state, const char *name, int num_dims, int *sizes) {
    if (!state || !name || !*name || num_dims <= 0) return NULL;
    
    char norm_name[40];
    int i;
    for (i = 0; name[i] && i < 39; i++) {
        norm_name[i] = toupper((unsigned char)name[i]);
    }
    norm_name[i] = '\0';
    
    GW_Array *prev = NULL;
    GW_Array *curr = state->arrays;
    while (curr) {
        if (strcmp(curr->name, norm_name) == 0) {
            if (prev) {
                prev->next = curr->next;
            } else {
                state->arrays = curr->next;
            }
            if (curr->data) {
                if (curr->type == TYPE_STRING) {
                    int total_elements = 1;
                    for (int j = 0; j < curr->num_dims; j++) {
                        total_elements *= curr->dims[j].size;
                    }
                    GW_String *strings = (GW_String *)curr->data;
                    for (int j = 0; j < total_elements; j++) {
                        if (strings[j].ptr) free(strings[j].ptr);
                    }
                }
                free(curr->data);
            }
            free(curr);
            break;
        }
        prev = curr;
        curr = curr->next;
    }
    
    GW_Array *new_arr = (GW_Array *)calloc(1, sizeof(GW_Array));
    if (!new_arr) return NULL;
    
    strncpy(new_arr->name, norm_name, 39);
    
    size_t name_len = strlen(norm_name);
    char last = norm_name[name_len - 1];
    GW_Type t = TYPE_SINGLE;
    if (last == '%') t = TYPE_INTEGER;
    else if (last == '!') t = TYPE_SINGLE;
    else if (last == '#') t = TYPE_DOUBLE;
    else if (last == '$') t = TYPE_STRING;
    else {
        char first = norm_name[0];
        if (isalpha((unsigned char)first)) {
            t = state->default_types[first - 'A'];
        }
    }
    
    new_arr->type = t;
    new_arr->num_dims = (int16_t)num_dims;
    
    int total_elements = 1;
    for (int j = 0; j < num_dims; j++) {
        new_arr->dims[j].lbound = 0;
        new_arr->dims[j].ubound = (int16_t)sizes[j];
        new_arr->dims[j].size = (int16_t)(sizes[j] + 1);
        total_elements *= new_arr->dims[j].size;
    }
    
    size_t cell_size = sizeof(double);
    if (t == TYPE_INTEGER) cell_size = sizeof(int16_t);
    else if (t == TYPE_SINGLE) cell_size = sizeof(float);
    else if (t == TYPE_STRING) cell_size = sizeof(GW_String);
    
    new_arr->data = calloc(total_elements, cell_size);
    new_arr->next = state->arrays;
    state->arrays = new_arr;
    
    return new_arr;
}
