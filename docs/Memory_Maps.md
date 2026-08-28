# BASIC++ v6.5.2 Memory Maps

## 1. MEMORY ARCHITECTURE

BASIC++ uses a flat memory model where all allocations come from a single address space managed by the MemoryContext (engine/include/memory/memory.h). The total memory pool is divided into four regions: program memory, variable memory, string heap, and scratch area. The sizes of these regions depend on the build profile.

## 2. MODERN 64-BIT PROFILE (baspp)

The standard desktop edition allocates 640 MB total:

```text
+------------------------------------------+  0x00000000
|  Program Memory        (128 MB)          |
|  - Line-numbered source storage          |
|  - ProgramStore sorted index             |
+------------------------------------------+  0x08000000
|  Variable Memory       (128 MB)          |
|  - Named variable table                  |
|  - Array element storage                 |
|  - TYPE/CLASS instance data              |
+------------------------------------------+  0x10000000
|  String Heap           (256 MB)          |
|  - Reference-counted BppStringRef blocks |
|  - Garbage collected when near capacity  |
+------------------------------------------+  0x20000000
|  Scratch Area          (128 MB)          |
|  - Temporary evaluation results          |
|  - Expression evaluator stacks           |
|  - Statement handler working buffers     |
+------------------------------------------+  0x28000000
```

Stack depths: GosubStack 1023, ForStack 1023, WhileStack 1023, DoStack 1023, SelectStack 1023, SubStack 1023, TryStack 255. Named variable limit: 8192. DIM arrays: 4096. Array elements: 4,194,304. User-defined functions: 256.

## 3. LITE 64-BIT PROFILE (bpp)

The lite edition allocates 384 MB total:

| Region | Size |
|--------|------|
| Program Memory | 64 MB |
| Variable Memory | 64 MB |
| String Heap | 192 MB |
| Scratch Area | 64 MB |

Stack depths: same as modern (1023). Named variable limit: 8192. DIM arrays: 4096. Array elements: 4,194,304.

## 4. BATCH RUNNER PROFILE (bs)

The batch script runner allocates 64 MB total with smaller region sizes appropriate for scripted, non-interactive workloads.

## 5. FREEDOS 16-BIT PROFILE

The FreeDOS profile fits within the 640 KB conventional memory limit of real-mode DOS:

| Region | Size |
|--------|------|
| Program Memory | 32 KB |
| Variable Memory | 16 KB |
| String Heap | 16 KB |
| Scratch Area | 8 KB |

Stack depths: 63. Named variable limit: 128. DIM arrays: 32. Array elements: 2,048. User-defined functions: 16.

## 6. EMBEDDED PROFILE

The embedded profile targets microcontrollers with under 64 KB of RAM:

| Region | Size |
|--------|------|
| Program Memory | 8 KB |
| Variable Memory | 4 KB |
| String Heap | 4 KB |
| Scratch Area | 2 KB |

Stack depths: 31. Named variable limit: 64. DIM arrays: 16. Array elements: 512. User-defined functions: 8.

## 7. MEMORY INTROSPECTION

The following functions query the current memory state:

FRE(0) — Free bytes in the string heap.
FRE(-1) — Largest contiguous free block in the string heap.
FRE(-2) — Free bytes in the variable memory region.

MEMMAP$ returns the name of the active memory profile as a string: "MODERN", "LITE", "FREEDOS", or "EMBEDDED".

The INFO command displays the complete memory configuration including all region sizes and current usage.

## 8. MEMORY CONFIGURATION DEFINES

The memory profile is controlled by preprocessor defines in engine/include/types/config.h:

| Define | Purpose |
|--------|---------|
| BASIC_DEFAULT_PROG_MEM | Program memory size in bytes |
| BASIC_DEFAULT_VAR_MEM | Variable memory size in bytes |
| BASIC_DEFAULT_STR_MEM | String heap size in bytes |
| BASIC_DEFAULT_SCRATCH_MEM | Scratch area size in bytes |
| BASIC_DEFAULT_MAX_VARS | Maximum named variables |
| BASIC_DEFAULT_STACK_DEPTH | Maximum stack depth |
| BASIC_DEFAULT_MAX_DIM | Maximum DIM arrays |
| BASIC_DEFAULT_MAX_ELEMENTS | Maximum array elements |
| BASIC_DEFAULT_MAX_DEFFN | Maximum user-defined functions |

These values are set differently depending on which profile macros are defined: BASIC_FREEDOS_16, BASIC_EMBEDDED, BASIC_LITE_BUILD, or the default modern profile.

## 9. DYNAMIC SCALING

On modern hosts, BASIC++ can dynamically scale memory usage beyond the default profile. The total announced memory (640 MB for standard) is the default allocation. If the host has sufficient memory, the interpreter can expand regions on demand. The CLEAR statement with a memory argument explicitly sets the string heap size: `CLEAR 1000000` allocates 1 MB for strings.
