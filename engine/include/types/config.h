// FILENAME: config.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: baspp.exe, bpp.exe, bs.exe, libboot, libcore, libengine
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (version.c)
// NEEDS: libkernel (version.h)
// Provides core logic and interface definitions for config within BASIC++.
//
// ---- Includes ----

#ifndef TYPES_CONFIG_H
#define TYPES_CONFIG_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "types/version.h"

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

// Name and identity strings based on build targets
#ifdef BASIC_LITE_BUILD
  #define BASIC_NAME "BASIC++ Lite"
  #define BASIC_PROMPT "] "
#else
  #define BASIC_NAME "BASIC++ Standard"
  #define BASIC_PROMPT "> "
#endif

#define BASIC_COPYRIGHT "@COPYLEFT ALL WRONGS RESERVED"
#define BASIC_READY "Ready."



// Bitness detection
#if defined(_WIN64) || defined(__x86_64__) || defined(__ppc64__) || defined(__aarch64__) || (defined(__WORDSIZE) && __WORDSIZE == 64) || (defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 8)
  #define BASIC_BITNESS "64-Bit"
#elif defined(__i386__) || defined(__i486__) || defined(__i586__) || defined(__i686__) || defined(_M_IX86) || (defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4)
  #define BASIC_BITNESS "32-Bit"
#else
  #define BASIC_BITNESS "16-Bit"
#endif

// -------------------------------------------------------------
// MEMORY PROFILES
// -------------------------------------------------------------
#if defined(BASIC_LITE_BUILD)
  // Lite Build Profile - aimed for resource-constrained systems, IoT, servers
  #define BASIC_PROFILE_NAME          "Lite / " BASIC_BITNESS
  #define BASIC_DEFAULT_PROG_MEM      67108864L   // 64 MB program storage
  #define BASIC_DEFAULT_VAR_MEM       67108864L   // 64 MB variable space
  #define BASIC_DEFAULT_STR_MEM       201326592L  // 192 MB string heap
  #define BASIC_DEFAULT_SCRATCH_MEM   67108864L   // 64 MB scratch area
  #define BASIC_MAX_STACK_DEPTH       1023
  #define BASIC_MAX_NAMED_VARS        8192
  #define BASIC_MAX_DIM_ARRAYS        1024
  #define BASIC_MAX_ARRAY_ELEMENTS    4194304
  #define BASIC_MAX_USER_FUNCS        256
  #define BASIC_MAX_MODULES           64
  #define BASIC_MAX_BREAKPOINTS       64
#elif defined(BASIC_FREEDOS_16)
  // FreeDOS 16-bit Watcom Profile - fits conventional memory limits (< 640KB)
  #define BASIC_PROFILE_NAME          "FreeDOS 16-Bit"
  #define BASIC_DEFAULT_PROG_MEM      32768L    // 32 KB program storage
  #define BASIC_DEFAULT_VAR_MEM       16384L    // 16 KB variable space
  #define BASIC_DEFAULT_STR_MEM       16384L    // 16 KB string heap
  #define BASIC_DEFAULT_SCRATCH_MEM   8192L     // 8 KB scratch area
  #define BASIC_MAX_STACK_DEPTH       63        // Deep enough for simple programs
  #define BASIC_MAX_NAMED_VARS        128
  #define BASIC_MAX_DIM_ARRAYS        32
  #define BASIC_MAX_ARRAY_ELEMENTS    2048
  #define BASIC_MAX_USER_FUNCS        16
  #define BASIC_MAX_MODULES           4
  #define BASIC_MAX_BREAKPOINTS       8
#elif defined(BASIC_EMBEDDED)
  // Extremely resource constrained (microcontrollers, e.g. Arduino, ESP32)
  #define BASIC_PROFILE_NAME          "Embedded"
  #define BASIC_DEFAULT_PROG_MEM      8192L     // 8 KB program storage
  #define BASIC_DEFAULT_VAR_MEM       4096L     // 4 KB variable space
  #define BASIC_DEFAULT_STR_MEM       4096L     // 4 KB string heap
  #define BASIC_DEFAULT_SCRATCH_MEM   2048L     // 2 KB scratch area
  #define BASIC_MAX_STACK_DEPTH       31
  #define BASIC_MAX_NAMED_VARS        64
  #define BASIC_MAX_DIM_ARRAYS        16
  #define BASIC_MAX_ARRAY_ELEMENTS    512
  #define BASIC_MAX_USER_FUNCS        8
  #define BASIC_MAX_MODULES           2
  #define BASIC_MAX_BREAKPOINTS       4
#else
  // Modern systems (Windows 11, Linux, BSD, iOS, Android, 32-bit FreeDOS)
  #define BASIC_PROFILE_NAME          "Modern / " BASIC_BITNESS
  #define BASIC_DEFAULT_PROG_MEM      134217728L  // 128 MB default program storage
  #define BASIC_DEFAULT_VAR_MEM       134217728L  // 128 MB default variable space
  #define BASIC_DEFAULT_STR_MEM       268435456L  // 256 MB default string heap
  #define BASIC_DEFAULT_SCRATCH_MEM   134217728L  // 128 MB default scratch area
  #define BASIC_MAX_STACK_DEPTH       1023
  #define BASIC_MAX_NAMED_VARS        8192
  #define BASIC_MAX_DIM_ARRAYS        1024
  #define BASIC_MAX_ARRAY_ELEMENTS    4194304
  #define BASIC_MAX_USER_FUNCS        256
  #define BASIC_MAX_MODULES           64
  #define BASIC_MAX_BREAKPOINTS       64
#endif

// Maximum number of open files simultaneously
#define BASIC_MAX_OPEN_FILES        16

// Maximum RS-232 COM port buffer size
#define BASIC_DEFAULT_COM_BUF       512

// Maximum record length for RANDOM files
#define BASIC_DEFAULT_RECORD_LEN    128

// -------------------------------------------------------------
// FEATURE SUPPORT GATES
// -------------------------------------------------------------
#ifndef SUPPORT_FILES
  #define SUPPORT_FILES 1
#endif
#ifndef SUPPORT_TRY
  #define SUPPORT_TRY 1
#endif
#ifndef SUPPORT_STRUCT
  #define SUPPORT_STRUCT 1
#endif
#ifndef SUPPORT_MODULE
  #define SUPPORT_MODULE 1
#endif
#ifndef SUPPORT_TASK
  #define SUPPORT_TASK 1
#endif
#ifndef SUPPORT_HELP
  #define SUPPORT_HELP 1
#endif
#ifndef SUPPORT_GRAPHICS
  #ifdef BASIC_LITE_BUILD
    #define SUPPORT_GRAPHICS 0
  #else
    #define SUPPORT_GRAPHICS 1
  #endif
#endif

#ifdef BASIC_LITE_BUILD
  #undef SUPPORT_HELP
  #define SUPPORT_HELP 1
  #undef SUPPORT_MAT
  #define SUPPORT_MAT 1
  #undef SUPPORT_NET
  #define SUPPORT_NET 1
  #undef SUPPORT_GEMINI
  #define SUPPORT_GEMINI 1
  #undef SUPPORT_BIOS
  #define SUPPORT_BIOS 0
  #undef SUPPORT_JSON
  #define SUPPORT_JSON 0
  #undef SUPPORT_XML
  #define SUPPORT_XML 0
  #undef SUPPORT_INI
  #define SUPPORT_INI 0
  #define SUPPORT_YAML 1
  #undef SUPPORT_OOP
  #define SUPPORT_OOP 1
  #undef SUPPORT_EDITOR
  #define SUPPORT_EDITOR 0
  #undef SUPPORT_FILES
  #define SUPPORT_FILES 1
#else
  #ifndef SUPPORT_MAT
    #define SUPPORT_MAT 1
  #endif
  #ifndef SUPPORT_NET
    #define SUPPORT_NET 1
  #endif
  #ifndef SUPPORT_GEMINI
    #define SUPPORT_GEMINI 1
  #endif
  #ifndef SUPPORT_BIOS
    #define SUPPORT_BIOS 1
  #endif
  #ifndef SUPPORT_JSON
    #define SUPPORT_JSON 1
  #endif
  #ifndef SUPPORT_XML
    #define SUPPORT_XML 1
  #endif
  #ifndef SUPPORT_INI
    #define SUPPORT_INI 1
  #endif
  #ifndef SUPPORT_YAML
    #define SUPPORT_YAML 1
  #endif
  #ifndef SUPPORT_OOP
    #define SUPPORT_OOP 1
  #endif
  #ifndef SUPPORT_EDITOR
    #define SUPPORT_EDITOR 1
  #endif
#endif

#endif // TYPES_CONFIG_H

