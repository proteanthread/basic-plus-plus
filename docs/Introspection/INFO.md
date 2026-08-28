# INFO Statement Reference

The `INFO` statement displays real-time runtime diagnostics, memory allocation statistics, virtual device metrics, security levels, and engine version telemetry directly to the virtual console.

## Syntax

```basic
INFO
```

## Parameters

*(No parameters required.)*

## Description

`INFO` provides a comprehensive runtime dashboard for developers and system administrators. It interrogates the `VMContext`, `MemoryContext`, `StringContext`, `VariableContext`, and `VDevContext` structures to output an instantaneous snapshot of system resource consumption.

### Displayed Telemetry:
1. **Engine Version & Identity**: Engine version (v6.5.2 "Phoenix"), build edition (`Standard`, `Lite`, `Server`), and target bitness (32-bit / 64-bit).
2. **Active Dialect**: Current active dialect mode (`GW-BASIC`, `QBASIC`, `ECMA-116`, `Super BASIC`).
3. **Memory Metrics**:
   - **Program Memory**: Total bytes allocated vs. bytes consumed by stored program lines.
   - **Variable Space**: Number of active variables, arrays dimensioned, and memory consumed.
   - **String Pool**: Total string heap capacity, active string count, and free string bytes.
   - **Stack Depth**: Current call stack frames vs. `BASIC_MAX_STACK_DEPTH`.
4. **Security & Sandbox Level**: Current security level (0 = OPEN to 5 = RESTRICTED) and active capability restrictions.
5. **Virtual Devices**: Count of registered virtual devices and active open file channels.

```
======================================================================
BASIC++ v6.5.2 Standard Edition (64-Bit) — System Information
======================================================================
Dialect:              GW-BASIC (GWBS)
Memory Pool:          640.00 MB Total (671,088,640 bytes)
  - Program Storage:  128.00 MB (Used: 4,120 bytes, 18 lines)
  - Variable Space:   128.00 MB (Used: 256 bytes, 6 variables)
  - String Heap:      256.00 MB (Used: 1,024 bytes, 12 strings)
  - Scratch Arena:    128.00 MB
Call Stack:           0 / 1023 frames active
Security Level:       0 (OPEN - All capabilities enabled)
Active Devices:       8 registered (CON:, COM1:, VFS0:, N1:, LPT1:)
Open Channels:        0 / 16 active
======================================================================
Ok
```

---

## Code Examples

### Example 1: Immediate Diagnostic Inspection
```basic
INFO
REM Prints the complete runtime telemetry snapshot to the console
```

---

## Engine Implementation (`system.c` & `vm/context.c`)

In `engine/src/statements/system/system.c`, `stmt_info_handler` gathers metrics via accessor APIs:
- `mem_get_total_free()`, `mem_get_total_used()` (`engine/include/memory/memory.h`)
- `str_get_total_allocated()`, `str_get_string_count()` (`engine/include/runtime/strings.h`)
- `var_get_count()` (`engine/include/runtime/variables.h`)
- `vdev_get_registered_count()` (`engine/include/device/vdev.h`)

Output is dispatched via `vdev_printf(vm_get_vdev(vm), ...)`.

---

## Error Codes

| Error Code | Name | Condition |
|------------|------|-----------|
| 2 | Syntax Error (`ERR_SYNTAX_ERROR`) | Trailing unexpected arguments passed to `INFO` |

---

## Cross-References

- **`VER`** — Displays concise engine and program version numbers.
- **`MEMMAP$`** — Queries active BIOS memory map model name.
- **`HELP`** — Queries interactive documentation.
- **`CATALOG`** — Lists language keyword categories.

---

## Proposed Expansion or Changes

1. **`INFO$` Function**: Introduce `telemetry$ = INFO$("MEMORY")` or `INFO$("STACK")` to query specific telemetry values programmatically in BASIC scripts.
2. **Interactive Benchmark Metric**: Include CPU cycle count or MIPS estimate in the diagnostic dashboard.
