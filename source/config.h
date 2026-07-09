/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: config.h
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Global compile-time configuration defines for pool memory sizes and limits.
 *
 * 2. WHAT TO EXPECT:
 *    Constant parameters compiled directly into the binary file. Cannot negotiate limits at runtime.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Compile-time limits (e.g. increase MAX_STACK_DEPTH, MAX_STRING_POOL, or dial in BPP_EMBEDDED constants).
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Preprocessor flag rules, feature mappings.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    If compilation size is too big, reduce memory pool sizes. Verify all dependencies rebuild after editing config.h.
 * ===================================================================== */

 // ---
 // BASIC++ Interpreter - config.h
 // ---
 //
 // Central compile-time configuration for the BASIC++ interpreter.
 //
 // PURPOSE:
 // All tunable constants, pool sizes, limits, and feature flags are
 // defined here so that the interpreter can be reconfigured without
 // editing individual modules. This is the single point of control
 // for memory layout, capacity, and build-time options.
 //
 // PORTABILITY:
 // The interpreter source is compiled as C17 (ISO/IEC 9899:2018).
 // The built-in BASIC++->C transpiler still emits C89 for maximum
 // portability of generated code.
 //
 // HOW TO EXTEND:
 // Add new #define constants here. Do not scatter magic numbers
 // across modules. Group related constants together and document
 // their purpose and valid ranges.
 //
 // ---

#ifndef BASICPP_CONFIG_H
#define BASICPP_CONFIG_H

// --- C17 Standard Headers ---
 // C17 guarantees <stdint.h>, <stdbool.h>, and <stddef.h>.
 // These provide intptr_t, uintptr_t, int32_t, uint64_t,
 // bool, true, false, size_t, etc.
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// --- VERSION AND IDENTITY ---
 // Version uses MAJOR.MINOR.PATCH semantic versioning:
 // MAJOR - dialect milestone (1.0 = first complete dialect)
 // MINOR - feature additions (new statements, modules, dialects)
 // PATCH - bug fixes only (no new features)
 //
 // These strings are printed at startup and embedded in SAVE file
 // headers (if applicable in future versions).
#define BASICPP_VERSION_MAJOR 5
#define BASICPP_VERSION_MINOR 0
#define BASICPP_VERSION_PATCH 1
#define BASICPP_VERSION "5.0.1"

#ifdef BPP_LITE_BUILD
  #define BASICPP_NAME "BASIC++ Lite"
  #define BASICPP_PROMPT "] "
#else
  #ifdef INPUT_CONSOLE
    #define BASICPP_NAME "BASIC++ Standard"
  #else
    #define BASICPP_NAME "BASIC++ SDL"
  #endif
  #define BASICPP_PROMPT "> "
#endif

#define BASICPP_COPYRIGHT "@COPYLEFT ALL WRONGS RESERVED"
#define BASICPP_READY "Ready."

// ===================================================================
 // DEFAULT DIALECT
 // ===================================================================
 //
 // The default dialect is used when no -d flag is given on the
 // command line and no dialect= setting exists in basicpp.cfg.
 //
 // HOW TO CHANGE THE DEFAULT:
 // Replace DIALECT_GW_BASIC below with any DialectId value from
 // dialect.h.  Common choices:
 //
 //   DIALECT_GW_BASIC     GW-BASIC (default -- best DOS compatibility)
 //   DIALECT_ECMA116      ECMA-116 Full BASIC (richest language)
 //   DIALECT_QBASIC       QBasic (structured, no line numbers needed)
 //   DIALECT_TINY_BASIC   Palo Alto Tiny BASIC (minimalist)
 //   DIALECT_ECMA55       ECMA-55 Minimal BASIC (ISO standard)
 //
 // On FreeDOS, only DIALECT_GW_BASIC and DIALECT_ECMA116 are
 // compiled in.  The user can switch at runtime:
 //   DIALECT "E116"       (switch to ECMA-116)
 //   DIALECT "GWBS"       (switch to GW-BASIC)
 //
 // On Windows/Linux, all 16 dialects are available.  Switch with:
 //   DIALECT "QBAS"       (or any 4-char dialect code)
 //   basicpp -d QBAS      (command-line switch)
 //   dialect=QBAS         (in basicpp.cfg)
#define BASICPP_DEFAULT_DIALECT DIALECT_GW_BASIC

// ===================================================================
 // MEMORY POOL SIZES -- PLATFORM-SPECIFIC
 // ===================================================================
 //
 // BASIC++ uses two build profiles selected at compile time:
 //
 //   BPP_FREEDOS   Defined automatically by the Makefile for
 //                 OpenWatcom builds (make watcom / make watcom386).
 //                 Fits within FreeDOS's 512K conventional memory.
 //                 Includes only GW-BASIC + ECMA-116 dialects.
 //
 //   (default)     Used by gcc, clang, and MSVC builds.
 //                 Generous pools for modern systems (4GB - 256GB).
 //                 Includes all 16 dialects and all modules.
 //
 // HOW TO CUSTOMIZE FOR YOUR SYSTEM:
 // If you are building for a system with specific memory constraints,
 // edit the values in the appropriate section below.  Key guidelines:
 //
 //   - PROGRAM_MEMORY_SIZE controls stored program text.
 //     8 MB holds approximately 250,000 lines of BASIC.
 //
 //   - MAX_STRING_POOL controls all string values during a RUN.
 //     16 MB is generous even for text-heavy programs.
 //     Reduce to 1 MB for systems with < 8 GB RAM.
 //
 //   - MAX_ARRAY_ELEMENTS controls total array element count
 //     across all DIM arrays.  4M elements x ~16 bytes each = 64 MB.
 //     Reduce to 262144 (256K x 16B = 4 MB) for 4 GB systems.
 //
 //   - VARIABLE_MEMORY_SIZE and SCRATCH_MEMORY_SIZE are internal
 //     working pools.  1 MB each is sufficient for any program.
 //
 // Memory estimates for default modern settings:
 //   Program memory:      8 MB
 //   Variable memory:     1 MB
 //   Scratch memory:    512 KB
 //   String pool:        16 MB
 //   Array pool:        ~64 MB (4M elements x 16 bytes)
 //   --------------------------
 //   Approximate total:  ~90 MB  (well under 4 GB minimum)
 //
 // Memory estimates for FreeDOS settings:
 //   Program memory:     32 KB
 //   Variable memory:    16 KB

#if defined(BPP_FREEDOS) && defined(BPP_LITE_BUILD)
// ===================================================================
// FREEDOS LITE BUILD -- 16-bit Watcom Lite (Conventional Memory constraints)
// ===================================================================
// Fits within FreeDOS's 640 KB conventional memory.
// Defaults to GW-BASIC.
#undef BASICPP_DEFAULT_DIALECT
#define BASICPP_DEFAULT_DIALECT DIALECT_GW_BASIC

#define PROGRAM_MEMORY_SIZE   32768L // 32 KB
#define VARIABLE_MEMORY_SIZE  16384L // 16 KB
#define SCRATCH_MEMORY_SIZE   16384L // 16 KB
#define GRAPHICS_MEMORY_SIZE  16384L // 16 KB

#define MAX_PROGRAM_LINES     1024
#define MAX_STACK_DEPTH       64

#define MAX_NAMED_VARS        128
#define MAX_DATA_ITEMS        1024

#define MAX_STRING_POOL       16384L // 16 KB

#define MAX_DIM_ARRAYS        32
#define MAX_ARRAY_ELEMENTS    2048

#define MAX_USER_FUNCS        32
#define MAX_MODULES           8
#define MAX_BREAKPOINTS       16

#define MAX_EXT_FUNCS         4
#define MAX_EXT_LIBS          2
#define MAX_EXT_FEATURES      4
#define MAX_EXT_PLUGINS       2

#define MAX_USER_TYPES        8
#define MAX_TYPE_FIELDS       16
#define MAX_TYPED_VARS        32

#define GFX_WIDTH             80
#define GFX_HEIGHT            25

#elif defined(BPP_EMBEDDED)
// ===================================================================
 // EMBEDDED BUILD -- ESP32 / Arduino Due / Raspberry Pi Pico
 // ===================================================================
 // Targets microcontrollers with 96 KB - 520 KB SRAM.
 // ESP32-WROOM has ~200 KB available after FreeRTOS + WiFi stack.
 // Arduino Due has 96 KB SRAM.
 // Raspberry Pi Pico has 264 KB SRAM.
 //
 // Default dialect: Palo Alto Tiny BASIC (smallest footprint).
 // Only essential dialects compiled in. No optional modules
 // (USB, FujiNet, UPnP, JIT). No dynamic library loading.
 // No codegen/transpiler.
 //
 // For ESP32 with PSRAM (ESP32-WROVER), you can increase pool
 // sizes significantly. Define BPP_PSRAM to use larger pools:
 //   #define BPP_PSRAM
 //   #define PROGRAM_MEMORY_SIZE  65536L   (64 KB)
 //   #define MAX_STRING_POOL      65536L   (64 KB)
 //
 // Memory estimate (default BPP_EMBEDDED):
 //   Program memory:      8 KB
 //   Variable memory:     4 KB
 //   Scratch memory:      2 KB
 //   String pool:         8 KB
 //   Array pool:       ~16 KB (1024 elements x 16 bytes)
 //   --------------------------
 //   Pools subtotal:    ~38 KB
 //   RuntimeState:      ~30 KB (with reduced mem_segment)
 //   Stack + overhead:  ~10 KB
 //   --------------------------
 //   Approximate total: ~78 KB  (fits in ESP32 ~200 KB)
#undef BASICPP_DEFAULT_DIALECT
#define BASICPP_DEFAULT_DIALECT DIALECT_GW_BASIC

#define PROGRAM_MEMORY_SIZE   8192L // 8 KB
#define VARIABLE_MEMORY_SIZE  4096L // 4 KB
#define SCRATCH_MEMORY_SIZE   2048L // 2 KB
#define GRAPHICS_MEMORY_SIZE  8192L  // 8 KB

#define MAX_PROGRAM_LINES     256
#define MAX_STACK_DEPTH       32

#define MAX_NAMED_VARS        64
#define MAX_DATA_ITEMS        256

#define MAX_STRING_POOL       8192L // 8 KB

#define MAX_DIM_ARRAYS        16
#define MAX_ARRAY_ELEMENTS    1024

#define MAX_USER_FUNCS        16
#define MAX_MODULES           4
#define MAX_BREAKPOINTS       8

#define MAX_EXT_FUNCS         2 // loaded ext functions
#define MAX_EXT_LIBS          1 // loaded BASIC++ libs
#define MAX_EXT_FEATURES      2 // loaded spec features
#define MAX_EXT_PLUGINS       1 // loaded plugin pkgs

#define MAX_USER_TYPES        4
#define MAX_TYPE_FIELDS       8
#define MAX_TYPED_VARS        16

#define GFX_WIDTH             64
#define GFX_HEIGHT            32

#else
// ===================================================================
 // MODERN BUILD -- Windows 11 / Linux (4 GB - 256 GB RAM)
 // ===================================================================
 // All 16 dialects and all modules are compiled in.
 // Pool sizes are generous but still modest relative to modern RAM.
 // Even on a slim 4 GB system, the ~90 MB total is well within reach.
 //
 // CUSTOMIZATION EXAMPLES:
 //
 //   Slim 4 GB system (reduce array and string pools):
 //     #define MAX_STRING_POOL    1048576L     (1 MB)
 //     #define MAX_ARRAY_ELEMENTS 262144       (256K elements)
 //
 //   Beefy 64 GB workstation (maximize everything):
 //     #define PROGRAM_MEMORY_SIZE 16777216L   (16 MB)
 //     #define MAX_STRING_POOL     67108864L   (64 MB)
 //     #define MAX_ARRAY_ELEMENTS  16777216    (16M elements)
 //     #define MAX_PROGRAM_LINES   262144      (256K lines)
 //
 //   Server / batch processing (minimize graphics):
 //     #define GFX_WIDTH  80
 //     #define GFX_HEIGHT 50

#define PROGRAM_MEMORY_SIZE   8388608L // 8 MB
#define VARIABLE_MEMORY_SIZE  1048576L // 1 MB
#define SCRATCH_MEMORY_SIZE   524288L // 512 KB
#define GRAPHICS_MEMORY_SIZE  4194304L // 4 MB

#define MAX_PROGRAM_LINES     65536
#define MAX_STACK_DEPTH       1024

#define MAX_NAMED_VARS        4096
#define MAX_DATA_ITEMS        65536

#define MAX_STRING_POOL       16777216L // 16 MB

#define MAX_DIM_ARRAYS        1024
#define MAX_ARRAY_ELEMENTS    4194304 // 4M elements

#define MAX_USER_FUNCS        256
#define MAX_MODULES           64
#define MAX_BREAKPOINTS       256

#define MAX_EXT_FUNCS         32 // loaded ext functions
#define MAX_EXT_LIBS          8 // loaded BASIC++ libs
#define MAX_EXT_FEATURES      16 // loaded spec features
#define MAX_EXT_PLUGINS       8 // loaded plugin pkgs

#define MAX_USER_TYPES        64
#define MAX_TYPE_FIELDS       64
#define MAX_TYPED_VARS        512

#define GFX_WIDTH             320
#define GFX_HEIGHT            200

#endif // BPP_EMBEDDED

// ===================================================================
 // SHARED CONSTANTS -- same on all platforms
 // ===================================================================
 // These values are fixed by the BASIC language definition or by
 // practical limits that don't vary with available memory.

// Standard Library Boundaries (Milestone 8)
#define STDLIB_CORE_ENABLED 1
#define STDLIB_DIALECT_ENABLED 1
#define STDLIB_MAX_OVERRIDES 64

// Line length: 255 chars matches classic BASIC convention
#define MAX_LINE_LENGTH 255

// Line number range: 1-65529 (matches GW-BASIC, fits in 32-bit signed or 16-bit unsigned)
#define LINE_NUMBER_MIN 1
#define LINE_NUMBER_MAX 65529

// Single-letter variables: A-Z = 26 (always available)
#define MAX_VARIABLES 26

// Default PRINT field width (PATB convention)
#define DEFAULT_PRINT_WIDTH 6

// File channels: #1 through #8
#define MAX_FILE_CHANNELS 8

// Variable name length (e.g., "SCORE", "COUNT")
#define MAX_VAR_NAME_LEN 31

// Input buffer: line length + newline + null
#define INPUT_BUFFER_SIZE (MAX_LINE_LENGTH + 2)

// String values: max length of a single string
#define MAX_STRING_LENGTH 255

// String variables: A$-Z$ = 26
#define MAX_STRING_VARS 26

// Array dimensions: up to 3 (matches most BASIC dialects)
#define MAX_ARRAY_DIMS 3

// Function arguments: 4 covers MID$(s$, start, len) etc.
#define MAX_FUNC_ARGS 4

// DEF FN parameters: 4 (GW-BASIC/QBasic compatibility)
#define MAX_FN_PARAMS 4

// DEF FN body: stored as string, max line length
#define MAX_FN_BODY MAX_LINE_LENGTH

// VM expression evaluation stack depth
#define VM_EVAL_STACK_SIZE 64

// Virtual memory segment size (DEF SEG / PEEK / POKE)
 // On embedded targets, this is reduced to save RAM.
 // Full 64K emulates classic BASIC address space.
 // Reduced to 4K on embedded (sufficient for most PEEK/POKE). 
#if defined(BPP_EMBEDDED)
#define MAX_MEM_SEGMENT 4096
#else
#define MAX_MEM_SEGMENT 1048576
#endif

// Graphics color palette size
#define GFX_MAX_COLORS 16

#ifdef BPP_LITE_BUILD
  // Limit settings for Lite profile
  #undef MAX_NAMED_VARS
  #define MAX_NAMED_VARS 64
  #undef MAX_ARRAY_ELEMENTS
  #define MAX_ARRAY_ELEMENTS 512
  #undef LINE_NUMBER_MAX
  #define LINE_NUMBER_MAX 16384

  // Feature gates for blite
  #undef BPP_SUPPORT_GRAPHICS
  #undef BPP_SUPPORT_SOUND
  #undef BPP_SUPPORT_FILEMGMT
  #undef BPP_SUPPORT_MAT
  #undef BPP_SUPPORT_STRUCT
  #undef BPP_SUPPORT_ERRHAND
  #undef BPP_SUPPORT_SHELL
  #undef BPP_SUPPORT_DEBUG
  #undef BPP_SUPPORT_COMPILER
  #undef BPP_SUPPORT_VFS
  #undef BPP_SUPPORT_TXN
#else
  // Standard compile parameters
  #define BPP_SUPPORT_GRAPHICS
  #define BPP_SUPPORT_SOUND
  #define BPP_SUPPORT_FILEMGMT
  #define BPP_SUPPORT_MAT
  #define BPP_SUPPORT_STRUCT
  #define BPP_SUPPORT_ERRHAND
  #define BPP_SUPPORT_SHELL
  #define BPP_SUPPORT_DEBUG
  #define BPP_SUPPORT_COMPILER
  #define BPP_SUPPORT_VFS
  #define BPP_SUPPORT_TXN
#endif

#endif // BASICPP_CONFIG_H
