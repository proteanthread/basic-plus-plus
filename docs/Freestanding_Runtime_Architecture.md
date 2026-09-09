<!--
Title:        Freestanding_Runtime_Architecture
Tier:         2
Applies to:   BASIC++ v6.5.2 (bare-metal, UEFI, embedded, iot)
Authority:    engine/include/runtime/, engine/include/hal/
Generated:    no, manual specification
Status:       current
-->

# Freestanding C17 Runtime & Hardware Abstraction Layer (HAL)

## 1. Overview & ISO C17 Architecture

BASIC++ features a freestanding C17 runtime subsystem (`libcore_runtime`) and Freestanding Hardware Abstraction Layer (`libhal`). This architecture allows the virtual machine to execute in bare-metal, embedded, IoT, UEFI, and microcontroller environments without relying on host operating system standard libraries (`<stdlib.h>`, `<string.h>`, `<stdio.h>`, `<math.h>`, `<time.h>`, `<ctype.h>`).

Freestanding execution requires only standard ISO C17 freestanding headers (§4 ¶6):
- `<stdint.h>`
- `<stdbool.h>`
- `<stddef.h>`
- `<stdarg.h>`
- `<float.h>`
- `<limits.h>`

All core runtime functions and platform interactions route through decoupled micro-libraries and pluggable HAL virtual tables.

## 2. Micro-Library Functional Categories

The freestanding runtime is partitioned into decoupled micro-libraries located under `engine/include/runtime/` and `engine/src/runtime/`:

| Subsystem | Micro-Library | Header Path | Key Responsibilities |
| :--- | :--- | :--- | :--- |
| **`ctype`** | `libcore_runtime_ctype` | `runtime/ctype/ctype.h` | 256-byte static classification lookup table and ASCII case conversion. Constant-time O(1) operations with zero branch mispredictions: `runtime_isdigit`, `runtime_isalpha`, `runtime_toupper`, etc. |
| **`string`** | `libcore_runtime_string` | `runtime/string/memops.h`<br>`runtime/string/strops.h` | Word-aligned optimized memory copies (`runtime_memcpy`, `runtime_memset`, `runtime_memmove`) and bounded string operations (`runtime_strlen`, `runtime_strcmp`, `runtime_strlcpy`, `runtime_strstr`). |
| **`memory`** | `libcore_runtime_memory` | `runtime/memory/alloc.h`<br>`runtime/memory/arena.h` | Boundary-tagged coalescing heap pool allocator (`RuntimeMemPool`) and scoped linear arena allocator (`RuntimeArena`) for temporary AST and token allocation. |
| **`conv`** | `libcore_runtime_conv` | `runtime/conv/num_parse.h`<br>`runtime/conv/float_parse.h` | Base-adaptive integer parsing (`runtime_strtoll`, `runtime_lltoa_format`) and IEEE 754 floating-point conversion (`runtime_strtod`, `runtime_dtoa_format`) with 15-16 digit precision matching. |
| **`math`** | `libcore_runtime_math` | `runtime/math/basic.h`<br>`runtime/math/trig.h` | Standalone mathematical computation library without `<math.h>`. Polynomial trigonometry (`runtime_sin`, `runtime_cos`, `runtime_atan2`) and Newton-Raphson algebraic operations (`runtime_sqrt`, `runtime_pow`). |
| **`format`** | `libcore_runtime_format` | `runtime/format/snprintf.h`<br>`runtime/format/sscanf.h` | Freestanding string formatting and scanning (`runtime_snprintf`, `runtime_vsnprintf`, `runtime_sscanf`) supporting numeric, pointer, and string specifiers without host stdio. |
| **`sort`** | `libcore_runtime_sort` | `runtime/sort/qsort.h` | Non-recursive iterative introsort (`runtime_qsort`) using bounded execution stack (zero heap allocation, zero stack overflow risk) and binary search (`runtime_bsearch`). |
| **`time`** | `libcore_runtime_time` | `runtime/time/calendar.h` | Pure epoch arithmetic and leap-year calendar routines (`runtime_time_epoch_to_calendar`, `runtime_time_calendar_to_epoch`) calculating calendar components without host OS dependencies. |

## 3. Hardware Abstraction Layer (HAL) Architecture

The HAL (`libhal`) defines clean, pluggable interfaces between the virtual machine and the underlying execution environment:
- `HalMem` (`mem_hal.h`): Virtual table for `alloc`, `free`, `realloc`, and memory statistics.
- `HalIo` (`io_hal.h`): Virtual table for console character I/O (`putchar`, `getchar`) and stream operations.
- `HalTime` (`time_hal.h`): Virtual table for monotonic timestamps, epoch time, and millisecond delays (`sleep_ms`).
- `HalAudio` (`audio_hal.h`): Virtual table for tone generation, frequencies, and speaker control.
- `HalVideo` (`video_hal.h`): Virtual table for frame presentation, palette updates, and video events.
- `HalInput` (`input_hal.h`): Virtual table for keyboard polling and mouse tracking.

### Implementations

1. **Hosted HAL (`hal_hosted.c`)**: Direct mapping from HAL vtables to platform abstractions (`libplatform`), Win32, POSIX, and FreeDOS environments. Initialized automatically during boot.
2. **Freestanding / Bare-Metal HAL (`hal_freestanding.c`)**: Standalone implementation routing memory to `RuntimeMemPool`, formatting to `runtime_snprintf`, and providing hooks for UART consoles, hardware timers, and memory-mapped framebuffers.

## 4. Verification & Testing

The freestanding runtime is continuously verified against unit test suites (`tests/runtime_freestanding_test.c`) checking:
1. O(1) `ctype` classification and conversion accuracy.
2. Word-aligned `memops` and bounded `strops` boundary safety.
3. Multi-block allocation, coalescing, alignment, and arena rollback.
4. Base-adaptive integer parsing and 15-16 digit double-precision float fidelity.
5. Trigonometric, algebraic, and basic math accuracy against analytical constants.
6. `runtime_snprintf` formatting compliance across numeric formats.
7. Iterative non-recursive introsort order stability.
8. Calendar epoch-to-date and date-to-epoch transformations.
9. Pluggable HAL registration and execution under freestanding harnesses.
