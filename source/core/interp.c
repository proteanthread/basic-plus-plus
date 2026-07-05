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
#define _DEFAULT_SOURCE
#include "interp.h"
#include "../vdev.h"
#include "tokenizer.h"
#include "../memmap.h"

// External declarations for KEYWORD properties
extern void keyword_prop_remove(int kw, const char *name);
extern void keyword_prop_clear(int kw);
extern int keyword_prop_set(int kw, const char *name, const char *value);
extern int keyword_prop_is_on(int kw, const char *name);
extern const char *keyword_prop_get(int kw, const char *name);
extern int keyword_prop_get_int(int kw, const char *name, int def_val);


#include "eval.h"
#include "variables.h"
#include "strings.h"
#include "memory.h"
#include "sdl2.h"
#include "console.h"
#include "../platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int dialect_runtime_create(const char *name, const char *base_name);
extern int dialect_runtime_enable_category(const char *name, const char *category_name);
extern int dialect_runtime_disable_keyword(const char *name, const char *kw_name);
extern int 0;
extern void 
extern void dialect_apply(void);
extern const char *dialect_get_name(void);
extern const char *dialect_get_short_name(void);
extern void dialect_list_all(void);
#include <ctype.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <dirent.h>
#include <fnmatch.h>
#ifndef FNM_CASEFOLD
#define FNM_CASEFOLD 0
#endif
#endif

// local stacks for execution control
static GW_Line *g_gosub_stack[128];
static uint8_t *g_gosub_ip_stack[128];
static int g_gosub_ptr = 0;

typedef struct {
    GW_Var *var;
    double limit;
    double step;
    GW_Line *loop_line;
    uint8_t *loop_ip;
} GW_ForLoop;

static GW_ForLoop g_for_stack[32];
static int g_for_ptr = 0;

typedef struct {
    GW_Line *loop_line;
    uint8_t *loop_ip;
} GW_WhileLoop;

static GW_WhileLoop g_while_stack[32];
static int g_while_ptr = 0;

static int g_graphics_x = 0;
static int g_graphics_y = 0;
static int g_draw_scale = 4;   // Default scale 1.0 (S4)
static int g_draw_color = 15;  // Default white (C15)

void gw_program_insert(GW_State *state, int32_t line_num, const uint8_t *tokens, size_t len) {
    if (!state) return;
    GW_Line *curr = state->program_head;
    GW_Line *prev_line = NULL;
    while (curr) {
        if (curr->line_num == line_num) {
            if (len == 0 || (len == 1 && curr->tokens[0] == '\0')) {
                if (prev_line) {
                    prev_line->next = curr->next;
                } else {
                    state->program_head = curr->next;
                }
                free(curr->tokens);
                free(curr);
            } else {
                free(curr->tokens);
                curr->tokens = (uint8_t *)malloc(len + 1);
                if (curr->tokens) {
                    memcpy(curr->tokens, tokens, len + 1);
                }
                curr->length = len;
            }
            return;
        }
        prev_line = curr;
        curr = curr->next;
    }
    if (len == 0 || (len == 1 && tokens[0] == '\0')) return;
    GW_Line *nl = (GW_Line *)malloc(sizeof(GW_Line));
    if (!nl) return;
    nl->line_num = line_num;
    nl->tokens = (uint8_t *)malloc(len + 1);
    if (nl->tokens) {
        memcpy(nl->tokens, tokens, len + 1);
    }
    nl->length = len;
    nl->next = NULL;
    GW_Line **p = &state->program_head;
    while (*p && (*p)->line_num < line_num) {
        p = &(*p)->next;
    }
    nl->next = *p;
    *p = nl;
}

void gw_program_clear(GW_State *state) {
    if (!state) return;
    GW_Line *l = state->program_head;
    while (l) {
        GW_Line *next = l->next;
        free(l->tokens);
        free(l);
        l = next;
    }
    state->program_head = NULL;
    state->current_line = NULL;
    state->ip = NULL;
    gw_vars_clear(state);
}

static GW_Line *find_line(GW_State *state, int32_t line_num) {
    GW_Line *l = state->program_head;
    while (l) {
        if (l->line_num == line_num) return l;
        l = l->next;
    }
    return NULL;
}

static void handle_print(GW_State *state) {
    bool newline = true;
    bool is_first_item = true;
    while (state->ip && *state->ip && *state->ip != ':' && *state->ip != '\n' && *state->ip != TOK_ELSE) {
        // Skip spaces
        while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
        if (!*state->ip || *state->ip == ':' || *state->ip == '\n' || *state->ip == TOK_ELSE) break;
        
        if (is_first_item) {
            const char *prefix = keyword_prop_get((int)TOK_PRINT, "PREFIX");
            if (prefix) {
                printf("%s", prefix);
            }
            if (*state->ip == '#') {
                state->ip++;
                GW_Value chan_val;
                gw_eval_expr(state, &chan_val);
                int channel = gw_val_get_int(&chan_val);
                printf("HANDLE_PRINT_FILE_CALLED channel=%d\n", channel);
                
                // skip comma
                while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
                if (*state->ip == ',') state->ip++;
                
                // For now, just skip the rest of the statement to avoid syntax errors
                while (*state->ip && *state->ip != ':' && *state->ip != '\n' && *state->ip != TOK_ELSE) {
                    state->ip++;
                }
                return;
            }
            is_first_item = false;
        }
        
        if (*state->ip == ';') {
            newline = false;
            state->ip++;
            continue;
        }
        

        if (*state->ip == ',') {
            int zone = keyword_prop_get_int((int)TOK_PRINT, "ZONE", 14);
            // Print next tab stop: we just print spaces to align to zone boundary
            // We would need to track column, but for now we'll just print fixed spaces or a tab
            if (zone == 14) {
                printf("\t");
            } else {
                for (int z = 0; z < zone; z++) printf(" ");
            }
            newline = false;
            state->ip++;
            continue;
        }

        
        GW_Value val;
        gw_eval_expr(state, &val);
        if (state->error_jump_pending || !state->execution_active) {
            if (val.type == TYPE_STRING) gw_str_free(&val.str);
            break;
        }
        

        if (val.type == TYPE_STRING) {
            if (val.str.ptr && val.str.len > 0) {
                if (keyword_prop_is_on((int)TOK_PRINT, "UPPERCASE")) {
                    for (size_t i = 0; i < val.str.len; i++) {
                        printf("%c", toupper((unsigned char)val.str.ptr[i]));
                    }
                } else if (keyword_prop_is_on((int)TOK_PRINT, "LOWERCASE")) {
                    for (size_t i = 0; i < val.str.len; i++) {
                        printf("%c", tolower((unsigned char)val.str.ptr[i]));
                    }
                } else {
                    printf("%.*s", (int)val.str.len, val.str.ptr);
                }
            }
            gw_str_free(&val.str);
        } else {

            // Numeric format
            printf(" %g ", gw_val_get_double(&val));
        }
        newline = true;
    }
    
    if (newline) {
        printf("\n");
    }
}

static void handle_let(GW_State *state) {
    // Explicit LET X = 5 or implicit X = 5
    if (*state->ip == TOK_LET) {
        state->ip++; // skip TOK_LET
    }
    while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
    
    // Parse Variable name
    char name[40];
    int idx = 0;
    while (isalnum(*state->ip) || *state->ip == '%' || *state->ip == '!' || *state->ip == '#' || *state->ip == '$') {
        if (idx < 39) name[idx++] = *state->ip;
        state->ip++;
    }
    name[idx] = '\0';
    
    // Check if it's an array cell assignment
    uint8_t *check_ptr = state->ip;
    while (*check_ptr == ' ' || *check_ptr == '\t') check_ptr++;
    
    if (*check_ptr == '(') {
        state->ip = check_ptr + 1; // skip past '('
        
        int indices[10];
        int num_dims = 0;
        while (num_dims < 10) {
            GW_Value idx_val;
            gw_eval_expr(state, &idx_val);
            indices[num_dims++] = gw_val_get_int(&idx_val);
            if (idx_val.type == TYPE_STRING) gw_str_free(&idx_val.str);
            
            while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
            if (*state->ip == ',') {
                state->ip++;
            } else if (*state->ip == ')') {
                state->ip++;
                break;
            } else {
                gw_error(2); // Syntax error
                return;
            }
        }
        
        while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
        if (*state->ip == '=') {
            state->ip++;
            GW_Value val;
            gw_eval_expr(state, &val);
            
            GW_Array *arr = gw_arr_get(state, name, true);
            if (arr) {
                void *ptr = gw_arr_index_ptr(arr, num_dims, indices);
                if (ptr) {
                    if (arr->type == TYPE_STRING) {
                        if (val.type == TYPE_STRING) {
                            GW_String *s_ptr = (GW_String *)ptr;
                            if (s_ptr->ptr) free(s_ptr->ptr);
                            s_ptr->ptr = (char *)malloc(val.str.len + 1);
                            if (s_ptr->ptr) {
                                memcpy(s_ptr->ptr, val.str.ptr, val.str.len);
                                s_ptr->ptr[val.str.len] = '\0';
                                s_ptr->len = val.str.len;
                            }
                            gw_str_free(&val.str);
                        } else {
                            gw_error(13); // Type mismatch
                        }
                    } else if (arr->type == TYPE_INTEGER) {
                        *(int16_t *)ptr = gw_val_get_int(&val);
                        if (val.type == TYPE_STRING) gw_str_free(&val.str);
                    } else if (arr->type == TYPE_SINGLE) {
                        *(float *)ptr = (float)gw_val_get_double(&val);
                        if (val.type == TYPE_STRING) gw_str_free(&val.str);
                    } else if (arr->type == TYPE_DOUBLE) {
                        *(double *)ptr = gw_val_get_double(&val);
                        if (val.type == TYPE_STRING) gw_str_free(&val.str);
                    }
                } else {
                    gw_error(9); // Subscript out of range
                    if (val.type == TYPE_STRING) gw_str_free(&val.str);
                }
            } else {
                gw_error(8); // Undefined array
                if (val.type == TYPE_STRING) gw_str_free(&val.str);
            }
        } else {
            gw_error(2); // Syntax error
        }
        return;
    }
    
    // Skip spaces to =
    while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
    
    if (*state->ip == '=') {
        state->ip++;
        GW_Value val;
        gw_eval_expr(state, &val);
        
        GW_Var *var = gw_var_get(state, name, true);
        if (var) {
            if (var->type == TYPE_STRING) {
                if (val.type == TYPE_STRING) {
                    gw_var_set_str(state, var, val.str.ptr, val.str.len);
                    gw_str_free(&val.str);
                } else {
                    gw_error(13); // Type mismatch
                }
            } else {
                gw_var_set_num(var, gw_val_get_double(&val));
            }
        }
    } else {
        gw_error(2); // Syntax error
    }
}

static bool get_lvalue(GW_State *state, GW_Var **out_var, GW_Array **out_arr, void **out_ptr) {
    *out_var = NULL;
    *out_arr = NULL;
    *out_ptr = NULL;
    
    while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
    char name[40];
    int idx = 0;
    while (isalnum(*state->ip) || *state->ip == '%' || *state->ip == '!' || *state->ip == '#' || *state->ip == '$') {
        if (idx < 39) name[idx++] = *state->ip;
        state->ip++;
    }
    name[idx] = '\0';
    if (idx == 0) return false;
    
    uint8_t *check_ptr = state->ip;
    while (*check_ptr == ' ' || *check_ptr == '\t') check_ptr++;
    if (*check_ptr == '(') {
        state->ip = check_ptr + 1;
        int indices[10];
        int num_dims = 0;
        while (num_dims < 10) {
            GW_Value idx_val;
            gw_eval_expr(state, &idx_val);
            if (state->error_jump_pending) return false;
            indices[num_dims++] = gw_val_get_int(&idx_val);
            if (idx_val.type == TYPE_STRING) gw_str_free(&idx_val.str);
            while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
            if (*state->ip == ',') {
                state->ip++;
            } else if (*state->ip == ')') {
                state->ip++;
                break;
            } else {
                gw_error(2);
                return false;
            }
        }
        *out_arr = gw_arr_get(state, name, true);
        if (*out_arr) {
            *out_ptr = gw_arr_index_ptr(*out_arr, num_dims, indices);
            if (!*out_ptr) {
                gw_error(9);
                return false;
            }
        } else {
            gw_error(8);
            return false;
        }
    } else {
        *out_var = gw_var_get(state, name, true);
        if (!*out_var) {
            gw_error(2);
            return false;
        }
    }
    return true;
}

static void get_lvalue_val(GW_Var *var, GW_Array *arr, void *ptr, GW_Value *val) {
    if (arr && ptr) {
        val->type = arr->type;
        if (arr->type == TYPE_STRING) {
            GW_String *s_ptr = (GW_String *)ptr;
            if (s_ptr->ptr && s_ptr->len > 0) {
                val->str.ptr = (char *)malloc(s_ptr->len + 1);
                memcpy(val->str.ptr, s_ptr->ptr, s_ptr->len);
                val->str.ptr[s_ptr->len] = '\0';
                val->str.len = s_ptr->len;
            } else {
                val->str.ptr = NULL;
                val->str.len = 0;
            }
        } else if (arr->type == TYPE_INTEGER) {
            val->i_val = *(int16_t *)ptr;
        } else if (arr->type == TYPE_SINGLE) {
            val->s_val = *(float *)ptr;
        } else if (arr->type == TYPE_DOUBLE) {
            val->d_val = *(double *)ptr;
        }
    } else if (var) {
        val->type = var->type;
        if (var->type == TYPE_STRING) {
            if (var->str.ptr && var->str.len > 0) {
                val->str.ptr = (char *)malloc(var->str.len + 1);
                memcpy(val->str.ptr, var->str.ptr, var->str.len);
                val->str.ptr[var->str.len] = '\0';
                val->str.len = var->str.len;
            } else {
                val->str.ptr = NULL;
                val->str.len = 0;
            }
        } else if (var->type == TYPE_INTEGER) {
            val->i_val = var->i_val;
        } else if (var->type == TYPE_SINGLE) {
            val->s_val = var->s_val;
        } else if (var->type == TYPE_DOUBLE) {
            val->d_val = var->d_val;
        }
    }
}

static void set_lvalue_val(GW_State *state, GW_Var *var, GW_Array *arr, void *ptr, GW_Value *val) {
    if (arr && ptr) {
        if (arr->type == TYPE_STRING) {
            GW_String *s_ptr = (GW_String *)ptr;
            if (s_ptr->ptr) free(s_ptr->ptr);
            if (val->str.ptr && val->str.len > 0) {
                s_ptr->ptr = (char *)malloc(val->str.len + 1);
                memcpy(s_ptr->ptr, val->str.ptr, val->str.len);
                s_ptr->ptr[val->str.len] = '\0';
                s_ptr->len = val->str.len;
            } else {
                s_ptr->ptr = NULL;
                s_ptr->len = 0;
            }
        } else if (arr->type == TYPE_INTEGER) {
            *(int16_t *)ptr = (int16_t)gw_val_get_double(val);
        } else if (arr->type == TYPE_SINGLE) {
            *(float *)ptr = (float)gw_val_get_double(val);
        } else if (arr->type == TYPE_DOUBLE) {
            *(double *)ptr = gw_val_get_double(val);
        }
    } else if (var) {
        if (var->type == TYPE_STRING) {
            gw_var_set_str(state, var, val->str.ptr, val->str.len);
        } else {
            gw_var_set_num(var, gw_val_get_double(val));
        }
    }
}

static void handle_swap(GW_State *state) {
    GW_Var *v1 = NULL, *v2 = NULL;
    GW_Array *a1 = NULL, *a2 = NULL;
    void *p1 = NULL, *p2 = NULL;
    
    if (!get_lvalue(state, &v1, &a1, &p1)) return;
    
    while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
    if (*state->ip == ',') {
        state->ip++;
    } else {
        gw_error(2);
        return;
    }
    
    if (!get_lvalue(state, &v2, &a2, &p2)) return;
    
    GW_Value val1 = {0}, val2 = {0};
    get_lvalue_val(v1, a1, p1, &val1);
    get_lvalue_val(v2, a2, p2, &val2);
    
    if (val1.type == TYPE_STRING && val2.type != TYPE_STRING) {
        gw_error(13);
        if (val1.type == TYPE_STRING) gw_str_free(&val1.str);
        if (val2.type == TYPE_STRING) gw_str_free(&val2.str);
        return;
    }
    if (val2.type == TYPE_STRING && val1.type != TYPE_STRING) {
        gw_error(13);
        if (val1.type == TYPE_STRING) gw_str_free(&val1.str);
        if (val2.type == TYPE_STRING) gw_str_free(&val2.str);
        return;
    }
    
    set_lvalue_val(state, v1, a1, p1, &val2);
    set_lvalue_val(state, v2, a2, p2, &val1);
    
    if (val1.type == TYPE_STRING) gw_str_free(&val1.str);
    if (val2.type == TYPE_STRING) gw_str_free(&val2.str);
}

static void handle_goto(GW_State *state) {
    GW_Value val;
    gw_eval_expr(state, &val);
    int32_t target_line = (int32_t)gw_val_get_double(&val);
    
    GW_Line *l = find_line(state, target_line);
    if (l) {
        state->current_line = l;
        state->ip = l->tokens;
        state->jump_pending = true;
    } else {
        gw_error(8); // Undefined line number
    }
}

static void handle_gosub(GW_State *state) {
    GW_Value val;
    gw_eval_expr(state, &val);
    int32_t target_line = (int32_t)gw_val_get_double(&val);
    
    GW_Line *l = find_line(state, target_line);
    if (l) {
        if (g_gosub_ptr < 128) {
            g_gosub_stack[g_gosub_ptr] = state->current_line;
            g_gosub_ip_stack[g_gosub_ptr] = state->ip;
            g_gosub_ptr++;
            
            state->current_line = l;
            state->ip = l->tokens;
            state->jump_pending = true;
        } else {
            gw_error(7); // Out of memory/stack overflow
        }
    } else {
        gw_error(8); // Undefined line number
    }
}

static void handle_return(GW_State *state) {
    if (g_gosub_ptr > 0) {
        g_gosub_ptr--;
        state->current_line = g_gosub_stack[g_gosub_ptr];
        state->ip = g_gosub_ip_stack[g_gosub_ptr];
        state->jump_pending = true;
    } else {
        gw_error(3); // RETURN without GOSUB
    }
}

static void handle_poke(GW_State *state) {
    GW_Value offset_val, data_val;
    gw_eval_expr(state, &offset_val);
    
    while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
    if (*state->ip == ',') state->ip++;
    
    gw_eval_expr(state, &data_val);
    
    uint16_t offset = (uint16_t)gw_val_get_int(&offset_val);
    uint8_t data = (uint8_t)gw_val_get_int(&data_val);
    
    gw_mem_poke(state->mem_sys, offset, data);
}

static void handle_def_seg(GW_State *state) {
    // Syntax: DEF SEG = segment
    if (*state->ip == TOK_SEG) {
        state->ip++;
        while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
        if (*state->ip == '=') {
            state->ip++;
            GW_Value seg_val;
            gw_eval_expr(state, &seg_val);
            gw_mem_def_seg(state->mem_sys, (uint16_t)gw_val_get_int(&seg_val));
        } else {
            gw_mem_def_seg(state->mem_sys, 0);
        }
    }
}

static void handle_screen(GW_State *state) {
    GW_Value val;
    gw_eval_expr(state, &val);
    int mode = gw_val_get_int(&val);
    state->screen_mode = mode;
    
    int cols = 80;
    if (mode == 0) {
        cols = 80; // default for text
    } else if (mode == 3) {
        if (state->machine == MACHINE_HERCULES) {
            cols = 90;
        } else if (state->machine == MACHINE_TANDY || state->machine == MACHINE_PCJR) {
            cols = 20;
        } else {
            cols = 80;
        }
    } else if (mode == 14) {
        if (state->machine == MACHINE_PLANTRONICS) {
            cols = 40;
        } else {
            cols = 80;
        }
    } else if (mode == 15) {
        cols = 80;
    } else if (mode == 1 || mode == 7 || mode == 13 || mode == 4 || mode == 5) {
        cols = 40;
    } else {
        cols = 80;
    }
    
    if (mode > 0) {
        gw_sdl2_init(640, 400, "BASIC++ Emulation", 0);
#ifndef NO_SDL2
        gw_sdl2_set_mode(mode, cols);
#endif
    } else {
#ifdef INPUT_CONSOLE
        gw_sdl2_cleanup();
#else
        gw_sdl2_set_mode(0, cols);
#endif
    }
}

static void handle_color(GW_State *state) {
    while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
    
    int val1 = -1;
    int val2 = -1;
    int val3 = -1;
    
    if (*state->ip && *state->ip != ',' && *state->ip != ':' && *state->ip != '\n' && *state->ip != TOK_ELSE) {
        GW_Value v1;
        gw_eval_expr(state, &v1);
        val1 = gw_val_get_int(&v1);
        if (v1.type == TYPE_STRING) gw_str_free(&v1.str);
    }
    
    while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
    if (*state->ip == ',') {
        state->ip++;
        while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
        if (*state->ip && *state->ip != ',' && *state->ip != ':' && *state->ip != '\n' && *state->ip != TOK_ELSE) {
            GW_Value v2;
            gw_eval_expr(state, &v2);
            val2 = gw_val_get_int(&v2);
            if (v2.type == TYPE_STRING) gw_str_free(&v2.str);
        }
    }
    
    while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
    if (*state->ip == ',') {
        state->ip++;
        while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
        if (*state->ip && *state->ip != ',' && *state->ip != ':' && *state->ip != '\n' && *state->ip != TOK_ELSE) {
            GW_Value v3;
            gw_eval_expr(state, &v3);
            val3 = gw_val_get_int(&v3);
            if (v3.type == TYPE_STRING) gw_str_free(&v3.str);
        }
    }
    
    if (state->screen_mode == 0) {
        // Text mode: COLOR [foreground][, [background][, border]]
        if (val1 >= 0) state->fg_color = val1;
        if (val2 >= 0) state->bg_color = val2;
        gw_sdl2_update_palette(0, state->machine, state->bg_color, 1);
        gw_sdl2_set_text_color(GW_PALETTE[state->fg_color % 16], GW_PALETTE[state->bg_color % 16]);
    } else if (state->screen_mode == 1) {
        // CGA SCREEN 1: COLOR [background][, palette]
        if (val1 >= 0) state->bg_color = val1;
        int palette_idx = (val2 >= 0) ? val2 : 1;
        gw_sdl2_update_palette(1, state->machine, state->bg_color, palette_idx);
        gw_sdl2_set_text_color(GW_PALETTE[3], GW_PALETTE[0]);
    } else if (state->screen_mode == 2) {
        // CGA SCREEN 2: COLOR [foreground][, background]
        if (val1 >= 0) state->fg_color = val1;
        if (val2 >= 0) state->bg_color = val2;
        gw_sdl2_update_palette(2, state->machine, state->bg_color, 1);
        gw_sdl2_set_text_color(GW_PALETTE[1], GW_PALETTE[0]);
    } else {
        // EGA/VGA modes: COLOR [foreground][, background]
        if (val1 >= 0) state->fg_color = val1;
        if (val2 >= 0) state->bg_color = val2;
        gw_sdl2_update_palette(state->screen_mode, state->machine, state->bg_color, 1);
        gw_sdl2_set_text_color(GW_PALETTE[state->fg_color % 16], GW_PALETTE[state->bg_color % 16]);
    }
}

static void handle_width(GW_State *state) {
    GW_Value val;
    gw_eval_expr(state, &val);
    int cols = gw_val_get_int(&val);
    if (cols == 40 || cols == 80) {
        gw_sdl2_set_mode(state->screen_mode, cols);
        printf("\033[2J\033[H"); // ANSI clear screen
        fflush(stdout);
    } else {
        gw_error(5); // Illegal function call
    }
}

static void list_files(const char *spec) {
#ifdef _WIN32
    WIN32_FIND_DATAA find_data;
    HANDLE hFind = FindFirstFileA(spec, &find_data);
    if (hFind == INVALID_HANDLE_VALUE) {
        printf("File not found\n");
        return;
    }
    
    int count = 0;
    do {
        if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            printf("<DIR> %-15s", find_data.cFileName);
        } else {
            printf("%-20s", find_data.cFileName);
        }
        count++;
        if (count % 4 == 0) {
            printf("\n");
        }
    } while (FindNextFileA(hFind, &find_data));
    
    FindClose(hFind);
    if (count % 4 != 0) {
        printf("\n");
    }
#else
    char spec_copy[256];
    strncpy(spec_copy, spec, sizeof(spec_copy) - 1);
    spec_copy[sizeof(spec_copy) - 1] = '\0';
    
    char *dir_name = ".";
    char *pattern = "*";
    
    char *last_slash = strrchr(spec_copy, '/');
    if (!last_slash) {
        last_slash = strrchr(spec_copy, '\\');
    }
    
    if (last_slash) {
        *last_slash = '\0';
        dir_name = spec_copy;
        pattern = last_slash + 1;
        if (strlen(dir_name) == 0) {
            dir_name = "/";
        }
        if (pattern == NULL || strlen(pattern) == 0) {
            pattern = "*";
        }
    } else {
        pattern = spec_copy;
    }
    
    if (strcmp(pattern, "*.*") == 0) {
        pattern = "*";
    }
    
    DIR *dir = opendir(dir_name);
    if (!dir) {
        printf("Directory not found: %s\n", dir_name);
        return;
    }
    
    struct dirent *entry;
    int count = 0;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        if (fnmatch(pattern, entry->d_name, FNM_CASEFOLD) == 0) {
            if (entry->d_type == DT_DIR) {
                printf("<DIR> %-15s", entry->d_name);
            } else {
                printf("%-20s", entry->d_name);
            }
            count++;
            if (count % 4 == 0) {
                printf("\n");
            }
        }
    }
    closedir(dir);
    if (count % 4 != 0) {
        printf("\n");
    }
#endif
}

static void handle_files(GW_State *state) {
    // Skip spaces
    while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
    
    char spec[256] = "*.*";
    if (*state->ip && *state->ip != ':' && *state->ip != '\n' && *state->ip != TOK_ELSE) {
        GW_Value val;
        gw_eval_expr(state, &val);
        if (val.type == TYPE_STRING) {
            if (val.str.ptr) {
                snprintf(spec, sizeof(spec), "%.*s", (int)val.str.len, val.str.ptr);
            }
            gw_str_free(&val.str);
        } else {
            gw_error(13); // Type mismatch
            return;
        }
    }
    list_files(spec);
}

static int get_bits_per_pixel(int mode) {
    switch (mode) {
        case 1: return 2;  // 4 colors (2 bpp)
        case 2: return 1;  // 2 colors (1 bpp)
        case 7: case 8: case 9: case 12: return 4;  // 16 colors (4 bpp)
        case 10: case 11: return 1;
        case 13: return 8;  // 256 colors (8 bpp)
        
        default: return 2;  // Fallback to 2 bpp
    }
}

static int get_palette_index(uint32_t color) {
    for (int i = 0; i < 16; i++) {
        if (GW_PALETTE[i] == color) return i;
    }
    return 0;
}

static void set_array_word(GW_Array *arr, int idx, uint16_t val) {
    if (idx < 0) return;
    int total_elements = 1;
    for (int i = 0; i < arr->num_dims; i++) {
        total_elements *= (arr->dims[i].ubound - arr->dims[i].lbound + 1);
    }
    if (idx >= total_elements) return;
    if (arr->type == TYPE_INTEGER) {
        ((int16_t *)arr->data)[idx] = (int16_t)val;
    } else if (arr->type == TYPE_SINGLE) {
        ((float *)arr->data)[idx] = (float)val;
    } else if (arr->type == TYPE_DOUBLE) {
        ((double *)arr->data)[idx] = (double)val;
    }
}

static uint16_t get_array_word(GW_Array *arr, int idx) {
    int total_elements = 1;
    for (int i = 0; i < arr->num_dims; i++) {
        total_elements *= (arr->dims[i].ubound - arr->dims[i].lbound + 1);
    }
    if (idx < 0 || idx >= total_elements) return 0;
    if (arr->type == TYPE_INTEGER) {
        return (uint16_t)((int16_t *)arr->data)[idx];
    } else if (arr->type == TYPE_SINGLE) {
        return (uint16_t)((float *)arr->data)[idx];
    } else if (arr->type == TYPE_DOUBLE) {
        return (uint16_t)((double *)arr->data)[idx];
    }
    return 0;
}

static void handle_pset_preset(GW_State *state, bool is_preset) {
    while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
    if (*state->ip == '(') state->ip++;
    
    GW_Value x_val, y_val;
    gw_eval_expr(state, &x_val);
    if (*state->ip == ',') state->ip++;
    gw_eval_expr(state, &y_val);
    if (*state->ip == ')') state->ip++;
    
    int x = gw_val_get_int(&x_val);
    int y = gw_val_get_int(&y_val);
    if (x_val.type == TYPE_STRING) gw_str_free(&x_val.str);
    if (y_val.type == TYPE_STRING) gw_str_free(&y_val.str);
    
    uint32_t color = GW_PALETTE[is_preset ? 0 : 15]; // Default: 0 for preset, 15 for pset
    
    while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
    if (*state->ip == ',') {
        state->ip++;
        GW_Value c_val;
        gw_eval_expr(state, &c_val);
        int color_idx = gw_val_get_int(&c_val) % 16;
        color = GW_PALETTE[color_idx];
        if (c_val.type == TYPE_STRING) gw_str_free(&c_val.str);
    }
    
    gw_sdl2_set_pixel(x, y, color);
    gw_sdl2_present();
    
    g_graphics_x = x;
    g_graphics_y = y;
}

static void handle_get(GW_State *state) {
    while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
    if (*state->ip == '(') state->ip++;
    
    GW_Value val_x1, val_y1;
    gw_eval_expr(state, &val_x1);
    if (*state->ip == ',') state->ip++;
    gw_eval_expr(state, &val_y1);
    if (*state->ip == ')') state->ip++;
    
    while (*state->ip == ' ' || *state->ip == '\t' || *state->ip == '-') state->ip++;
    if (*state->ip == '(') state->ip++;
    
    GW_Value val_x2, val_y2;
    gw_eval_expr(state, &val_x2);
    if (*state->ip == ',') state->ip++;
    gw_eval_expr(state, &val_y2);
    if (*state->ip == ')') state->ip++;
    
    while (*state->ip == ' ' || *state->ip == '\t' || *state->ip == ',') state->ip++;
    
    char array_name[40];
    int act_idx = 0;
    while (isalnum(*state->ip) || *state->ip == '%' || *state->ip == '!' || *state->ip == '#' || *state->ip == '$') {
        if (act_idx < 39) array_name[act_idx++] = *state->ip;
        state->ip++;
    }
    array_name[act_idx] = '\0';
    
    uint8_t *check_ptr = state->ip;
    while (*check_ptr == ' ' || *check_ptr == '\t') check_ptr++;
    if (*check_ptr == '(') {
        state->ip = check_ptr + 1;
        while (*state->ip && *state->ip != ')') state->ip++;
        if (*state->ip == ')') state->ip++;
    }
    
    int x1 = gw_val_get_int(&val_x1);
    int y1 = gw_val_get_int(&val_y1);
    int x2 = gw_val_get_int(&val_x2);
    int y2 = gw_val_get_int(&val_y2);
    
    if (val_x1.type == TYPE_STRING) gw_str_free(&val_x1.str);
    if (val_y1.type == TYPE_STRING) gw_str_free(&val_y1.str);
    if (val_x2.type == TYPE_STRING) gw_str_free(&val_x2.str);
    if (val_y2.type == TYPE_STRING) gw_str_free(&val_y2.str);
    
    GW_Array *arr = gw_arr_get(state, array_name, false);
    if (!arr) {
        gw_error(8); // Undefined array
        return;
    }
    
    int start_x = (x1 < x2) ? x1 : x2;
    int end_x = (x1 > x2) ? x1 : x2;
    int start_y = (y1 < y2) ? y1 : y2;
    int end_y = (y1 > y2) ? y1 : y2;
    
    int w = end_x - start_x + 1;
    int h = end_y - start_y + 1;
    int bpp = get_bits_per_pixel(state->screen_mode);
    int width_bits = w * bpp;
    int words_per_row = (width_bits + 15) / 16;
    
    set_array_word(arr, 0, (uint16_t)width_bits);
    set_array_word(arr, 1, (uint16_t)h);
    
    int arr_idx = 2;
    for (int curr_y = start_y; curr_y <= end_y; curr_y++) {
        for (int word_col = 0; word_col < words_per_row; word_col++) {
            uint16_t word_val = 0;
            for (int bit = 0; bit < 16; bit += bpp) {
                int pixel_in_word = bit / bpp;
                int pixel_x = start_x + word_col * (16 / bpp) + pixel_in_word;
                int color_idx = 0;
                if (pixel_x <= end_x) {
                    uint32_t color = gw_sdl2_get_pixel(pixel_x, curr_y);
                    color_idx = get_palette_index(color);
                }
                int shift = 16 - bpp - bit;
                word_val |= (color_idx & ((1 << bpp) - 1)) << shift;
            }
            set_array_word(arr, arr_idx++, word_val);
        }
    }
}

static void handle_put(GW_State *state) {
    while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
    if (*state->ip == '(') state->ip++;
    
    GW_Value val_x, val_y;
    gw_eval_expr(state, &val_x);
    if (*state->ip == ',') state->ip++;
    gw_eval_expr(state, &val_y);
    if (*state->ip == ')') state->ip++;
    
    while (*state->ip == ' ' || *state->ip == '\t' || *state->ip == ',') state->ip++;
    
    char array_name[40];
    int act_idx = 0;
    while (isalnum(*state->ip) || *state->ip == '%' || *state->ip == '!' || *state->ip == '#' || *state->ip == '$') {
        if (act_idx < 39) array_name[act_idx++] = *state->ip;
        state->ip++;
    }
    array_name[act_idx] = '\0';
    
    uint8_t *check_ptr = state->ip;
    while (*check_ptr == ' ' || *check_ptr == '\t') check_ptr++;
    if (*check_ptr == '(') {
        state->ip = check_ptr + 1;
        while (*state->ip && *state->ip != ')') state->ip++;
        if (*state->ip == ')') state->ip++;
    }
    
    int x = gw_val_get_int(&val_x);
    int y = gw_val_get_int(&val_y);
    if (val_x.type == TYPE_STRING) gw_str_free(&val_x.str);
    if (val_y.type == TYPE_STRING) gw_str_free(&val_y.str);
    
    int action = 0; // 0 = XOR, 1 = PSET, 2 = PRESET, 3 = OR, 4 = AND
    while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
    if (*state->ip == ',') {
        state->ip++;
        while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
        
        char act_name[20];
        int idx = 0;
        while (isalpha(*state->ip)) {
            if (idx < 19) act_name[idx++] = *state->ip;
            state->ip++;
        }
        act_name[idx] = '\0';
        for (int i = 0; act_name[i]; i++) act_name[i] = toupper((unsigned char)act_name[i]);
        
        if (strcmp(act_name, "PSET") == 0) action = 1;
        else if (strcmp(act_name, "PRESET") == 0) action = 2;
        else if (strcmp(act_name, "OR") == 0) action = 3;
        else if (strcmp(act_name, "AND") == 0) action = 4;
        else action = 0; // Default: XOR
    }
    
    GW_Array *arr = gw_arr_get(state, array_name, false);
    if (!arr) {
        gw_error(8); // Undefined array
        return;
    }
    
    uint16_t width_bits = get_array_word(arr, 0);
    uint16_t h = get_array_word(arr, 1);
    int bpp = get_bits_per_pixel(state->screen_mode);
    int w = width_bits / bpp;
    int words_per_row = (width_bits + 15) / 16;
    
    int arr_idx = 2;
    for (int curr_y = 0; curr_y < h; curr_y++) {
        int dest_y = y + curr_y;
        for (int word_col = 0; word_col < words_per_row; word_col++) {
            uint16_t word_val = get_array_word(arr, arr_idx++);
            for (int bit = 0; bit < 16; bit += bpp) {
                int pixel_in_word = bit / bpp;
                int pixel_x = word_col * (16 / bpp) + pixel_in_word;
                if (pixel_x < w) {
                    int dest_x = x + pixel_x;
                    
                    int shift = 16 - bpp - bit;
                    int color_idx = (word_val >> shift) & ((1 << bpp) - 1);
                    
                    uint32_t final_color = GW_PALETTE[color_idx];
                    
                    if (action == 0 || action == 3 || action == 4) { // XOR, OR, AND
                        uint32_t current_pixel_color = gw_sdl2_get_pixel(dest_x, dest_y);
                        int current_idx = get_palette_index(current_pixel_color);
                        int new_idx = 0;
                        if (action == 0) { // XOR
                            new_idx = current_idx ^ color_idx;
                        } else if (action == 3) { // OR
                            new_idx = current_idx | color_idx;
                        } else { // AND
                            new_idx = current_idx & color_idx;
                        }
                        final_color = GW_PALETTE[new_idx % 16];
                    } else if (action == 2) { // PRESET
                        int max_color = (1 << bpp) - 1;
                        int new_idx = max_color - color_idx;
                        final_color = GW_PALETTE[new_idx % 16];
                    }
                    
                    gw_sdl2_set_pixel(dest_x, dest_y, final_color);
                }
            }
        }
    }
    gw_sdl2_present();
    
    g_graphics_x = x;
    g_graphics_y = y;
}

static void handle_draw(GW_State *state) {
    GW_Value val;
    gw_eval_expr(state, &val);
    if (val.type != TYPE_STRING || !val.str.ptr) {
        gw_error(13); // Type mismatch
        if (val.type == TYPE_STRING) gw_str_free(&val.str);
        return;
    }
    
    const char *p = val.str.ptr;
    const char *end = val.str.ptr + val.str.len;
    
    while (p < end) {
        while (p < end && (isspace((unsigned char)*p) || *p == ';')) p++;
        if (p >= end) break;
        
        bool blank = false;
        bool no_update = false;
        
        if (toupper((unsigned char)*p) == 'B') {
            blank = true;
            p++;
        }
        if (p < end && toupper((unsigned char)*p) == 'N') {
            no_update = true;
            p++;
        }
        
        if (p >= end) break;
        
        char cmd = toupper((unsigned char)*p);
        p++;
        
        int n = 0;
        int has_arg = 0;
        
        if (cmd == 'M') {
            bool rel = false;
            int sign_x = 1;
            while (p < end && isspace((unsigned char)*p)) p++;
            if (p < end && *p == '+') {
                rel = true;
                p++;
            } else if (p < end && *p == '-') {
                rel = true;
                sign_x = -1;
                p++;
            }
            
            int target_x = 0;
            while (p < end && isdigit((unsigned char)*p)) {
                target_x = target_x * 10 + (*p - '0');
                p++;
            }
            target_x *= sign_x;
            
            while (p < end && (isspace((unsigned char)*p) || *p == ',')) p++;
            
            int sign_y = 1;
            if (p < end && *p == '+') {
                rel = true;
                p++;
            } else if (p < end && *p == '-') {
                rel = true;
                sign_y = -1;
                p++;
            }
            
            int target_y = 0;
            while (p < end && isdigit((unsigned char)*p)) {
                target_y = target_y * 10 + (*p - '0');
                p++;
            }
            target_y *= sign_y;
            
            int dest_x = target_x;
            int dest_y = target_y;
            if (rel) {
                dest_x = g_graphics_x + target_x;
                dest_y = g_graphics_y + target_y;
            }
            
            if (!blank) {
                uint32_t color = GW_PALETTE[g_draw_color % 16];
                gw_sdl2_draw_line(g_graphics_x, g_graphics_y, dest_x, dest_y, color);
            }
            
            if (!no_update) {
                g_graphics_x = dest_x;
                g_graphics_y = dest_y;
            }
            
        } else if (cmd == 'C' || cmd == 'S' || cmd == 'A' || cmd == 'U' || cmd == 'D' || cmd == 'L' || cmd == 'R' || cmd == 'E' || cmd == 'F' || cmd == 'G' || cmd == 'H') {
            while (p < end && isspace((unsigned char)*p)) p++;
            int sign = 1;
            if (p < end && *p == '+') p++;
            else if (p < end && *p == '-') {
                sign = -1;
                p++;
            }
            while (p < end && isdigit((unsigned char)*p)) {
                n = n * 10 + (*p - '0');
                p++;
                has_arg = 1;
            }
            n *= sign;
            
            if (cmd == 'C') {
                if (has_arg) g_draw_color = n;
            } else if (cmd == 'S') {
                if (has_arg) g_draw_scale = n;
            } else if (cmd == 'A') {
                // Rotations are not draw operations, ignored for donkey / simple graphics
            } else {
                double dist = (double)n * ((double)g_draw_scale / 4.0);
                if (!has_arg) dist = (double)g_draw_scale / 4.0;
                
                int dx = 0;
                int dy = 0;
                switch (cmd) {
                    case 'U': dy = -(int)dist; break;
                    case 'D': dy = (int)dist; break;
                    case 'L': dx = -(int)dist; break;
                    case 'R': dx = (int)dist; break;
                    case 'E': dx = (int)dist; dy = -(int)dist; break;
                    case 'F': dx = (int)dist; dy = (int)dist; break;
                    case 'G': dx = -(int)dist; dy = (int)dist; break;
                    case 'H': dx = -(int)dist; dy = -(int)dist; break;
                }
                
                int dest_x = g_graphics_x + dx;
                int dest_y = g_graphics_y + dy;
                
                if (!blank) {
                    uint32_t color = GW_PALETTE[g_draw_color % 16];
                    gw_sdl2_draw_line(g_graphics_x, g_graphics_y, dest_x, dest_y, color);
                }
                
                if (!no_update) {
                    g_graphics_x = dest_x;
                    g_graphics_y = dest_y;
                }
            }
        } else {
            p++;
        }
    }
    
    gw_sdl2_present();
    gw_str_free(&val.str);
}

static void handle_line(GW_State *state) {
    while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
    
    int x1 = g_graphics_x, y1 = g_graphics_y;
    int x2 = 0, y2 = 0;
    uint32_t color = GW_PALETTE[15]; // White
    
    if (*state->ip == '(') {
        state->ip++;
        GW_Value v;
        gw_eval_expr(state, &v);
        x1 = gw_val_get_int(&v);
        if (*state->ip == ',') state->ip++;
        gw_eval_expr(state, &v);
        y1 = gw_val_get_int(&v);
        if (*state->ip == ')') state->ip++;
        if (v.type == TYPE_STRING) gw_str_free(&v.str);
    }
    
    while (*state->ip == ' ' || *state->ip == '\t' || *state->ip == '-') state->ip++;
    
    if (*state->ip == '(') {
        state->ip++;
        GW_Value v;
        gw_eval_expr(state, &v);
        x2 = gw_val_get_int(&v);
        if (*state->ip == ',') state->ip++;
        gw_eval_expr(state, &v);
        y2 = gw_val_get_int(&v);
        if (*state->ip == ')') state->ip++;
        if (v.type == TYPE_STRING) gw_str_free(&v.str);
    }
    
    // Parse color if specified
    while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
    if (*state->ip == ',') {
        state->ip++;
        while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
        if (*state->ip && *state->ip != ',' && *state->ip != ':' && *state->ip != '\n' && *state->ip != TOK_ELSE) {
            GW_Value c_val;
            gw_eval_expr(state, &c_val);
            int color_idx = gw_val_get_int(&c_val) % 16;
            color = GW_PALETTE[color_idx];
            if (c_val.type == TYPE_STRING) gw_str_free(&c_val.str);
        }
    }
    
    bool box = false;
    bool fill = false;
    while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
    if (*state->ip == ',') {
        state->ip++;
        while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
        if (toupper(*state->ip) == 'B') {
            box = true;
            state->ip++;
            if (toupper(*state->ip) == 'F') {
                fill = true;
                state->ip++;
            }
        }
    }
    
    // Parse style if specified
    while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
    if (*state->ip == ',') {
        state->ip++;
        while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
        if (*state->ip && *state->ip != ':' && *state->ip != '\n' && *state->ip != TOK_ELSE) {
            GW_Value s_val;
            gw_eval_expr(state, &s_val);
            if (s_val.type == TYPE_STRING) gw_str_free(&s_val.str);
        }
    }
    
    if (box) {
        if (fill) {
            int start_y = (y1 < y2) ? y1 : y2;
            int end_y = (y1 > y2) ? y1 : y2;
            for (int curr_y = start_y; curr_y <= end_y; curr_y++) {
                gw_sdl2_draw_line(x1, curr_y, x2, curr_y, color);
            }
        } else {
            gw_sdl2_draw_line(x1, y1, x2, y1, color);
            gw_sdl2_draw_line(x2, y1, x2, y2, color);
            gw_sdl2_draw_line(x2, y2, x1, y2, color);
            gw_sdl2_draw_line(x1, y2, x1, y1, color);
        }
    } else {
        gw_sdl2_draw_line(x1, y1, x2, y2, color);
    }
    gw_sdl2_present();
    
    g_graphics_x = x2;
    g_graphics_y = y2;
}

static void handle_circle(GW_State *state) {
    while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
    
    int cx = 0, cy = 0, r = 0;
    uint32_t color = GW_PALETTE[15];
    
    if (*state->ip == '(') {
        state->ip++;
        GW_Value v;
        gw_eval_expr(state, &v);
        cx = gw_val_get_int(&v);
        if (*state->ip == ',') state->ip++;
        gw_eval_expr(state, &v);
        cy = gw_val_get_int(&v);
        if (*state->ip == ')') state->ip++;
        if (v.type == TYPE_STRING) gw_str_free(&v.str);
    }
    
    while (*state->ip == ' ' || *state->ip == '\t' || *state->ip == ',') state->ip++;
    
    GW_Value r_val;
    gw_eval_expr(state, &r_val);
    r = gw_val_get_int(&r_val);
    if (r_val.type == TYPE_STRING) gw_str_free(&r_val.str);
    
    while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
    if (*state->ip == ',') {
        state->ip++;
        while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
        if (*state->ip && *state->ip != ',' && *state->ip != ':' && *state->ip != '\n' && *state->ip != TOK_ELSE) {
            GW_Value c_val;
            gw_eval_expr(state, &c_val);
            int color_idx = gw_val_get_int(&c_val) % 16;
            color = GW_PALETTE[color_idx];
            if (c_val.type == TYPE_STRING) gw_str_free(&c_val.str);
        }
    }
    
    double start_angle = 0.0;
    double end_angle = 2.0 * M_PI;
    double aspect = (state->screen_mode == 2) ? (5.0 / 12.0) : (5.0 / 6.0);
    
    while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
    if (*state->ip == ',') {
        state->ip++;
        while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
        if (*state->ip && *state->ip != ',' && *state->ip != ':' && *state->ip != '\n' && *state->ip != TOK_ELSE) {
            GW_Value s_val;
            gw_eval_expr(state, &s_val);
            start_angle = gw_val_get_double(&s_val);
            if (s_val.type == TYPE_STRING) gw_str_free(&s_val.str);
        }
    }
    
    while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
    if (*state->ip == ',') {
        state->ip++;
        while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
        if (*state->ip && *state->ip != ',' && *state->ip != ':' && *state->ip != '\n' && *state->ip != TOK_ELSE) {
            GW_Value e_val;
            gw_eval_expr(state, &e_val);
            end_angle = gw_val_get_double(&e_val);
            if (e_val.type == TYPE_STRING) gw_str_free(&e_val.str);
        }
    }
    
    while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
    if (*state->ip == ',') {
        state->ip++;
        while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
        if (*state->ip && *state->ip != ',' && *state->ip != ':' && *state->ip != '\n' && *state->ip != TOK_ELSE) {
            GW_Value a_val;
            gw_eval_expr(state, &a_val);
            aspect = gw_val_get_double(&a_val);
            if (a_val.type == TYPE_STRING) gw_str_free(&a_val.str);
        }
    }
    
    if (aspect <= 0.0) aspect = 1.0;
    
    double start = start_angle;
    double end = end_angle;
    bool line_to_start = false;
    bool line_to_end = false;
    
    if (signbit(start)) {
        start = -start;
        line_to_start = true;
    }
    if (signbit(end)) {
        end = -end;
        line_to_end = true;
    }
    
    double rx = r;
    double ry = r;
    if (aspect < 1.0) {
        rx = r;
        ry = r * aspect;
    } else {
        ry = r;
        rx = r / aspect;
    }
    
    double step = 1.0 / (rx > ry ? rx : ry);
    if (step > 0.1) step = 0.1;
    if (step < 0.001) step = 0.001;
    
    double t = start;
    double target = end;
    if (start > end) {
        target = end + 2.0 * M_PI;
    }
    
    int first = 1;
    double last_x = 0, last_y = 0;
    for (double theta = t; theta <= target; theta += step) {
        double x = cx + rx * cos(theta);
        double y = cy - ry * sin(theta);
        if (first) {
            if (line_to_start) {
                gw_sdl2_draw_line(cx, cy, (int)x, (int)y, color);
            }
            first = 0;
        } else {
            gw_sdl2_draw_line((int)last_x, (int)last_y, (int)x, (int)y, color);
        }
        last_x = x;
        last_y = y;
    }
    
    double end_x = cx + rx * cos(end);
    double end_y = cy - ry * sin(end);
    gw_sdl2_draw_line((int)last_x, (int)last_y, (int)end_x, (int)end_y, color);
    if (line_to_end) {
        gw_sdl2_draw_line(cx, cy, (int)end_x, (int)end_y, color);
    }
    gw_sdl2_present();
    
    g_graphics_x = cx;
    g_graphics_y = cy;
}

static void handle_paint(GW_State *state) {
    // Syntax: PAINT (x, y), fill_color, border_color
    while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
    int x = 0, y = 0;
    uint32_t fill_color = GW_PALETTE[15];
    uint32_t border_color = GW_PALETTE[15];
    
    if (*state->ip == '(') {
        state->ip++;
        GW_Value v;
        gw_eval_expr(state, &v);
        x = gw_val_get_int(&v);
        if (*state->ip == ',') state->ip++;
        gw_eval_expr(state, &v);
        y = gw_val_get_int(&v);
        if (*state->ip == ')') state->ip++;
    }
    
    while (*state->ip == ' ' || *state->ip == '\t' || *state->ip == ',') state->ip++;
    
    GW_Value c_val;
    gw_eval_expr(state, &c_val);
    int fill_idx = gw_val_get_int(&c_val) % 16;
    fill_color = GW_PALETTE[fill_idx];
    
    while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
    if (*state->ip == ',') {
        state->ip++;
        GW_Value b_val;
        gw_eval_expr(state, &b_val);
        int border_idx = gw_val_get_int(&b_val) % 16;
        border_color = GW_PALETTE[border_idx];
    }
    
    gw_sdl2_paint(x, y, fill_color, border_color);
    gw_sdl2_present();
}

static void preprocess_mml(GW_State *state, const char *src, char *dst, size_t dst_max) {
    size_t dst_idx = 0;
    const char *p = src;
    
    while (*p && dst_idx < dst_max - 1) {
        if (*p == '=' && isalpha((unsigned char)p[1])) {
            p++; // skip '='
            char var_name[40];
            int var_len = 0;
            while (isalnum((unsigned char)*p) || *p == '%' || *p == '!' || *p == '#' || *p == '$') {
                if (var_len < 39) {
                    var_name[var_len++] = *p;
                }
                p++;
            }
            var_name[var_len] = '\0';
            
            // Look up variable
            GW_Var *v = gw_var_get(state, var_name, false);
            int val = 0;
            if (v) {
                val = (int)gw_var_get_num(v);
            }
            
            char val_str[32];
            sprintf(val_str, "%d", val);
            size_t val_len = strlen(val_str);
            if (dst_idx + val_len < dst_max - 1) {
                strcpy(dst + dst_idx, val_str);
                dst_idx += val_len;
            }
        } else {
            dst[dst_idx++] = *p++;
        }
    }
    dst[dst_idx] = '\0';
}

static void handle_play(GW_State *state) {
    GW_Value val;
    gw_eval_expr(state, &val);
    if (val.type == TYPE_STRING) {
        if (val.str.ptr) {
            char processed[4096];
            preprocess_mml(state, val.str.ptr, processed, sizeof(processed));
            gw_sdl2_play_mml(processed);
        }
        gw_str_free(&val.str);
    }
}

static void handle_sound(GW_State *state) {
    // Syntax: SOUND freq, duration
    GW_Value freq_val, dur_val;
    gw_eval_expr(state, &freq_val);
    
    while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
    if (*state->ip == ',') state->ip++;
    
    gw_eval_expr(state, &dur_val);
    
    float freq = (float)gw_val_get_double(&freq_val);
    int duration = (int)(gw_val_get_double(&dur_val) * 54.9); // GW-BASIC clock ticks (18.2Hz) to ms conversion
    
    gw_sdl2_play_tone(freq, duration, 1);
}

static void handle_call(GW_State *state) {
    while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
    char name[40];
    int idx = 0;
    while (isalnum(*state->ip) || *state->ip == '%' || *state->ip == '!' || *state->ip == '#' || *state->ip == '$' || *state->ip == '.') {
        if (idx < 39) name[idx++] = toupper(*state->ip);
        state->ip++;
    }
    name[idx] = '\0';
    
    GW_SubDef *sub = state->subs;
    while (sub) {
        if (strcmp(sub->name, name) == 0) break;
        sub = sub->next;
    }
    
    if (!sub) {
        gw_error(8); // Undefined
        return;
    }
    
    // Evaluate arguments into an array of values
    GW_Value args[20];
    int num_args = 0;
    while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
    if (*state->ip == '(') {
        state->ip++;
        while (num_args < 20) {
            gw_eval_expr(state, &args[num_args++]);
            while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
            if (*state->ip == ',') {
                state->ip++;
            } else if (*state->ip == ')') {
                state->ip++;
                break;
            } else {
                gw_error(2); // Syntax error
                return;
            }
        }
    }
    
    // Create new call frame
    GW_CallFrame *frame = (GW_CallFrame *)calloc(1, sizeof(GW_CallFrame));
    frame->return_line = state->current_line;
    frame->return_ip = state->ip;
    frame->local_variables = state->variables;
    frame->local_arrays = state->arrays;
    frame->next = state->call_stack;
    state->call_stack = frame;
    
    // Switch to local scope
    state->variables = NULL;
    state->arrays = NULL;
    
    // Parse parameters and assign argument values
    uint8_t *param_ip = sub->start_ip;
    while (*param_ip == ' ' || *param_ip == '\t') param_ip++;
    if (*param_ip == '(') {
        param_ip++;
        int param_idx = 0;
        while (param_idx < num_args) {
            while (*param_ip == ' ' || *param_ip == '\t') param_ip++;
            char pname[40];
            int pidx = 0;
            while (isalnum(*param_ip) || *param_ip == '%' || *param_ip == '!' || *param_ip == '#' || *param_ip == '$' || *param_ip == '.') {
                if (pidx < 39) pname[pidx++] = toupper(*param_ip);
                param_ip++;
            }
            pname[pidx] = '\0';
            
            // Assign argument to local variable
            GW_Var *var = gw_var_get(state, pname, true);
            if (var) {
                var->type = args[param_idx].type;
                if (var->type == TYPE_STRING) {
                    // Copy string value to avoid double free
                    gw_var_set_str(state, var, args[param_idx].str.ptr, args[param_idx].str.len);
                    gw_str_free(&args[param_idx].str);
                } else if (var->type == TYPE_DOUBLE) {
                    var->d_val = args[param_idx].d_val;
                } else if (var->type == TYPE_INTEGER) {
                    var->i_val = args[param_idx].i_val;
                } else {
                    var->s_val = args[param_idx].s_val;
                }
            }
            param_idx++;
            
            while (*param_ip == ' ' || *param_ip == '\t') param_ip++;
            if (*param_ip == ',') param_ip++;
            else if (*param_ip == ')') break;
        }
    }
    
    // Jump to sub body (which starts on the next line)
    state->current_line = sub->start_line->next;
    if (state->current_line) {
        state->ip = state->current_line->tokens;
    }
    state->jump_pending = true;
}

static void handle_sub_decl(GW_State *state, uint16_t end_tok) {
    // We entered a SUB or FUNCTION declaration during linear execution.
    // We must jump over its body.
    GW_Line *line = state->current_line;
    uint8_t *ip = state->ip;
    
    while (line) {
        while (*ip) {
            uint16_t tok = *ip;
            if (tok == 0xFE || tok == 0xFD || tok == 0xFF) {
                if (*(ip+1)) {
                    tok = (tok << 8) | *(ip+1);
                    ip++;
                }
            }
            if (tok == TOK_END) {
                uint8_t *next_ip = ip + 1;
                while (*next_ip == ' ' || *next_ip == '\t') next_ip++;
                uint16_t next_tok = *next_ip;
                if (next_tok == 0xFE || next_tok == 0xFD || next_tok == 0xFF) {
                    if (*(next_ip+1)) {
                        next_tok = (next_tok << 8) | *(next_ip+1);
                        next_ip++;
                    }
                }
                if (next_tok == end_tok) {
                    state->current_line = line;
                    state->ip = next_ip + 1;
                    return;
                }
            } else if (tok == '"') {
                ip++;
                while (*ip && *ip != '"') ip++;
                if (*ip) ip++;
                continue;
            }
            ip++;
        }
        line = line->next;
        if (line) ip = line->tokens;
    }
}

static void handle_dim(GW_State *state) {
    while (state->ip && *state->ip && *state->ip != ':' && *state->ip != '\n' && *state->ip != TOK_ELSE) {
        while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
        if (!*state->ip || *state->ip == ':' || *state->ip == '\n' || *state->ip == TOK_ELSE) break;
        
        char name[40];
        int idx = 0;
        while (isalnum(*state->ip) || *state->ip == '%' || *state->ip == '!' || *state->ip == '#' || *state->ip == '$') {
            if (idx < 39) name[idx++] = *state->ip;
            state->ip++;
        }
        name[idx] = '\0';
        
        while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
        if (*state->ip == '(') {
            state->ip++;
            int sizes[10];
            int num_dims = 0;
            while (num_dims < 10) {
                GW_Value size_val;
                gw_eval_expr(state, &size_val);
                sizes[num_dims++] = gw_val_get_int(&size_val);
                if (size_val.type == TYPE_STRING) gw_str_free(&size_val.str);
                
                while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
                if (*state->ip == ',') {
                    state->ip++;
                } else if (*state->ip == ')') {
                    state->ip++;
                    break;
                } else {
                    gw_error(2); // Syntax error
                    return;
                }
            }
            gw_arr_create_dims(state, name, num_dims, sizes);
        } else {
            gw_error(2); // Syntax error
            return;
        }
        
        while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
        if (*state->ip == ',') {
            state->ip++;
        }
    }
}

static void handle_restore(GW_State *state) {
    while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
    if (*state->ip && *state->ip != ',' && *state->ip != ':' && *state->ip != '\n' && *state->ip != TOK_ELSE) {
        GW_Value val;
        gw_eval_expr(state, &val);
        int32_t target_line = (int32_t)gw_val_get_double(&val);
        GW_Line *l = find_line(state, target_line);
        if (l) {
            state->data_line = l;
            state->data_ip = l->tokens;
        } else {
            gw_error(8); // Undefined line number
        }
    } else {
        state->data_line = state->program_head;
        state->data_ip = NULL;
    }
}

static void handle_read(GW_State *state) {
    while (state->ip && *state->ip && *state->ip != ':' && *state->ip != '\n' && *state->ip != TOK_ELSE) {
        while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
        if (!*state->ip || *state->ip == ':' || *state->ip == '\n' || *state->ip == TOK_ELSE) break;
        
        char var_name[40];
        int idx = 0;
        while (isalnum(*state->ip) || *state->ip == '%' || *state->ip == '!' || *state->ip == '#' || *state->ip == '$') {
            if (idx < 39) var_name[idx++] = *state->ip;
            state->ip++;
        }
        var_name[idx] = '\0';
        
        bool is_array = false;
        int indices[10];
        int num_dims = 0;
        
        uint8_t *check_ptr = state->ip;
        while (*check_ptr == ' ' || *check_ptr == '\t') check_ptr++;
        if (*check_ptr == '(') {
            is_array = true;
            state->ip = check_ptr + 1;
            while (num_dims < 10) {
                GW_Value idx_val;
                gw_eval_expr(state, &idx_val);
                indices[num_dims++] = gw_val_get_int(&idx_val);
                if (idx_val.type == TYPE_STRING) gw_str_free(&idx_val.str);
                
                while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
                if (*state->ip == ',') {
                    state->ip++;
                } else if (*state->ip == ')') {
                    state->ip++;
                    break;
                } else {
                    gw_error(2); // Syntax error
                    return;
                }
            }
        }
        
        bool found = (state->data_ip != NULL);
        if (!found) {
            while (state->data_line) {
                if (!state->data_ip) {
                    state->data_ip = state->data_line->tokens;
                }
                
                while (state->data_ip && *state->data_ip) {
                    while (*state->data_ip == ' ' || *state->data_ip == '\t') state->data_ip++;
                    if (*state->data_ip == TOK_DATA) {
                        state->data_ip++;
                        found = true;
                        break;
                    }
                    while (*state->data_ip && *state->data_ip != ':' && *state->data_ip != '\n') {
                        if (*state->data_ip == '"') {
                            state->data_ip++;
                            while (*state->data_ip && *state->data_ip != '"') state->data_ip++;
                            if (*state->data_ip == '"') state->data_ip++;
                        } else {
                            state->data_ip++;
                        }
                    }
                    if (*state->data_ip == ':') {
                        state->data_ip++;
                    }
                }
                
                if (found) break;
                
                state->data_line = state->data_line->next;
                state->data_ip = NULL;
            }
        }
        
        if (!found) {
            gw_error(4); // Out of DATA
            return;
        }
        
        while (*state->data_ip == ' ' || *state->data_ip == '\t') state->data_ip++;
        
        GW_Value val;
        val.type = TYPE_NONE;
        
        if (*state->data_ip == '"') {
            state->data_ip++;
            const char *start = (const char *)state->data_ip;
            while (*state->data_ip && *state->data_ip != '"') state->data_ip++;
            size_t len = (const char *)state->data_ip - start;
            if (*state->data_ip == '"') state->data_ip++;
            val.type = TYPE_STRING;
            val.str = gw_str_create(start, len);
        } else {
            const char *start = (const char *)state->data_ip;
            while (*state->data_ip && *state->data_ip != ',' && *state->data_ip != ':' && *state->data_ip != '\n') {
                state->data_ip++;
            }
            size_t len = (const char *)state->data_ip - start;
            
            while (len > 0 && isspace((unsigned char)start[0])) {
                start++;
                len--;
            }
            while (len > 0 && isspace((unsigned char)start[len - 1])) {
                len--;
            }
            
            bool is_num = true;
            if (len == 0) {
                is_num = false;
            } else {
                for (size_t i = 0; i < len; i++) {
                    char c = start[i];
                    if (!isdigit((unsigned char)c) && c != '.' && c != '-' && c != '+' && c != 'e' && c != 'E') {
                        is_num = false;
                        break;
                    }
                }
            }
            
            if (is_num) {
                char tmp[64];
                if (len > 63) len = 63;
                memcpy(tmp, start, len);
                tmp[len] = '\0';
                val.type = TYPE_DOUBLE;
                val.d_val = strtod(tmp, NULL);
            } else {
                val.type = TYPE_STRING;
                val.str = gw_str_create(start, len);
            }
        }
        
        if (is_array) {
            GW_Array *arr = gw_arr_get(state, var_name, true);
            if (arr) {
                void *ptr = gw_arr_index_ptr(arr, num_dims, indices);
                if (ptr) {
                    if (arr->type == TYPE_STRING) {
                        if (val.type == TYPE_STRING) {
                            GW_String *s_ptr = (GW_String *)ptr;
                            if (s_ptr->ptr) free(s_ptr->ptr);
                            s_ptr->ptr = (char *)malloc(val.str.len + 1);
                            if (s_ptr->ptr) {
                                memcpy(s_ptr->ptr, val.str.ptr, val.str.len);
                                s_ptr->ptr[val.str.len] = '\0';
                                s_ptr->len = val.str.len;
                            }
                            gw_str_free(&val.str);
                        } else {
                            gw_error(13);
                        }
                    } else {
                        double numeric_val = 0.0;
                        if (val.type == TYPE_STRING) {
                            numeric_val = gw_str_val(&val.str);
                            gw_str_free(&val.str);
                        } else {
                            numeric_val = val.d_val;
                        }
                        if (arr->type == TYPE_INTEGER) {
                            *(int16_t *)ptr = (int16_t)numeric_val;
                        } else if (arr->type == TYPE_SINGLE) {
                            *(float *)ptr = (float)numeric_val;
                        } else if (arr->type == TYPE_DOUBLE) {
                            *(double *)ptr = numeric_val;
                        }
                    }
                } else {
                    gw_error(9);
                    if (val.type == TYPE_STRING) gw_str_free(&val.str);
                }
            } else {
                gw_error(8);
                if (val.type == TYPE_STRING) gw_str_free(&val.str);
            }
        } else {
            GW_Var *var = gw_var_get(state, var_name, true);
            if (var) {
                if (var->type == TYPE_STRING) {
                    if (val.type == TYPE_STRING) {
                        gw_var_set_str(state, var, val.str.ptr, val.str.len);
                        gw_str_free(&val.str);
                    } else {
                        gw_error(13);
                    }
                } else {
                    double numeric_val = 0.0;
                    if (val.type == TYPE_STRING) {
                        numeric_val = gw_str_val(&val.str);
                        gw_str_free(&val.str);
                    } else {
                        numeric_val = val.d_val;
                    }
                    gw_var_set_num(var, numeric_val);
                }
            }
        }
        
        while (*state->data_ip == ' ' || *state->data_ip == '\t') state->data_ip++;
        if (*state->data_ip == ',') {
            state->data_ip++;
        } else {
            while (*state->data_ip && *state->data_ip != ':' && *state->data_ip != '\n') {
                state->data_ip++;
            }
            if (*state->data_ip == ':') {
                state->data_ip++;
            } else {
                state->data_line = state->data_line->next;
                state->data_ip = NULL;
            }
        }
        
        while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
        if (*state->ip == ',') {
            state->ip++;
        }
    }
}

static void handle_input(GW_State *state) {
    bool no_newline = false;
    while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
    
    if (*state->ip == ';') {
        no_newline = true;
        state->ip++;
    }
    
    while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
    
    const char *prompt = NULL;
    size_t prompt_len = 0;
    bool append_question = true;
    
    if (*state->ip == '"') {
        state->ip++;
        prompt = (const char *)state->ip;
        while (*state->ip && *state->ip != '"') state->ip++;
        prompt_len = (const char *)state->ip - prompt;
        if (*state->ip == '"') state->ip++;
        
        while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
        if (*state->ip == ';') {
            append_question = true;
            state->ip++;
        } else if (*state->ip == ',') {
            append_question = false;
            state->ip++;
        } else {
            gw_error(2);
            return;
        }
    }
    
    if (prompt) {
        printf("%.*s", (int)prompt_len, prompt);
    }
    if (append_question) {
        const char *custom_prompt = keyword_prop_get((int)TOK_INPUT, "PROMPT");
        if (custom_prompt && !prompt) {
            printf("%s", custom_prompt);
        } else {
            printf("? ");
        }
    }

    fflush(stdout);
    
    char user_input[256];
    gw_console_read_line(user_input, sizeof(user_input));
    
    char *input_ptr = user_input;
    
    while (state->ip && *state->ip && *state->ip != ':' && *state->ip != '\n' && *state->ip != TOK_ELSE) {
        while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
        if (!*state->ip || *state->ip == ':' || *state->ip == '\n' || *state->ip == TOK_ELSE) break;
        
        char var_name[40];
        int idx = 0;
        while (isalnum(*state->ip) || *state->ip == '%' || *state->ip == '!' || *state->ip == '#' || *state->ip == '$') {
            if (idx < 39) var_name[idx++] = *state->ip;
            state->ip++;
        }
        var_name[idx] = '\0';
        
        bool is_array = false;
        int indices[10];
        int num_dims = 0;
        
        uint8_t *check_ptr = state->ip;
        while (*check_ptr == ' ' || *check_ptr == '\t') check_ptr++;
        if (*check_ptr == '(') {
            is_array = true;
            state->ip = check_ptr + 1;
            while (num_dims < 10) {
                GW_Value idx_val;
                gw_eval_expr(state, &idx_val);
                indices[num_dims++] = gw_val_get_int(&idx_val);
                if (idx_val.type == TYPE_STRING) gw_str_free(&idx_val.str);
                
                while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
                if (*state->ip == ',') {
                    state->ip++;
                } else if (*state->ip == ')') {
                    state->ip++;
                    break;
                } else {
                    gw_error(2);
                    return;
                }
            }
        }
        
        while (*input_ptr == ' ' || *input_ptr == '\t') input_ptr++;
        
        GW_Value val;
        val.type = TYPE_NONE;
        
        if (*input_ptr == '"') {
            input_ptr++;
            const char *start = input_ptr;
            while (*input_ptr && *input_ptr != '"') input_ptr++;
            size_t len = input_ptr - start;
            if (*input_ptr == '"') input_ptr++;
            val.type = TYPE_STRING;
            val.str = gw_str_create(start, len);
        } else {
            const char *start = input_ptr;
            while (*input_ptr && *input_ptr != ',') {
                input_ptr++;
            }
            size_t len = input_ptr - start;
            
            while (len > 0 && isspace((unsigned char)start[0])) {
                start++;
                len--;
            }
            while (len > 0 && isspace((unsigned char)start[len - 1])) {
                len--;
            }
            
            bool is_num = true;
            if (len == 0) {
                is_num = false;
            } else {
                for (size_t i = 0; i < len; i++) {
                    char c = start[i];
                    if (!isdigit((unsigned char)c) && c != '.' && c != '-' && c != '+' && c != 'e' && c != 'E') {
                        is_num = false;
                        break;
                    }
                }
            }
            
            if (is_num) {
                char tmp[64];
                if (len > 63) len = 63;
                memcpy(tmp, start, len);
                tmp[len] = '\0';
                val.type = TYPE_DOUBLE;
                val.d_val = strtod(tmp, NULL);
            } else {
                val.type = TYPE_STRING;
                val.str = gw_str_create(start, len);
            }
        }
        
        if (is_array) {
            GW_Array *arr = gw_arr_get(state, var_name, true);
            if (arr) {
                void *ptr = gw_arr_index_ptr(arr, num_dims, indices);
                if (ptr) {
                    if (arr->type == TYPE_STRING) {
                        if (val.type == TYPE_STRING) {
                            GW_String *s_ptr = (GW_String *)ptr;
                            if (s_ptr->ptr) free(s_ptr->ptr);
                            s_ptr->ptr = (char *)malloc(val.str.len + 1);
                            if (s_ptr->ptr) {
                                memcpy(s_ptr->ptr, val.str.ptr, val.str.len);
                                s_ptr->ptr[val.str.len] = '\0';
                                s_ptr->len = val.str.len;
                            }
                            gw_str_free(&val.str);
                        } else {
                            gw_error(13);
                        }
                    } else {
                        double numeric_val = 0.0;
                        if (val.type == TYPE_STRING) {
                            numeric_val = gw_str_val(&val.str);
                            gw_str_free(&val.str);
                        } else {
                            numeric_val = val.d_val;
                        }
                        if (arr->type == TYPE_INTEGER) {
                            *(int16_t *)ptr = (int16_t)numeric_val;
                        } else if (arr->type == TYPE_SINGLE) {
                            *(float *)ptr = (float)numeric_val;
                        } else if (arr->type == TYPE_DOUBLE) {
                            *(double *)ptr = numeric_val;
                        }
                    }
                } else {
                    gw_error(9);
                    if (val.type == TYPE_STRING) gw_str_free(&val.str);
                }
            } else {
                gw_error(8);
                if (val.type == TYPE_STRING) gw_str_free(&val.str);
            }
        } else {
            GW_Var *var = gw_var_get(state, var_name, true);
            if (var) {
                if (var->type == TYPE_STRING) {
                    if (val.type == TYPE_STRING) {
                        gw_var_set_str(state, var, val.str.ptr, val.str.len);
                        gw_str_free(&val.str);
                    } else {
                        gw_error(13);
                    }
                } else {
                    double numeric_val = 0.0;
                    if (val.type == TYPE_STRING) {
                        numeric_val = gw_str_val(&val.str);
                        gw_str_free(&val.str);
                    } else {
                        numeric_val = val.d_val;
                    }
                    gw_var_set_num(var, numeric_val);
                }
            }
        }
        
        if (*input_ptr == ',') {
            input_ptr++;
        }
        
        while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
        if (*state->ip == ',') {
            state->ip++;
        }
    }
}

static void handle_locate(GW_State *state) {
    while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
    
    int row = state->cursor_y + 1;
    int col = state->cursor_x + 1;
    
    if (*state->ip && *state->ip != ',' && *state->ip != ':' && *state->ip != '\n' && *state->ip != TOK_ELSE) {
        GW_Value row_val;
        gw_eval_expr(state, &row_val);
        row = gw_val_get_int(&row_val);
        if (row_val.type == TYPE_STRING) gw_str_free(&row_val.str);
    }
    
    while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
    if (*state->ip == ',') {
        state->ip++;
        while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
        if (*state->ip && *state->ip != ',' && *state->ip != ':' && *state->ip != '\n' && *state->ip != TOK_ELSE) {
            GW_Value col_val;
            gw_eval_expr(state, &col_val);
            col = gw_val_get_int(&col_val);
            if (col_val.type == TYPE_STRING) gw_str_free(&col_val.str);
        }
    }
    
    while (*state->ip && *state->ip != ':' && *state->ip != '\n' && *state->ip != TOK_ELSE) {
        state->ip++;
    }
    
    state->cursor_y = row - 1;
    state->cursor_x = col - 1;
    gw_sdl2_set_cursor(state->cursor_x, state->cursor_y);
    
    printf("\033[%d;%dH", row, col);
    fflush(stdout);
}

static void handle_key(GW_State *state) {
    while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
    
    if (isdigit(*state->ip)) {
        GW_Value n_val;
        gw_eval_expr(state, &n_val);
        int key_num = gw_val_get_int(&n_val);
        if (n_val.type == TYPE_STRING) gw_str_free(&n_val.str);
        
        while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
        if (*state->ip == ',') state->ip++;
        
        GW_Value str_val;
        gw_eval_expr(state, &str_val);
        if (str_val.type == TYPE_STRING) gw_str_free(&str_val.str);
    } else {
        char name[40];
        int idx = 0;
        while (isalnum(*state->ip)) {
            if (idx < 39) name[idx++] = *state->ip;
            state->ip++;
        }
        name[idx] = '\0';
        
        for (int i = 0; name[i]; i++) {
            name[i] = toupper((unsigned char)name[i]);
        }
        
        if (strcmp(name, "ON") == 0) {
            state->key_display_on = true;
        } else if (strcmp(name, "OFF") == 0) {
            state->key_display_on = false;
        } else if (strcmp(name, "LIST") == 0) {
            for (int i = 1; i <= 10; i++) {
                printf("F%d: \n", i);
            }
        }
    }
}

static void resolve_relative_path(GW_State *state, const char *path, char *resolved_path, size_t max_len) {
    FILE *f = fopen(path, "rb");
    if (f) {
        fclose(f);
        strncpy(resolved_path, path, max_len - 1);
        resolved_path[max_len - 1] = '\0';
        return;
    }
    
    if (state->current_program_path[0]) {
        char dir[256];
        strncpy(dir, state->current_program_path, sizeof(dir) - 1);
        dir[sizeof(dir) - 1] = '\0';
        
        char *slash = strrchr(dir, '\\');
        char *fslash = strrchr(dir, '/');
        char *last_sep = (slash > fslash) ? slash : fslash;
        if (last_sep) {
            *(last_sep + 1) = '\0';
            snprintf(resolved_path, max_len, "%s%s", dir, path);
            return;
        }
    }
    
    strncpy(resolved_path, path, max_len - 1);
    resolved_path[max_len - 1] = '\0';
}

static bool gw_load_file_ex(GW_State *state, const char *path, bool merge) {
    if (!merge) {
        gw_program_clear(state);
    }
    
    char cleaned_path[256];
    strncpy(cleaned_path, path, sizeof(cleaned_path) - 1);
    cleaned_path[sizeof(cleaned_path) - 1] = '\0';
    
    size_t plen = strlen(cleaned_path);
    while (plen > 0 && isspace((unsigned char)cleaned_path[plen - 1])) {
        cleaned_path[--plen] = '\0';
    }
    
    char *dot = strrchr(cleaned_path, '.');
    char *slash = strrchr(cleaned_path, '\\');
    char *fslash = strrchr(cleaned_path, '/');
    char *last_sep = (slash > fslash) ? slash : fslash;
    if (!dot || (last_sep && dot < last_sep)) {
        strncat(cleaned_path, ".BAS", sizeof(cleaned_path) - strlen(cleaned_path) - 1);
    }
    
    char resolved[256];
    resolve_relative_path(state, cleaned_path, resolved, sizeof(resolved));
    
    if (!merge) {
        strncpy(state->current_program_path, resolved, sizeof(state->current_program_path) - 1);
        state->current_program_path[sizeof(state->current_program_path) - 1] = '\0';
    }
    
    FILE *f = fopen(resolved, "rb");
    if (!f) {
        return false;
    }
    
    int first = fgetc(f);
    if (first == 0xFF) {
        while (!feof(f)) {
            uint16_t offset = 0;
            if (fread(&offset, 2, 1, f) < 1 || offset == 0) break;
            uint16_t line_num = 0;
            fread(&line_num, 2, 1, f);
            uint8_t temp[BUFLEN + 1];
            int idx = 0;
            if (!gw_read_binary_line(f, temp, BUFLEN + 1, &idx)) break;
            
            char ascii_line[BUFLEN * 4 + 1];
            gw_detokenize_binary(temp, idx, ascii_line, sizeof(ascii_line));
            uint8_t crunched[BUFLEN + 1];
            size_t crunched_len = gw_crunch(ascii_line, crunched, sizeof(crunched));
            gw_program_insert(state, line_num, crunched, crunched_len);
        }
    } else {
        if (first != EOF) ungetc(first, f);
        char line_buf[BUFLEN + 1];
        while (fgets(line_buf, sizeof(line_buf), f)) {
            size_t l_len = strlen(line_buf);
            while (l_len > 0 && (line_buf[l_len - 1] == '\n' || line_buf[l_len - 1] == '\r')) {
                line_buf[--l_len] = '\0';
            }
            char *start = line_buf;
            while (*start == ' ' || *start == '\t') start++;
            char *endptr;
            long line_num = strtol(start, &endptr, 10);
            if (endptr != start && line_num >= 0) {
                while (*endptr == ' ' || *endptr == '\t') endptr++;
                uint8_t crunched[BUFLEN + 1];
                size_t crunched_len = gw_crunch(endptr, crunched, sizeof(crunched));
                gw_program_insert(state, line_num, crunched, crunched_len);
            }
        }
    }
    fclose(f);
    return true;
}

static bool gw_load_file(GW_State *state, const char *path) {
    return gw_load_file_ex(state, path, false);
}

static uint8_t *find_next_statement(uint8_t *ip) {
    if (!ip) return NULL;
    uint8_t *p = ip;
    while (*p && *p != '\n') {
        if (*p == '"') {
            p++;
            while (*p && *p != '"' && *p != '\n') p++;
            if (*p == '"') p++;
            continue;
        }
        if (*p == ':') {
            p++; // skip the ':'
            // skip spaces
            while (*p == ' ' || *p == '\t') p++;
            return p;
        }
        p++;
    }
    return NULL;
}

static void handle_on(GW_State *state) {
    while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
    
    if (*state->ip == TOK_ERROR) {
        state->ip++; // skip TOK_ERROR
        while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
        
        if (*state->ip == TOK_GOTO) {
            state->ip++; // skip TOK_GOTO
            while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
            
            GW_Value val;
            gw_eval_expr(state, &val);
            int32_t target_line = (int32_t)gw_val_get_double(&val);
            if (val.type == TYPE_STRING) gw_str_free(&val.str);
            
            state->error_handler_line = target_line;
        } else {
            gw_error(2); // Syntax error
        }
    } else if (*state->ip == TOK_TIMER) {
        state->ip++; // skip TOK_TIMER
        while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
        
        int timer_idx = 0; // Default index if none provided
        
        if (*state->ip == '(') {
            state->ip++;
            GW_Value val;
            gw_eval_expr(state, &val);
            timer_idx = (int)gw_val_get_double(&val);
            if (val.type == TYPE_STRING) gw_str_free(&val.str);
            while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
            if (*state->ip == ')') state->ip++;
            while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
        }
        
        if (*state->ip == TOK_GOSUB) {
            state->ip++; // skip GOSUB
            while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
            
            GW_Value val;
            gw_eval_expr(state, &val);
            int32_t target_line = (int32_t)gw_val_get_double(&val);
            if (val.type == TYPE_STRING) gw_str_free(&val.str);
            
            extern void gw_event_register_timer(GW_State *state, int timer_idx, int32_t gosub_line);
            gw_event_register_timer(state, timer_idx, target_line);
        } else {
            gw_error(2); // Syntax error
        }
    } else {
        // ON expression GOTO/GOSUB line1, line2...
        GW_Value expr_val;
        gw_eval_expr(state, &expr_val);
        int idx = (int)gw_val_get_double(&expr_val);
        if (expr_val.type == TYPE_STRING) gw_str_free(&expr_val.str);
        
        while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
        
        uint8_t branch_tok = *state->ip;
        if (branch_tok != TOK_GOTO && branch_tok != TOK_GOSUB) {
            gw_error(2); // Syntax error
            return;
        }
        state->ip++; // skip GOTO/GOSUB
        
        int current_idx = 1;
        int32_t selected_line = -1;
        
        while (1) {
            while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
            
            GW_Value line_val;
            gw_eval_expr(state, &line_val);
            int32_t line_num = (int32_t)gw_val_get_double(&line_val);
            if (line_val.type == TYPE_STRING) gw_str_free(&line_val.str);
            
            if (current_idx == idx) {
                selected_line = line_num;
            }
            
            while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
            if (*state->ip == ',') {
                state->ip++;
                current_idx++;
            } else {
                break;
            }
        }
        
        if (selected_line != -1) {
            GW_Line *l = find_line(state, selected_line);
            if (l) {
                if (branch_tok == TOK_GOSUB) {
                    if (g_gosub_ptr < 128) {
                        g_gosub_stack[g_gosub_ptr] = state->current_line;
                        g_gosub_ip_stack[g_gosub_ptr] = state->ip;
                        g_gosub_ptr++;
                    } else {
                        gw_error(7); // Out of memory
                        return;
                    }
                }
                state->current_line = l;
                state->ip = l->tokens;
            } else {
                gw_error(8); // Undefined line number
            }
        }
    }
}

static void handle_resume(GW_State *state) {
    while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
    
    if (*state->ip == TOK_NEXT) {
        state->ip++; // skip TOK_NEXT
        
        if (!state->in_error_handler) {
            gw_error(20); // RESUME without error
            return;
        }
        
        state->in_error_handler = false;
        
        uint8_t *next_stmt = find_next_statement(state->error_ip);
        if (next_stmt) {
            state->current_line = state->error_line_ptr;
            state->ip = next_stmt;
        } else {
            if (state->error_line_ptr) {
                state->current_line = state->error_line_ptr->next;
                if (state->current_line) {
                    state->ip = state->current_line->tokens;
                } else {
                    state->ip = NULL;
                }
            } else {
                state->current_line = NULL;
                state->ip = NULL;
            }
        }
    } else if (isdigit(*state->ip) || *state->ip == '-' || *state->ip == '.') {
        GW_Value val;
        gw_eval_expr(state, &val);
        int32_t target_line = (int32_t)gw_val_get_double(&val);
        if (val.type == TYPE_STRING) gw_str_free(&val.str);
        
        if (!state->in_error_handler) {
            gw_error(20); // RESUME without error
            return;
        }
        
        state->in_error_handler = false;
        
        if (target_line == 0) {
            state->current_line = state->error_line_ptr;
            state->ip = state->error_ip;
        } else {
            GW_Line *l = find_line(state, target_line);
            if (l) {
                state->current_line = l;
                state->ip = l->tokens;
            } else {
                gw_error(8); // Undefined line number
            }
        }
    } else {
        // RESUME or RESUME 0
        if (!state->in_error_handler) {
            gw_error(20); // RESUME without error
            return;
        }
        
        state->in_error_handler = false;
        
        state->current_line = state->error_line_ptr;
        state->ip = state->error_ip;
    }
    state->jump_pending = true;
}

static void handle_error_stmt(GW_State *state) {
    GW_Value val;
    gw_eval_expr(state, &val);
    int code = gw_val_get_int(&val);
    if (val.type == TYPE_STRING) gw_str_free(&val.str);
    
    gw_error(code);
}

static void handle_def_type(GW_State *state, GW_Type type) {
    while (1) {
        while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
        
        if (!isalpha(*state->ip)) {
            gw_error(2); // Syntax error
            return;
        }
        
        char start_c = toupper((unsigned char)*state->ip);
        state->ip++;
        char end_c = start_c;
        
        while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
        
        if (*state->ip == '-') {
            state->ip++; // skip '-'
            while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
            if (!isalpha(*state->ip)) {
                gw_error(2); // Syntax error
                return;
            }
            end_c = toupper((unsigned char)*state->ip);
            state->ip++;
        }
        
        if (start_c >= 'A' && start_c <= 'Z' && end_c >= 'A' && end_c <= 'Z') {
            char low = (start_c < end_c) ? start_c : end_c;
            char high = (start_c > end_c) ? start_c : end_c;
            for (char c = low; c <= high; c++) {
                state->default_types[c - 'A'] = type;
            }
        } else {
            gw_error(2); // Syntax error
            return;
        }
        
        while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
        if (*state->ip == ',') {
            state->ip++;
        } else {
            break;
        }
    }
}

static void handle_chain(GW_State *state) {
    while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
    
    bool merge = false;
    if (*state->ip == TOK_MERGE) {
        merge = true;
        state->ip++;
        while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
    }
    
    GW_Value val;
    gw_eval_expr(state, &val);
    if (val.type != TYPE_STRING || !val.str.ptr) {
        gw_error(13); // Type mismatch
        if (val.type == TYPE_STRING) gw_str_free(&val.str);
        return;
    }
    
    char path[256];
    snprintf(path, sizeof(path), "%.*s", (int)val.str.len, val.str.ptr);
    gw_str_free(&val.str);
    
    int32_t target_line = -1;
    while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
    if (*state->ip == ',') {
        state->ip++;
        while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
        
        if (isdigit(*state->ip) || *state->ip == '-' || *state->ip == '.') {
            GW_Value line_val;
            gw_eval_expr(state, &line_val);
            target_line = (int32_t)gw_val_get_double(&line_val);
            if (line_val.type == TYPE_STRING) gw_str_free(&line_val.str);
        }
    }
    
    bool all = false;
    while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
    if (*state->ip == ',') {
        state->ip++;
        while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
        
        if (toupper(state->ip[0]) == 'A' && toupper(state->ip[1]) == 'L' && toupper(state->ip[2]) == 'L') {
            all = true;
            state->ip += 3;
        }
    }
    
    if (!all) {
        gw_vars_clear(state);
    }
    
    // printf("[DEBUG] handle_chain: path='%s', merge=%d, target_line=%d, all=%d\n", path, merge, target_line, all);
    // fflush(stdout);
    
    bool load_ok = gw_load_file_ex(state, path, merge);
    // printf("[DEBUG] handle_chain: gw_load_file_ex returned %d\n", load_ok);
    // fflush(stdout);
    if (!load_ok) {
        gw_error(5); // Illegal function call
        return;
    }
    
    if (target_line != -1) {
        GW_Line *l = find_line(state, target_line);
        // printf("[DEBUG] handle_chain: target line %d pointer = %p\n", target_line, (void*)l);
        // fflush(stdout);
        if (l) {
            state->current_line = l;
            state->ip = l->tokens;
        } else {
            gw_error(8); // Undefined line number
            return;
        }
    } else {
        if (state->program_head) {
            state->current_line = state->program_head;
            state->ip = state->program_head->tokens;
        } else {
            state->current_line = NULL;
            state->ip = NULL;
            state->execution_active = false;
            return;
        }
    }
    
    state->chain_jump_pending = true;
}

static void handle_merge(GW_State *state) {
    while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
    GW_Value val;
    gw_eval_expr(state, &val);
    if (val.type != TYPE_STRING || !val.str.ptr) {
        gw_error(13); // Type mismatch
        if (val.type == TYPE_STRING) gw_str_free(&val.str);
        return;
    }
    
    char path[256];
    snprintf(path, sizeof(path), "%.*s", (int)val.str.len, val.str.ptr);
    gw_str_free(&val.str);
    
    if (!gw_load_file_ex(state, path, true)) {
        gw_error(5); // Illegal function call
    }
}


static void handle_open(GW_State *state) {
    printf("HANDLE_OPEN_CALLED\n");
    int mode = 1; // 1=read, 2=write, 8=append, 4=random
    char filename[256] = {0};
    int channel = 1;
    
    // Very rudimentary parser just to consume the tokens to avoid Syntax Error
    // We will advance state->ip until we hit ':' or '\n'
    while (*state->ip && *state->ip != ':' && *state->ip != '\n' && *state->ip != TOK_ELSE) {
        state->ip++;
    }
    
    // In a real implementation we would call gw_file_open(state, channel, filename, mode, 128);
}

static void handle_close(GW_State *state) {
    while (*state->ip && *state->ip != ':' && *state->ip != '\n' && *state->ip != TOK_ELSE) {
        state->ip++;
    }
}

static void handle_shell(GW_State *state) {
    if (*state->ip == ':' || *state->ip == '\n' || *state->ip == TOK_ELSE || *state->ip == 0) {
#ifdef _WIN32
        system("cmd");
#else
        system("/bin/sh");
#endif
        return;
    }
    
    GW_Value cmd;
    gw_eval_expr(state, &cmd);
    if (state->jump_pending) return;
    if (cmd.type == TYPE_STRING && cmd.str.ptr) {
        char buf[512];
        snprintf(buf, sizeof(buf), "%.*s", (int)cmd.str.len, cmd.str.ptr);
        system(buf);
        gw_str_free(&cmd.str);
    } else {
        if (cmd.type == TYPE_STRING) gw_str_free(&cmd.str);
        gw_error(13); // Type mismatch
    }
}

static void handle_exec(GW_State *state) {
    GW_Value cmd;
    gw_eval_expr(state, &cmd);
    if (state->jump_pending) return;
    if (cmd.type == TYPE_STRING && cmd.str.ptr) {
        char buf[512];
#ifdef _WIN32
        snprintf(buf, sizeof(buf), "start \"\" %.*s", (int)cmd.str.len, cmd.str.ptr);
#else
        snprintf(buf, sizeof(buf), "%.*s &", (int)cmd.str.len, cmd.str.ptr);
#endif
        system(buf);
        gw_str_free(&cmd.str);
    } else {
        if (cmd.type == TYPE_STRING) gw_str_free(&cmd.str);
        gw_error(13); // Type mismatch
    }
}

static void handle_environ(GW_State *state) {
    GW_Value env;
    gw_eval_expr(state, &env);
    if (state->jump_pending) return;
    if (env.type == TYPE_STRING && env.str.ptr) {
        char buf[512];
        snprintf(buf, sizeof(buf), "%.*s", (int)env.str.len, env.str.ptr);
        if (strchr(buf, '=')) {
#ifdef _WIN32
            _putenv(buf);
#else
            putenv(plat_strdup(buf));
#endif
        } else {
            gw_error(5); // Illegal function call
        }
        gw_str_free(&env.str);
    } else {
        if (env.type == TYPE_STRING) gw_str_free(&env.str);
        gw_error(13); // Type mismatch
    }
}

static void handle_sandbox(GW_State *state) {
    GW_Value level;
    gw_eval_expr(state, &level);
    if (state->jump_pending) return;
    // Consume optional LEVEL if present (we'll just evaluate and consume)
    // Actually, sandbox doesn't need to do much right now
}

static void handle_module(GW_State *state) {
    GW_Value mod;
    gw_eval_expr(state, &mod);
    if (!state->jump_pending) {
        if (mod.type == TYPE_STRING) gw_str_free(&mod.str);
    }
}

static void handle_import(GW_State *state) {
    GW_Value mod;
    gw_eval_expr(state, &mod);
    if (!state->jump_pending) {
        if (mod.type == TYPE_STRING && mod.str.ptr) {
            char name[256];
            snprintf(name, sizeof(name), "%.*s", (int)mod.str.len, mod.str.ptr);
            extern int module_activate(const char *, void *);
            module_activate(name, state);
            gw_str_free(&mod.str);
        } else if (mod.type == TYPE_STRING) {
            gw_str_free(&mod.str);
        }
    }
}

static void handle_export(GW_State *state) {
    // Parses and skips variable list for EXPORT mapping
    while (*state->ip && *state->ip != ':' && *state->ip != '\n' && *state->ip != TOK_ELSE) {
        state->ip++;
    }
}

static void handle_plugin(GW_State *state) {
    GW_Value plug;
    gw_eval_expr(state, &plug);
    if (!state->jump_pending) {
        if (plug.type == TYPE_STRING && plug.str.ptr) {
            char path[256];
            snprintf(path, sizeof(path), "%.*s", (int)plug.str.len, plug.str.ptr);
            extern int module_load_dynamic(const char *);
            module_load_dynamic(path);
            gw_str_free(&plug.str);
        } else if (plug.type == TYPE_STRING) {
            gw_str_free(&plug.str);
        }
    }
}

static void handle_memmap(GW_State *state) {
    GW_Value val;
    gw_eval_expr(state, &val);
    if (state->jump_pending) return;
    
    if (val.type == TYPE_STRING && val.str.ptr) {
        char buf[64];
        snprintf(buf, sizeof(buf), "%.*s", (int)val.str.len, val.str.ptr);
        MemMapType mtype = memmap_from_string(buf, (int)val.str.len);
        if (mtype != MMAP_COUNT) {
            extern uint8_t *gw_mem_get_buffer(struct GW_Memory *mem);
            uint8_t *buffer = gw_mem_get_buffer(state->mem_sys);
            if (buffer) {
                memmap_init((unsigned char *)buffer, mtype);
            } else {
                gw_error(5); // Illegal function call
            }
        } else {
            gw_error(5); // Illegal function call
        }
        gw_str_free(&val.str);
    } else {
        gw_error(13); // Type mismatch
    }
}

void gw_exec_statement(GW_State *state) {
    if (!state || !state->ip || !state->execution_active || state->error_jump_pending || state->chain_jump_pending || state->jump_pending) return;
    
    // Safety check for runaway execution
    if (state->exec_count++ > 100000) {
        gw_sdl2_poll_events();
        state->exec_count = 0;
    }
    
    state->stmt_ip = state->ip;
    
    // Skip spaces
    while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
    
    if (!*state->ip) return;
    
    uint16_t full_tok = *state->ip;
    if (full_tok == 0xFE || full_tok == 0xFD || full_tok == 0xFF) {
        state->ip++;
        if (*state->ip) {
            full_tok = (full_tok << 8) | *state->ip;
        }
    }
    
    switch (full_tok) {
        case TOK_PRINT:
            state->ip++;
            handle_print(state);
            break;
        case TOK_LET:
            handle_let(state);
            break;
        case TOK_DIM:
            state->ip++;
            handle_dim(state);
            break;
        case TOK_DATA:
            while (state->ip && *state->ip && *state->ip != ':' && *state->ip != '\n' && *state->ip != TOK_ELSE) {
                if (*state->ip == '"') {
                    state->ip++;
                    while (*state->ip && *state->ip != '"') state->ip++;
                    if (*state->ip == '"') state->ip++;
                } else {
                    state->ip++;
                }
            }
            break;
        case TOK_READ:
            state->ip++;
            handle_read(state);
            break;
        case TOK_RESTORE:
            state->ip++;
            handle_restore(state);
            break;
        case TOK_INPUT:
            state->ip++;
            handle_input(state);
            break;
        case TOK_LOCATE:
            state->ip++;
            handle_locate(state);
            break;
        case TOK_ON:
            state->ip++;
            handle_on(state);
            break;
        case TOK_RESUME:
            state->ip++;
            handle_resume(state);
            break;
        case TOK_ERROR:
            state->ip++;
            handle_error_stmt(state);
            break;
        case TOK_KEY:
            state->ip++;
            handle_key(state);
            break;
        case TOK_GOTO:
            state->ip++;
            handle_goto(state);
            break;
        case TOK_GOSUB:
            state->ip++;
            handle_gosub(state);
            break;
        case TOK_RETURN:
            state->ip++;
            handle_return(state);
            break;
        case TOK_POKE:
            state->ip++;
            handle_poke(state);
            break;
        case TOK_DEF:
            state->ip++;
            handle_def_seg(state);
            break;
        case TOK_DEFINT:
            state->ip++;
            handle_def_type(state, TYPE_INTEGER);
            break;
        case TOK_DEFSNG:
            state->ip++;
            handle_def_type(state, TYPE_SINGLE);
            break;
        case TOK_DEFDBL:
            state->ip++;
            handle_def_type(state, TYPE_DOUBLE);
            break;
        case TOK_DEFSTR:
            state->ip++;
            handle_def_type(state, TYPE_STRING);
            break;
        case TOK_SCREEN:
            state->ip++;
            handle_screen(state);
            break;
        case TOK_COLOR:
            state->ip++;
            handle_color(state);
            break;
        case TOK_LINE:
            state->ip++;
            handle_line(state);
            break;
        case TOK_CIRCLE:
            state->ip++;
            handle_circle(state);
            break;
        case TOK_PAINT:
            state->ip++;
            handle_paint(state);
            break;
        case TOK_PSET:
            state->ip++;
            handle_pset_preset(state, false);
            break;
        case TOK_PRESET:
            state->ip++;
            handle_pset_preset(state, true);
            break;
        case TOK_GET:
            state->ip++;
            handle_get(state);
            break;
        case TOK_PUT:
            state->ip++;
            handle_put(state);
            break;
        case TOK_DRAW:
            state->ip++;
            handle_draw(state);
            break;
        case TOK_PLAY:
            state->ip++;
            handle_play(state);
            break;
        case TOK_SOUND:
            state->ip++;
            handle_sound(state);
            break;
        case TOK_BEEP:
            state->ip++;
            gw_sdl2_beep();
            break;
        case TOK_REM:
            while (state->ip && *state->ip && *state->ip != '\n') {
                state->ip++;
            }
            break;
        case TOK_CLS:
            state->ip++;
            if (state->screen_mode == 0) {
                gw_sdl2_clear_screen(GW_PALETTE[state->bg_color % 16]);
            } else {
                gw_sdl2_clear_screen(GW_PALETTE[0]);
            }
            printf("\033[2J\033[H");
            fflush(stdout);
            break;
        case TOK_CLEAR:
            state->ip++;
            gw_vars_clear(state);
            break;
        case TOK_WIDTH:
            state->ip++;
            handle_width(state);
            break;
        case TOK_FILES:
            state->ip++;
            handle_files(state);
            break;
        case TOK_CONSOLE: {
            state->ip++;
            int start = -1;
            int lines = -1;
            int fn_keys = -1;
            int mono = -1;
            
            while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
            
            if (*state->ip && *state->ip != ',' && *state->ip != ':' && *state->ip != '\n' && *state->ip != TOK_ELSE) {
                GW_Value val;
                gw_eval_expr(state, &val);
                start = gw_val_get_int(&val);
                if (val.type == TYPE_STRING) gw_str_free(&val.str);
            }
            
            while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
            if (*state->ip == ',') {
                state->ip++;
                while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
                if (*state->ip && *state->ip != ',' && *state->ip != ':' && *state->ip != '\n' && *state->ip != TOK_ELSE) {
                    GW_Value val;
                    gw_eval_expr(state, &val);
                    lines = gw_val_get_int(&val);
                    if (val.type == TYPE_STRING) gw_str_free(&val.str);
                }
            }
            
            while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
            if (*state->ip == ',') {
                state->ip++;
                while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
                if (*state->ip && *state->ip != ',' && *state->ip != ':' && *state->ip != '\n' && *state->ip != TOK_ELSE) {
                    GW_Value val;
                    gw_eval_expr(state, &val);
                    fn_keys = gw_val_get_int(&val);
                    if (val.type == TYPE_STRING) gw_str_free(&val.str);
                }
            }
            
            while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
            if (*state->ip == ',') {
                state->ip++;
                while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
                if (*state->ip && *state->ip != ',' && *state->ip != ':' && *state->ip != '\n' && *state->ip != TOK_ELSE) {
                    GW_Value val;
                    gw_eval_expr(state, &val);
                    mono = gw_val_get_int(&val);
                    if (val.type == TYPE_STRING) gw_str_free(&val.str);
                }
            }
            
            gw_sdl2_set_console(start, lines, fn_keys, mono);
            break;
        }
        case TOK_SWAP: // SWAP
            state->ip++;
            handle_swap(state);
            break;
        case TOK_SUB:
            handle_sub_decl(state, TOK_SUB);
            break;
        case TOK_FUNCTION:
            handle_sub_decl(state, TOK_FUNCTION);
            break;
        case TOK_CALL:
            state->ip++;
            handle_call(state);
            break;
        case TOK_SHELL:
            state->ip++;
            handle_shell(state);
            break;
        case TOK_EXEC:
            state->ip++;
            handle_exec(state);
            break;
        case TOK_ENVIRON:
            state->ip++;
            handle_environ(state);
            break;
        case TOK_SANDBOX:
            state->ip++;
            handle_sandbox(state);
            break;
        case TOK_MODULE:
            state->ip++;
            handle_module(state);
            break;
        case TOK_IMPORT:
            state->ip++;
            handle_import(state);
            break;
        case TOK_EXPORT:
            state->ip++;
            handle_export(state);
            break;
        case TOK_PLUGIN:
            state->ip++;
            handle_plugin(state);
            break;
        case TOK_MEMMAP:
            state->ip++;
            handle_memmap(state);
            break;
        case TOK_IF:
        case TOK_UNLESS: {
            bool is_unless = (full_tok == TOK_UNLESS);
            state->ip++;
            GW_Value cond;
            gw_eval_expr(state, &cond);
            bool is_true = (gw_val_get_double(&cond) != 0.0);
            if (is_unless) is_true = !is_true;
            while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
            uint8_t branch_tok = *state->ip;
            if (branch_tok == TOK_THEN || branch_tok == TOK_GOTO) {
                state->ip++;
            } else {
                gw_error(2);
                break;
            }
            uint8_t *else_ptr = NULL;
            uint8_t *p = state->ip;
            int paren_depth = 0;
            while (*p && *p != '\n') {
                if (*p == '"') {
                    p++;
                    while (*p && *p != '"') p++;
                    if (*p == '"') p++;
                    continue;
                }
                if (*p == '(') paren_depth++;
                if (*p == ')') paren_depth--;
                if (paren_depth == 0 && *p == TOK_ELSE) {
                    else_ptr = p;
                    break;
                }
                p++;
            }
            if (is_true) {
                if (branch_tok == TOK_GOTO || isdigit(*state->ip)) {
                    handle_goto(state);
                } else {
                    while (state->ip && *state->ip && *state->ip != '\n' && state->ip != else_ptr) {
                        if (state->error_jump_pending || state->chain_jump_pending || state->jump_pending) break;
                        gw_exec_statement(state);
                    }
                    if (state->ip == else_ptr) {
                        state->ip = p;
                        while (*state->ip && *state->ip != '\n') state->ip++;
                    }
                }
            } else {
                if (else_ptr) {
                    state->ip = else_ptr + 1;
                    while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
                    if (isdigit(*state->ip)) {
                        handle_goto(state);
                    } else {
                        while (state->ip && *state->ip && *state->ip != '\n') {
                            if (state->error_jump_pending || state->chain_jump_pending || state->jump_pending) break;
                            gw_exec_statement(state);
                        }
                    }
                } else {
                    while (state->ip && *state->ip && *state->ip != '\n') state->ip++;
                }
            }
            break;
        }
        case TOK_FOR: {
            state->ip++;
            while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
            char name[40];
            int idx = 0;
            while (isalnum(*state->ip) || *state->ip == '%' || *state->ip == '!' || *state->ip == '#' || *state->ip == '$') {
                if (idx < 39) name[idx++] = *state->ip;
                state->ip++;
            }
            name[idx] = '\0';
            GW_Var *var = gw_var_get(state, name, true);
            if (!var || var->type == TYPE_STRING) {
                gw_error(13);
                break;
            }
            while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
            if (*state->ip == '=') state->ip++;
            GW_Value start_val;
            gw_eval_expr(state, &start_val);
            gw_var_set_num(var, gw_val_get_double(&start_val));
            while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
            if (*state->ip == TOK_TO) state->ip++;
            GW_Value limit_val;
            gw_eval_expr(state, &limit_val);
            double limit = gw_val_get_double(&limit_val);
            double step = 1.0;
            while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
            uint16_t step_tok = *state->ip;
            if (step_tok == 0xFE) {
                step_tok = (0xFE << 8) | *(state->ip + 1);
            }
            if (step_tok == TOK_STEP || step_tok == TOK_BY) {
                state->ip += (step_tok > 0xFF) ? 2 : 1;
                GW_Value step_val;
                gw_eval_expr(state, &step_val);
                step = gw_val_get_double(&step_val);
            }
            if (g_for_ptr < 32) {
                g_for_stack[g_for_ptr].var = var;
                g_for_stack[g_for_ptr].limit = limit;
                g_for_stack[g_for_ptr].step = step;
                g_for_stack[g_for_ptr].loop_line = state->current_line;
                g_for_stack[g_for_ptr].loop_ip = state->ip;
                g_for_ptr++;
            } else {
                gw_error(7);
            }
            break;
        }
        case TOK_NEXT: {
            state->ip++;
            while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
            char name[40];
            int idx = 0;
            while (isalnum(*state->ip) || *state->ip == '%' || *state->ip == '!' || *state->ip == '#' || *state->ip == '$') {
                if (idx < 39) name[idx++] = *state->ip;
                state->ip++;
            }
            name[idx] = '\0';
            int loop_idx = g_for_ptr - 1;
            if (idx > 0) {
                while (loop_idx >= 0) {
                    if (strcmp(g_for_stack[loop_idx].var->name, name) == 0) break;
                    loop_idx--;
                }
            }
            if (loop_idx >= 0) {
                GW_ForLoop *loop = &g_for_stack[loop_idx];
                double val = gw_var_get_num(loop->var);
                val += loop->step;
                gw_var_set_num(loop->var, val);
                bool done = false;
                if (loop->step >= 0 && val > loop->limit) done = true;
                if (loop->step < 0 && val < loop->limit) done = true;
                if (done) {
                    g_for_ptr = loop_idx;
                } else {
                    state->current_line = loop->loop_line;
                    state->ip = loop->loop_ip;
                }
            } else {
                gw_error(1);
            }
            break;
        }
        case TOK_WHILE: {
            uint8_t *start_ip = state->ip;
            state->ip++;
            GW_Line *cond_line = state->current_line;
            GW_Value cond;
            gw_eval_expr(state, &cond);
            bool is_true = (gw_val_get_double(&cond) != 0.0);
            if (is_true) {
                if (g_while_ptr < 32) {
                    g_while_stack[g_while_ptr].loop_line = cond_line;
                    g_while_stack[g_while_ptr].loop_ip = start_ip;
                    g_while_ptr++;
                } else {
                    gw_error(7);
                }
            } else {
                int depth = 1;
                while (state->current_line) {
                    while (state->ip && *state->ip) {
                        if (*state->ip == TOK_WHILE) depth++;
                        if (*state->ip == TOK_WEND) {
                            depth--;
                            if (depth == 0) {
                                state->ip++; // skip TOK_WEND
                                break;
                            }
                        }
                        state->ip++;
                    }
                    if (depth == 0) {
                        break;
                    }
                    state->current_line = state->current_line->next;
                    if (state->current_line) {
                        state->ip = state->current_line->tokens;
                    } else {
                        break;
                    }
                }
            }
            break;
        }
        case TOK_WEND: {
            state->ip++;
            if (g_while_ptr > 0) {
                g_while_ptr--;
                state->current_line = g_while_stack[g_while_ptr].loop_line;
                state->ip = g_while_stack[g_while_ptr].loop_ip;
            } else {
                gw_error(2);
            }
            break;
        }
        case TOK_LOAD: {
            state->ip++;
            GW_Value val;
            gw_eval_expr(state, &val);
            if (val.type == TYPE_STRING && val.str.ptr) {
                if (!gw_load_file(state, val.str.ptr)) {
                    gw_error(5);
                }
                gw_str_free(&val.str);
            } else {
                gw_error(13);
            }
            break;
        }
        case TOK_CHAIN:
            state->ip++;
            handle_chain(state);
            break;
        case TOK_MERGE:
            state->ip++;
            handle_merge(state);
            break;
        case TOK_RUN: {
            state->ip++;
            while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
            if (*state->ip && *state->ip != ':' && *state->ip != '\n' && *state->ip != TOK_ELSE) {
                GW_Value val;
                gw_eval_expr(state, &val);
                if (val.type == TYPE_STRING && val.str.ptr) {
                    if (gw_load_file(state, val.str.ptr)) {
                        gw_run_program(state);
                    } else {
                        gw_error(5);
                    }
                    gw_str_free(&val.str);
                } else {
                    gw_error(13);
                }
            } else {
                gw_run_program(state);
            }
            break;
        }
        case TOK_SAVE: {
            state->ip++;
            GW_Value val;
            gw_eval_expr(state, &val);
            if (val.type == TYPE_STRING && val.str.ptr) {
                FILE *f = fopen(val.str.ptr, "w");
                if (f) {
                    GW_Line *l = state->program_head;
                    while (l) {
                        char text[BUFLEN + 1];
                        gw_list(l->tokens, l->length, text, sizeof(text));
                        fprintf(f, "%d %s\n", l->line_num, text);
                        l = l->next;
                    }
                    fclose(f);
                } else {
                    gw_error(5);
                }
                gw_str_free(&val.str);
            } else {
                gw_error(13);
            }
            break;
        }
        case TOK_END: {
            state->ip++;
            while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
            uint16_t next_tok = *state->ip;
            if (next_tok == 0xFE || next_tok == 0xFD || next_tok == 0xFF) {
                if (*(state->ip+1)) {
                    next_tok = (next_tok << 8) | *(state->ip+1);
                }
            }
            if (next_tok == TOK_SUB || next_tok == TOK_FUNCTION) {
                if (next_tok > 0xFF) state->ip += 2; else state->ip++;
                if (state->call_stack) {
                    GW_CallFrame *frame = state->call_stack;
                    state->call_stack = frame->next;
                    
                    // Restore scope
                    GW_Var *v = state->variables;
                    while (v) {
                        GW_Var *next = v->next;
                        if (v->type == TYPE_STRING && v->str.ptr) free(v->str.ptr);
                        free(v);
                        v = next;
                    }
                    GW_Array *a = state->arrays;
                    while (a) {
                        GW_Array *next = a->next;
                        if (a->data) free(a->data);
                        free(a);
                        a = next;
                    }
                    
                    state->variables = frame->local_variables;
                    state->arrays = frame->local_arrays;
                    
                    state->current_line = frame->return_line;
                    state->ip = frame->return_ip;
                    state->jump_pending = true;
                    free(frame);
                } else {
                    gw_error(2); // Syntax error (END SUB without CALL)
                }
                break;
            }
            state->execution_active = false;
            break;
        }
        case TOK_STOP:
            state->execution_active = false;
            break;
        case ':':
            state->ip++; // Move past multi-statement separators
            break;
            
        case TOK_HELP:
            state->ip++;
            {
                // Skip spaces
                while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
                
                char topic[64] = {0};
                if (*state->ip == '"') {
                    state->ip++; // skip quote
                    int i = 0;
                    while (*state->ip && *state->ip != '"' && *state->ip != '\n' && i < 63) {
                        topic[i++] = *state->ip++;
                    }
                    if (*state->ip == '"') state->ip++;
                    topic[i] = '\0';
                } else if (*state->ip && *state->ip != ':' && *state->ip != '\n') {
                    // Raw string uncrunch
                    uint8_t temp[1024];
                    int tidx = 0;
                    while (*state->ip && *state->ip != ':' && *state->ip != '\n' && tidx < 1000) {
                        temp[tidx++] = *state->ip++;
                    }
                    temp[tidx] = 0;
                    char buf[1024];
                    gw_list(temp, tidx, buf, sizeof(buf));
                    // get first word
                    char *p = buf;
                    while (*p == ' ') p++;
                    int i = 0;
                    while (*p && *p != ' ' && i < 63) {
                        topic[i++] = toupper((unsigned char)*p++);
                    }
                    topic[i] = '\0';
                }
                
                // Need to extern or include help.h
                // For now, declare extern here
                extern void help_show(const char *topic);
                help_show(topic[0] ? topic : NULL);
            }
            break;
            
        case TOK_CATALOG:
            state->ip++;
            extern void help_catalog(void);
            help_catalog();
            break;
        
        case 0xBA: // OPEN
        case TOK_OPEN_BPP:
            state->ip++;
            handle_open(state);
            break;
        case 0xBB: // CLOSE
        case TOK_CLOSE_BPP:
            state->ip++;
            handle_close(state);
            break;
        case TOK_KEYWORD_BPP:
            state->ip++;
            {
                // Uncrunch the rest of the line so we don't have to deal with TOK_ON etc.
                char line_buf[1024];
                uint8_t temp_toks[1024];
                int tidx = 0;
                while (*state->ip && *state->ip != ':' && *state->ip != '\n' && tidx < 1000) {
                    temp_toks[tidx++] = *state->ip++;
                }
                temp_toks[tidx] = 0;
                gw_list(temp_toks, tidx, line_buf, sizeof(line_buf));
                
                // Now parse line_buf
                char prop[64] = {0};
                char val[64] = {0};
                char target_kw[64] = {0};
                int target_tok = 0;
                
                // We might have multiple spaces, and val might be a quoted string
                // Let's use a simple scanner
                char *p = line_buf;
                while (*p == ' ') p++;
                
                // read target_kw
                int i=0;
                while (*p && *p != ' ' && i < 63) target_kw[i++] = *p++;
                target_kw[i] = 0;
                
                while (*p == ' ') p++;
                
                // read prop
                i=0;
                while (*p && *p != ' ' && i < 63) prop[i++] = *p++;
                prop[i] = 0;
                
                while (*p == ' ') p++;
                
                // read val
                i=0;
                while (*p && *p != '\n' && i < 63) val[i++] = *p++;
                val[i] = 0;
                
                // trim trailing spaces
                while(i > 0 && val[i-1] == ' ') { i--; val[i] = 0; }
                
                // Trim quotes from val if present
                if (val[0] == '"') {
                    int vlen = (int)strlen(val);
                    if (vlen > 0 && val[vlen-1] == '"') {
                        val[vlen-1] = '\0';
                    }
                    memmove(val, val+1, vlen);
                }
                
                if (_stricmp(target_kw, "PRINT") == 0) target_tok = TOK_PRINT;
                else if (_stricmp(target_kw, "INPUT") == 0) target_tok = TOK_INPUT;
                
                if (target_tok != 0) {
                    if (_stricmp(val, "OFF") == 0) {
                        keyword_prop_remove((int)target_tok, prop);
                    } else if (_stricmp(prop, "RESET") == 0) {
                        keyword_prop_clear((int)target_tok);
                    } else {
                        keyword_prop_set((int)target_tok, prop, val);
                    }
                }
            }
            break;

        case TOK_VDEV:
            state->ip++;
            vdev_list_all();
            break;
        case TOK_VMEM:
            state->ip++;
            /* VMEM diagnostics disabled */
            break;
        case TOK_VNET:
            state->ip++;
            printf("VNET: Virtual Network Interface Active\n");
            break;
        case TOK_VCON:
            state->ip++;
            printf("VCON: Console active\n");
            break;
        case TOK_VTERM:
            state->ip++;
            printf("VTERM: Terminal active\n");
            break;
        case TOK_VMACH:
            state->ip++;
            printf("VMACH: Machine Stats\n");
            break;
        case TOK_DEVMAP:
            state->ip++;
            printf("DEVMAP: Device Map\n");
            break;
        case TOK_VER:
            state->ip++;
            {
                extern void help_version(void);
                help_version();
            }
            break;
        case TOK_DIALECT: {
            state->ip++;
            
            // Detokenize the rest of the statement to parse safely without keyword conflicts
            uint8_t *end_ip = state->ip;
            while (*end_ip && *end_ip != ':') end_ip++;
            size_t stmt_len = end_ip - state->ip;
            if (stmt_len > 255) stmt_len = 255;
            
            char dstmt[256];
            gw_detokenize_binary(state->ip, stmt_len, dstmt, sizeof(dstmt));
            
            // Advance IP to end of statement
            state->ip = end_ip;
            
            char dname[64] = {0};
            char bname[64] = {0};
            char cat[64] = {0};
            char kw[64] = {0};
            
            // 1. DIALECT "MYBASIC" ENABLE CATEGORY "NETWORK"
            if (sscanf(dstmt, " \"%63[^\"]\" ENABLE CATEGORY \"%63[^\"]\"", dname, cat) == 2) {
                if (dialect_runtime_enable_category(dname, cat) < 0) { gw_error(5); }
                break;
            }
            // 2. DIALECT "MYBASIC" DISABLE KEYWORD "PRINT"
            if (sscanf(dstmt, " \"%63[^\"]\" DISABLE KEYWORD \"%63[^\"]\"", dname, kw) == 2) {
                if (dialect_runtime_disable_keyword(dname, kw) < 0) { gw_error(5); }
                break;
            }
            // 3. DIALECT CREATE "MYBASIC" FROM "GW-BASIC"
            if (sscanf(dstmt, " CREATE \"%63[^\"]\" FROM \"%63[^\"]\"", dname, bname) == 2) {
                if (dialect_runtime_create(dname, bname) < 0) { gw_error(5); }
                break;
            }
            // 4. DIALECT "GW-BASIC"
            if (sscanf(dstmt, " \"%63[^\"]\"", dname) == 1) {
                int found = 0;
                if (found < 0) { gw_error(5); break; }
                
                
                printf("Dialect: %s [%s]\n", "BASIC++", "BPP");
                break;
            }
            // 5. DIALECT 3 (by id)
            int did = -1;
            if (sscanf(dstmt, " %d", &did) == 1) {
                if (did < 0 || did >= 100) { gw_error(5); break; }
                
                
                printf("Dialect: %s [%s]\n", "BASIC++", "BPP");
                break;
            }
            // 6. DIALECT LIST
            if (strncmp(dstmt, " LIST", 5) == 0 || dstmt[0] == '\0') {
                
                break;
            }
            
            gw_error(2); // Syntax error if nothing matched
            break;
        }
        default:

            // Check if variable assignment
            if (isalpha(full_tok)) {
                handle_let(state);
            } else {
                state->ip++; // Unknown/skipped token
            }
            break;
    }
}

static void gw_scan_subs(GW_State *state) {
    // Clear existing subs
    GW_SubDef *s = state->subs;
    while (s) {
        GW_SubDef *next = s->next;
        free(s);
        s = next;
    }
    state->subs = NULL;
    
    GW_Line *line = state->program_head;
    while (line) {
        uint8_t *ip = line->tokens;
        while (*ip) {
            uint16_t tok = *ip;
            if (tok == 0xFE || tok == 0xFD || tok == 0xFF) {
                if (*(ip+1)) {
                    tok = (tok << 8) | *(ip+1);
                    ip++;
                }
            }
            if (tok == TOK_SUB || tok == TOK_FUNCTION) {
                printf("[SCAN] Found SUB/FUNCTION: 0x%04X\n", tok);
                bool is_function = (tok == TOK_FUNCTION);
                ip++;
                while (*ip == ' ' || *ip == '\t') ip++;
                
                char name[40];
                int idx = 0;
                while (isalnum(*ip) || *ip == '%' || *ip == '!' || *ip == '#' || *ip == '$' || *ip == '.') {
                    if (idx < 39) name[idx++] = toupper(*ip);
                    ip++;
                }
                name[idx] = '\0';
                
                GW_SubDef *new_sub = (GW_SubDef *)calloc(1, sizeof(GW_SubDef));
                strncpy(new_sub->name, name, 39);
                new_sub->start_line = line;
                new_sub->start_ip = ip;
                new_sub->next = state->subs;
                state->subs = new_sub;
            } else if (tok == '"') {
                ip++;
                while (*ip && *ip != '"') ip++;
                if (*ip) ip++;
            } else {
                ip++;
            }
        }
        line = line->next;
    }
}

void gw_run_program(GW_State *state) {
    if (!state || !state->program_head) return;
    
    g_gosub_ptr = 0;
    g_for_ptr = 0;
    
    gw_scan_subs(state);
    
    state->current_line = state->program_head;
    state->ip = state->current_line->tokens;
    state->execution_active = true;
    
    while (state->execution_active && state->current_line) {
        state->current_line_num = state->current_line->line_num;
        
        gw_exec_statement(state);
        
        if (state->error_jump_pending) {
            state->error_jump_pending = false;
            GW_Line *l = find_line(state, state->error_handler_line);
            if (l) {
                state->current_line = l;
                state->ip = l->tokens;
            } else {
                gw_error(8); // Undefined line number
            }
            continue;
        }
        
        if (state->chain_jump_pending) {
            state->chain_jump_pending = false;
            continue;
        }
        
        if (state->jump_pending) {
            state->jump_pending = false;
            continue;
        }
        
        // Check if statement pointer reached end of line
        if (state->execution_active && (!state->ip || !*state->ip)) {
            // Move to next line
            state->current_line = state->current_line->next;
            if (state->current_line) {
                state->ip = state->current_line->tokens;
            }
        }
        
        gw_sdl2_poll_events();
    }
    
    if (state->in_error_handler) {
        gw_error(19); // No RESUME
    }
    
    state->execution_active = false;
}
