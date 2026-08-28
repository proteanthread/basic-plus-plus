# C17 API Reference: Engine Configuration & Limits (`types/config.h`)

## 1. Subsystem Overview & Responsibilities

The Engine Configuration Subsystem (`types/config.h`) defines compile-time build parameters, bitness detection macros, memory pool partition sizes, buffer thresholds, array dimension limits, and target profile definitions for the BASIC++ v6.5.2 engine.

Key architectural responsibilities include:
- **Build Target Profiles**:
  - `BASIC_STD_BUILD` (`baspp` Standard Desktop Edition): Console + SDL Combined, prompt `> `, default 640 MB RAM.
  - `BASIC_LITE_BUILD` (`bpp` Lite Edition): Headless REPL for terminals and IoT, prompt `] `, default 384 MB RAM.
  - `BASIC_SCRIPT_BUILD` (`bs` Batch Script Runner): Non-interactive, zero prompt, default 64 MB RAM.
- **Architectural Bitness Auto-Detection**: Detects 64-bit, 32-bit, or 16-bit word sizes via preprocessor macros (`__x86_64__`, `_WIN64`, `__aarch64__`).
- **Resource Constraints & Limits**: Sets standard limits for open file channels, evaluation stack depth, named variables, string lengths, and array allocations.

## 2. Header Inclusion & Prerequisites

```c
#include "types/config.h"
```

## 3. Configuration Constants & Memory Profiles

### Memory Pool Allocations
```c
/* Standard Desktop Profile (baspp) - 640 MB Total */
#define BASIC_DEFAULT_PROG_MEM      134217728L   /* 128 MB program storage */
#define BASIC_DEFAULT_VAR_MEM       134217728L   /* 128 MB variable space */
#define BASIC_DEFAULT_STR_MEM       335544320L   /* 320 MB string heap */
#define BASIC_DEFAULT_SCRATCH_MEM   67108864L    /* 64 MB scratch area */

/* Lite Edition Profile (bpp) - 384 MB Total */
#define BASIC_DEFAULT_PROG_MEM      67108864L    /* 64 MB program storage */
#define BASIC_DEFAULT_VAR_MEM       67108864L    /* 64 MB variable space */
#define BASIC_DEFAULT_STR_MEM       201326592L   /* 192 MB string heap */
#define BASIC_DEFAULT_SCRATCH_MEM   67108864L    /* 64 MB scratch area */

/* Batch Runner Profile (bs) - 64 MB Total */
#define BASIC_SCRIPT_TOTAL_MEM      67108864L    /* 64 MB unified pool */
```

### Resource Capacities & Limits
```c
#define BASIC_MAX_OPEN_FILES        256          /* Max concurrent file channels */
#define BASIC_MAX_STACK_DEPTH       1023         /* Max evaluation / call stack frames */
#define BASIC_MAX_NAMED_VARS        8192         /* Max active variable symbols */
#define BASIC_MAX_DIM_ARRAYS        1024         /* Max active dimensioned arrays */
#define BASIC_MAX_STRING_LEN        65535        /* Max string length (64 KB) */
#define BASIC_MAX_LINE_LEN          1024         /* Max BASIC line length */
```

### Identity and Prompt Strings
```c
#ifdef BASIC_LITE_BUILD
  #define BASIC_NAME                "BASIC++ Lite"
  #define BASIC_PROMPT              "] "
  #define BASIC_STATUS              "Ready.\n"
#else
  #define BASIC_NAME                "BASIC++ Standard"
  #define BASIC_PROMPT              "> "
  #define BASIC_STATUS              "Ok\n"
#endif

#define BASIC_VERSION_STRING        "6.5.2"
#define BASIC_COPYRIGHT             "@COPYLEFT ALL WRONGS RESERVED"
```

## 4. Architectural Invariants

- **Prompt Invariants**:
  - `baspp` must strictly use `> ` prompt with `Ok` status.
  - `bpp` must strictly use `] ` prompt with `Ready.` status.
  - `bs` must have zero banner, zero prompt, and zero REPL iterations.
- **Memory Footprint Invariants**:
  - `baspp` standard allocation must be 640 MB (`671088640L` bytes).
  - `bpp` lite allocation must be 384 MB (`402653184L` bytes).
  - `bs` script allocation must be 64 MB (`67108864L` bytes).

## 5. Usage Example: Checking Target Profile in C

```c
#include "types/config.h"
#include <stdio.h>

void print_build_profile(void) {
    printf("%s v%s (%s)\n", BASIC_NAME, BASIC_VERSION_STRING, BASIC_BITNESS);
    printf("Prompt: %s\n", BASIC_PROMPT);
}
```
