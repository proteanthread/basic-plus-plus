/**
 * @file bpp_types.h
 * @brief Core types, value representations, and structured errors for BASIC++.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Defines the base type definitions for interpreter values, line numbers,
 *   opcodes, and structured errors.
 * - Why it exists: Provides a single, clean definition of the type system shared by the lexer,
 *   parser, expression evaluator, VM, and runtime.
 * - Why it works this way: By utilizing a disciplined union-based BValue (with double as the universal
 *   number and opaque references for strings), we achieve GW-BASIC compatibility while allowing
 *   QBASIC type suffixes and future extension domains.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: ValueType enums can be expanded to support new types (e.g. COMPLEX, objects).
 * - What cannot be changed: BValue's union layout without updating the evaluator and garbage collector.
 * - What to expect: Changes to base types require rebuilding the entire codebase.
 * - What to do if something breaks: If type alignment or size constraints fail (especially on 16-bit targets),
 *   verify struct packing and padding.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: Relies on IEEE 754 double precision floating point representation for universal numbers.
 * - Portability concerns: double is universally supported, but check Watcom 16-bit double alignment behaviors.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - How future expansion can occur safely: Add additional type codes to ValueType and corresponding union elements.
 * - How to write external extensions: External plugins interact with the VM using BValue structures.
 */

#ifndef BPP_TYPES_H
#define BPP_TYPES_H

#include <stdint.h>
#include <stdbool.h>

/* Forward declaration for the opaque string handle */
typedef struct BppString BppString;
typedef BppString* BppStringRef;
typedef struct BppMap BppMap;
typedef struct BppTypeRegistry BppTypeRegistry;

/* Universal type codes for values */
typedef enum {
    VAL_NONE = 0,       /* Uninitialized state */
    VAL_NUMBER,         /* Universal floating-point number (double) */
    VAL_INTEGER,        /* Optimized integer (int32_t represented in double or flag) */
    VAL_STRING,         /* Reference-counted string handle */
    VAL_MAP,            /* Reference-counted dictionary/map */
    VAL_ARRAY_REF,      /* Symbolic link to ArrayContext array */
    VAL_FIELD_STRING    /* GW-BASIC Random Access File bound string */
} ValueType;

/* The primary tagged-union value type */
typedef struct {
    ValueType type;
    union {
        double       number; /* Stores float/double/integer values */
        BppStringRef string; /* Reference to isolated string heap */
        BppMap      *map;    /* Reference-counted map */
        const char  *array_name; /* Name of the target array for VAL_ARRAY_REF */
        struct {
            int channel;
            int offset;
            int length;
        } field_str;         /* Bound to a VDev file buffer */
    } as;
} BValue;

/* Line number representation (double to support fractional lines like 10.5) */
typedef double BppLineNumber;

/* Opcodes for the virtual machine execution engine (Cached Opcode Mode) */
typedef enum {
    OP_HALT = 0,
    OP_NOP,
    OP_PUSH_CONST,
    OP_PUSH_VAR,
    OP_POP_VAR,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_CMP_EQ,
    OP_CMP_NE,
    OP_CMP_LT,
    OP_CMP_GT,
    OP_CMP_LE,
    OP_CMP_GE,
    OP_JMP,
    OP_JMP_IF_FALSE,
    OP_CALL,
    OP_RET,
    OP_PRINT,
    OP_INPUT,
    OP_COUNT  /* Count of opcodes */
} BppOpcode;

/* Category of errors in the structured error system */
typedef enum {
    ERR_CAT_NONE = 0,
    ERR_CAT_SYNTAX,     /* Compile/parse errors */
    ERR_CAT_RUNTIME,    /* VM execution errors */
    ERR_CAT_SYSTEM,     /* Device, file, OS errors */
    ERR_CAT_INTERNAL    /* Compiler, VM limits, panic faults */
} BppErrorCategory;

/* Structured error block containing full traceback metadata */
typedef struct {
    int              code;       /* GW-BASIC/QBASIC compatible error code (1-255) */
    BppErrorCategory category;   /* Categorization for filtering */
    const char      *message;    /* Detailed, readable error message string */
    BppLineNumber    line;       /* BASIC program line number where fault occurred */
    int              col;        /* Column position where scanner failed */
    const char      *file;       /* Host source file containing the emitter */
} BppError;

#endif /* BPP_TYPES_H */
