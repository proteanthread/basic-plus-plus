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
 *    - Global structures, data types (GW_State, GW_Value, GW_Var), and config constants.
 *    - Global interpreter state struct, type system, and constant tables.
 * 
 * 4. WHAT TO DO IF SOMETHING BREAKS:
 *    - Check variable tables, default variable type states, and stack pointers.
 *    - Cross-reference token layouts with original detokenization specifications.
 *    - Run diagnostic verification script to identify isolated error line numbers.
 * -----------------------------------------------------------------------------
 */
#ifndef GWBASIC_H
#define GWBASIC_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// --- Configuration Constants (from OEM.H) ---
#define BUFLEN 255     // Line input buffer length
#define LINLN  80      // Standard screen line width
#define LPTLEN 132     // Max printer width
#define CLMWID 14      // Width of print zones
#define NMKEYF 10      // Number of function keys
#define MAX_DEVICES 15 // Max open files/devices simultaneously

// --- Basic Data Types ---
typedef enum {
    TYPE_INTEGER,
    TYPE_SINGLE,
    TYPE_DOUBLE,
    TYPE_STRING,
    TYPE_COMPLEX,
    TYPE_NONE
} GW_Type;

typedef enum {
    MACHINE_VGA = 0,      // Default (CGA/EGA/VGA)
    MACHINE_HERCULES = 1, // HGC (SCREEN 3 -> 720x348)
    MACHINE_TANDY = 2,    // Tandy 1000 (SCREEN 3 -> 160x200)
    MACHINE_PCJR = 3,     // IBM PCjr (SCREEN 3 -> 160x200)
    MACHINE_PLANTRONICS = 4, // Plantronics Colorplus
    MACHINE_ATT = 5,      // AT&T 6300 (640x400)
    MACHINE_AMSTRAD = 6,  // Amstrad PC1512/1640 (640x200 16-color)
    MACHINE_PC98 = 7,     // NEC PC-9801 (640x400 16-color, N88-BASIC)
    MACHINE_MDA = 8,      // Monochrome Display Adapter (MDA - Text-only Default)
    MACHINE_CGA = 9,      // Color Graphics Adapter (CGA - SDL GUI Default)
    MACHINE_EGA = 10      // Enhanced Graphics Adapter
} GW_MachineType;

// String Descriptor
typedef struct {
    uint8_t len;
    char *ptr;
} GW_String;

// Tagged Value type for expressions
typedef struct {
    GW_Type type;
    union {
        int16_t i_val;
        float s_val;
        double d_val;
        GW_String str;
        struct {
            double real;
            double imag;
        } c_val;
    };
} GW_Value;

// BASIC Variable structure
typedef struct GW_Var {
    char name[40];
    GW_Type type;
    union {
        int16_t i_val;
        float s_val;
        double d_val;
        GW_String str;
        struct {
            double real;
            double imag;
        } c_val;
    };
    struct GW_Var *next;
} GW_Var;

// Array dimension information
typedef struct {
    int16_t size;
    int16_t lbound;
    int16_t ubound;
} GW_Dim;

// BASIC Array structure
typedef struct GW_Array {
    char name[40];
    GW_Type type;
    int16_t num_dims;
    GW_Dim dims[10];
    void *data; // Points to allocated array of type values
    struct GW_Array *next;
} GW_Array;

// Tokenized line of code
typedef struct GW_Line {
    int32_t line_num;
    uint8_t *tokens;
    size_t length;
    struct GW_Line *next;
} GW_Line;

#include "standalone/events/events_core.h"
typedef BppEventTrapCore GW_EventTrap;

// --- File/Device Structure ---
typedef enum {
    DEV_NONE,
    DEV_FILE,
    DEV_SERIAL,
    DEV_PRINTER,
    DEV_CONSOLE
} GW_DeviceType;

typedef struct {
    GW_DeviceType type;
    int mode; // OPEN mode (input, output, random, append)
    int record_len;
    void *handle; // Points to FILE*, GW_SerialPort*, etc.
    uint8_t *record_buffer; // For RANDOM access record buffers
    size_t record_buffer_size;
} GW_FileChannel;

typedef struct GW_SubDef {
    char name[40];
    struct GW_Line *start_line;
    uint8_t *start_ip;
    struct GW_SubDef *next;
} GW_SubDef;

typedef struct GW_CallFrame {
    struct GW_Line *return_line;
    uint8_t *return_ip;
    struct GW_Var *local_variables;
    struct GW_Array *local_arrays;
    struct GW_CallFrame *next;
} GW_CallFrame;

// --- Global Interpreter State Structure (Replaces DSEG) ---
typedef struct {
    // Program lines
    GW_Line *program_head;
    GW_Line *current_line;
    int32_t current_line_num; // CURLIN
    uint8_t *ip; // Instruction pointer within current line's tokens
    bool execution_active;
    bool ctrl_c_pressed;
    int32_t exec_count;

    // Variables & Arrays
    GW_Var *variables; // Current scope variables
    GW_Array *arrays;  // Current scope arrays
    GW_Var *global_variables; // Global scope
    GW_Array *global_arrays;  // Global scope
    GW_CallFrame *call_stack;
    GW_SubDef *subs; // Registered SUBs and FUNCTIONs
    GW_Type default_types[26]; // A-Z default type (from DEFINT/DEFSNG/DEFDBL/DEFSTR)

    // String space allocator
    char *string_space;
    size_t string_space_size;
    size_t string_space_free;

    // Input buffer
    char input_buffer[BUFLEN + 1];

    // Screen and editor attributes
    int screen_mode;
    int fg_color;
    int bg_color;
    int cursor_x;
    int cursor_y;
    bool key_display_on;

    // Error handling state (ON ERROR GOTO)
    int32_t error_handler_line;
    bool in_error_handler;
    bool error_jump_pending;
    int last_error_code;
    int32_t last_error_line;
    GW_Line *error_line_ptr;
    uint8_t *error_ip;
    uint8_t *stmt_ip;
    bool chain_jump_pending;
    bool jump_pending;
    char current_program_path[256];



    // Event traps
    GW_EventTrap key_traps[NMKEYF];
    GW_EventTrap com_traps[2];
    GW_EventTrap timer_traps[4];

    // Pluggable components context
    struct GW_Memory *mem_sys;
    struct GW_PluginManager *plugin_mgr;

    // File channels
    GW_FileChannel channels[MAX_DEVICES + 1];

    // Data pointer (for READ/DATA/RESTORE)
    GW_Line *data_line;
    uint8_t *data_ip;
    
    GW_MachineType machine;
} GW_State;

// Global instance pointer
extern GW_State *g_state;

// --- Built-in Functions & Methods ---
void gw_error(int error_code);
void gw_eval_expr(GW_State *state, GW_Value *result);
void gw_exec_statement(GW_State *state);

#endif // GWBASIC_H
