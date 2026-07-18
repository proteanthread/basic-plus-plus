/**
 * @file bpp_config.h
 * @brief Compile-time configurations, limits, and profile settings.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Sets compiler constants, memory boundaries, stack sizes, and feature flags.
 * - Why it exists: Provides a single location to tune interpreter capacities and choose compilation
 *   profiles for different system classes (Embedded, FreeDOS 16-bit, FreeDOS 32-bit, Modern).
 * - Why it works this way: Decoupling constants from implementation files prevents magic number scattering
 *   and ensures that the interpreter configuration can scale dynamically or remain fixed.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Limits for stacks, programs, strings, variables, and arrays to fit target constraints.
 * - What cannot be changed: Header guards and library standard includes.
 * - What to expect: Changes to this file require a complete project rebuild.
 * - What to do if something breaks: If compilation fails due to memory exhaustion (e.g. on 16-bit targets),
 *   reduce the sizes of the static and default dynamic pools defined below.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: Standard C17 types (uint32_t, bool) are available in <stdint.h>, <stdbool.h>, and <stddef.h>.
 * - Portability concerns: For 16-bit platforms, limit sizes to fit standard segmented architecture boundaries.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - How future expansion can occur safely: Add additional feature switches or profile sections here.
 * - How to write external extensions: External plugins should read compile-time constants through the VM context APIs.
 */

#ifndef BPP_CONFIG_H
#define BPP_CONFIG_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "bpp_version.h"

#ifdef _WIN32
  #ifndef strcasecmp
    #define strcasecmp _stricmp
  #endif
  #ifndef strncasecmp
    #define strncasecmp _strnicmp
  #endif
#else
  #include <strings.h>
#endif

/* Name and identity strings based on build targets */
#ifdef BPP_LITE_BUILD
  #define BPP_NAME "BASIC++ Lite"
  #define BPP_PROMPT "] "
#else
  #define BPP_NAME "BASIC++ Standard"
  #define BPP_PROMPT "> "
#endif

#define BPP_COPYRIGHT "@COPYLEFT ALL WRONGS RESERVED"
#define BPP_READY "Ready."

/* Default Dialect at boot if not specified via command line */
#define BPP_DEFAULT_DIALECT "BASIC++"

/* Bitness detection */
#if defined(_WIN64) || defined(__x86_64__) || defined(__ppc64__) || defined(__aarch64__) || (defined(__WORDSIZE) && __WORDSIZE == 64) || (defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 8)
  #define BPP_BITNESS "64-Bit"
#elif defined(__i386__) || defined(__i486__) || defined(__i586__) || defined(__i686__) || defined(_M_IX86) || (defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4)
  #define BPP_BITNESS "32-Bit"
#else
  #define BPP_BITNESS "16-Bit"
#endif

/* -------------------------------------------------------------
 * MEMORY PROFILES
 * ------------------------------------------------------------- */
#if defined(BPP_LITE_BUILD)
  /* Lite Build Profile - aimed for resource-constrained systems, IoT, servers */
  #define BPP_PROFILE_NAME          "Lite / " BPP_BITNESS
  #define BPP_DEFAULT_PROG_MEM      67108864L   /* 64 MB program storage */
  #define BPP_DEFAULT_VAR_MEM       67108864L   /* 64 MB variable space */
  #define BPP_DEFAULT_STR_MEM       201326592L  /* 192 MB string heap */
  #define BPP_DEFAULT_SCRATCH_MEM   67108864L   /* 64 MB scratch area */
  #define BPP_MAX_STACK_DEPTH       1023
  #define BPP_MAX_NAMED_VARS        8192
  #define BPP_MAX_DIM_ARRAYS        1024
  #define BPP_MAX_ARRAY_ELEMENTS    4194304
  #define BPP_MAX_USER_FUNCS        256
  #define BPP_MAX_MODULES           64
  #define BPP_MAX_BREAKPOINTS       64
#elif defined(BPP_FREEDOS_16)
  /* FreeDOS 16-bit Watcom Profile - fits conventional memory limits (< 640KB) */
  #define BPP_PROFILE_NAME          "FreeDOS 16-Bit"
  #define BPP_DEFAULT_PROG_MEM      32768L    /* 32 KB program storage */
  #define BPP_DEFAULT_VAR_MEM       16384L    /* 16 KB variable space */
  #define BPP_DEFAULT_STR_MEM       16384L    /* 16 KB string heap */
  #define BPP_DEFAULT_SCRATCH_MEM   8192L     /* 8 KB scratch area */
  #define BPP_MAX_STACK_DEPTH       63        /* Deep enough for simple programs */
  #define BPP_MAX_NAMED_VARS        128
  #define BPP_MAX_DIM_ARRAYS        32
  #define BPP_MAX_ARRAY_ELEMENTS    2048
  #define BPP_MAX_USER_FUNCS        16
  #define BPP_MAX_MODULES           4
  #define BPP_MAX_BREAKPOINTS       8
#elif defined(BPP_EMBEDDED)
  /* Extremely resource constrained (microcontrollers, e.g. Arduino, ESP32) */
  #define BPP_PROFILE_NAME          "Embedded"
  #define BPP_DEFAULT_PROG_MEM      8192L     /* 8 KB program storage */
  #define BPP_DEFAULT_VAR_MEM       4096L     /* 4 KB variable space */
  #define BPP_DEFAULT_STR_MEM       4096L     /* 4 KB string heap */
  #define BPP_DEFAULT_SCRATCH_MEM   2048L     /* 2 KB scratch area */
  #define BPP_MAX_STACK_DEPTH       31
  #define BPP_MAX_NAMED_VARS        64
  #define BPP_MAX_DIM_ARRAYS        16
  #define BPP_MAX_ARRAY_ELEMENTS    512
  #define BPP_MAX_USER_FUNCS        8
  #define BPP_MAX_MODULES           2
  #define BPP_MAX_BREAKPOINTS       4
#else
  /* Modern systems (Windows 11, Linux, BSD, iOS, Android, 32-bit FreeDOS) */
  #define BPP_PROFILE_NAME          "Modern / " BPP_BITNESS
  #define BPP_DEFAULT_PROG_MEM      134217728L  /* 128 MB default program storage */
  #define BPP_DEFAULT_VAR_MEM       134217728L  /* 128 MB default variable space */
  #define BPP_DEFAULT_STR_MEM       268435456L  /* 256 MB default string heap */
  #define BPP_DEFAULT_SCRATCH_MEM   134217728L  /* 128 MB default scratch area */
  #define BPP_MAX_STACK_DEPTH       1023
  #define BPP_MAX_NAMED_VARS        8192
  #define BPP_MAX_DIM_ARRAYS        1024
  #define BPP_MAX_ARRAY_ELEMENTS    4194304
  #define BPP_MAX_USER_FUNCS        256
  #define BPP_MAX_MODULES           64
  #define BPP_MAX_BREAKPOINTS       64
#endif

/* Maximum number of open files simultaneously */
#define BPP_MAX_OPEN_FILES        16

/* Maximum RS-232 COM port buffer size */
#define BPP_DEFAULT_COM_BUF       512

/* Maximum record length for RANDOM files */
#define BPP_DEFAULT_RECORD_LEN    128

/* -------------------------------------------------------------
 * FEATURE SUPPORT GATES
 * ------------------------------------------------------------- */
#ifndef BPP_SUPPORT_FILES
  #define BPP_SUPPORT_FILES 1
#endif
#ifndef BPP_SUPPORT_TRY
  #define BPP_SUPPORT_TRY 1
#endif
#ifndef BPP_SUPPORT_STRUCT
  #define BPP_SUPPORT_STRUCT 1
#endif
#ifndef BPP_SUPPORT_MODULE
  #define BPP_SUPPORT_MODULE 1
#endif
#ifndef BPP_SUPPORT_TASK
  #define BPP_SUPPORT_TASK 1
#endif
#ifndef BPP_SUPPORT_HELP
  #define BPP_SUPPORT_HELP 1
#endif
#ifndef BPP_SUPPORT_GRAPHICS
  #ifdef BPP_LITE_BUILD
    #define BPP_SUPPORT_GRAPHICS 0
  #else
    #define BPP_SUPPORT_GRAPHICS 1
  #endif
#endif

#ifdef BPP_LITE_BUILD
  #undef BPP_SUPPORT_MAT
  #define BPP_SUPPORT_MAT 0
  #undef BPP_SUPPORT_NET
  #define BPP_SUPPORT_NET 0
  #undef BPP_SUPPORT_GEMINI
  #define BPP_SUPPORT_GEMINI 0
  #undef BPP_SUPPORT_BIOS
  #define BPP_SUPPORT_BIOS 0
  #undef BPP_SUPPORT_JSON
  #define BPP_SUPPORT_JSON 0
  #undef BPP_SUPPORT_XML
  #define BPP_SUPPORT_XML 0
  #undef BPP_SUPPORT_INI
  #define BPP_SUPPORT_INI 0
  #define BPP_SUPPORT_YAML 1
  #undef BPP_SUPPORT_OOP
  #define BPP_SUPPORT_OOP 0
  #undef BPP_SUPPORT_EDITOR
  #define BPP_SUPPORT_EDITOR 0
  #undef BPP_SUPPORT_FILES
  #define BPP_SUPPORT_FILES 1 /* Keep active but modifiable */
#else
  #ifndef BPP_SUPPORT_MAT
    #define BPP_SUPPORT_MAT 1
  #endif
  #ifndef BPP_SUPPORT_NET
    #define BPP_SUPPORT_NET 1
  #endif
  #ifndef BPP_SUPPORT_GEMINI
    #define BPP_SUPPORT_GEMINI 1
  #endif
  #ifndef BPP_SUPPORT_BIOS
    #define BPP_SUPPORT_BIOS 1
  #endif
  #ifndef BPP_SUPPORT_JSON
    #define BPP_SUPPORT_JSON 1
  #endif
  #ifndef BPP_SUPPORT_XML
    #define BPP_SUPPORT_XML 1
  #endif
  #ifndef BPP_SUPPORT_INI
    #define BPP_SUPPORT_INI 1
  #endif
  #ifndef BPP_SUPPORT_YAML
    #define BPP_SUPPORT_YAML 1
  #endif
  #ifndef BPP_SUPPORT_OOP
    #define BPP_SUPPORT_OOP 1
  #endif
  #ifndef BPP_SUPPORT_EDITOR
    #define BPP_SUPPORT_EDITOR 1
  #endif
#endif

#endif /* BPP_CONFIG_H */

