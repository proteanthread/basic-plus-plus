# BASIC++ Public C17 API Audit & Compliance Matrix

## 1. Executive Summary & Audit Scope

This document provides a formal audit of all public C17 APIs, structs, function prototypes, and header boundaries in the BASIC++ v6.5.2 engine codebase. Each module is evaluated for compliance with the repository's foundational architecture rules:
1. **Freestanding C17 Cleanliness**: Compiles with `-std=c17` without reliance on non-standard compiler extensions or OS headers in core layers.
2. **Memory Safety & Ownership Discipline**: Tagged union `BValue` returns, reference-counted `StringContext` string handles with two-parameter `str_release(vm_get_str(vm), str)`, and no raw pointer caching across VM resets.
3. **Non-Recursive Execution**: Heap-managed evaluation stacks without host C stack recursion.
4. **Platform Decoupling**: All host operating system interactions encapsulated strictly within `libplatform`.
5. **No BPP Prefix Invariant**: Use descriptive module-specific prefixes (e.g. `boot_`, `vm_`, `str_`, `arr_`) rather than generic `bpp_` prefixes for new APIs.

## 2. API Compliance Matrix

| Subsystem / Header | Domain Layer | Key Types & Structs | Exported Function Count | Status | Notes |
|---|---|---|---|---|---|
| [`core/boot.h`](boot.md) | `libboot` | `BppBootConfig` | 4 | COMPLIANT | Multi-target bootloader |
| [`core/state.h`](state.md) | `libkernel` | `InterpreterState` | 6 | COMPLIANT | Global VM state & signals |
| [`core/version.h`](version.md) | `libkernel` | Version constants | 2 | COMPLIANT | Semantic version 6.5.2 |
| [`types/types.h`](types.md) | `libkernel` | `BValue`, `BppError`, `BppToken` | N/A | COMPLIANT | Tagged union data types |
| [`memory/memory.h`](memory.md) | `libkernel` | `MemoryContext`, `BppProgramLine` | 14 | COMPLIANT | Dynamic pool allocation |
| [`memory/map.h`](map.md) | `libkernel` | `HashMap`, `HashEntry` | 8 | COMPLIANT | Hash table indexing |
| [`lexer/lexer.h`](lexer.md) | `libkernel` | `LexerContext`, `BppToken` | 12 | COMPLIANT | Ephemeral token stream |
| [`eval/eval.h`](eval.md) | `libengine` | `EvalContext`, AST nodes | 16 | COMPLIANT | Non-recursive evaluator |
| [`stmt/stmt.h`](stmt.md) | `libengine` | `StmtHandler`, `StmtTable` | 10 | COMPLIANT | Statement dispatch bus |
| [`vm/vm.h`](vm.md) | `libengine` | `VMContext`, `BppSubFrame` | 28 | COMPLIANT | VM execution engine |
| [`runtime/variables.h`](variables.md) | `libengine` | `VariableContext`, `BppVariable` | 18 | COMPLIANT | Scoped variable tables |
| [`runtime/strings.h`](strings.md) | `libengine` | `StringContext`, `BppString` | 22 | COMPLIANT | Ref-counted string pool |
| [`runtime/arrays.h`](arrays.md) | `libengine` | `ArrayContext`, `BppArray` | 16 | COMPLIANT | Multi-dim array storage |
| [`runtime/struct.h`](struct.md) | `libengine` | `StructContext`, `BppStruct` | 10 | COMPLIANT | User-defined TYPE structs |
| [`runtime/file.h`](file.md) | `libscript` | `FileContext`, `BppFileChannel` | 24 | COMPLIANT | Filesystem & record I/O |
| [`runtime/vfs.h`](vfs.md) | `libserver` | `VfsContext`, `BppMountPoint` | 12 | COMPLIANT | Virtual filesystem mounts |
| [`runtime/funcreg.h`](funcreg.md) | `libcore` | `FunctionRegistry`, `FuncDef` | 8 | COMPLIANT | Native function registry |
| [`runtime/metadata.h`](metadata.md) | `libcore` | `MicroLibMetadata` | 6 | COMPLIANT | Metadata block catalog |
| [`runtime/task.h`](task.md) | `libserver` | `TaskContext`, `BppTask` | 10 | COMPLIANT | Multithreaded tasks |
| [`runtime/gemini.h`](gemini.md) | `libserver` | `GeminiContext`, `GeminiStream` | 8 | COMPLIANT | Gemini protocol client |
| [`device/vdev.h`](vdev.md) | `libkernel` | `VDevContext`, `VDev` | 18 | COMPLIANT | Virtual device bus |
| [`device/vcon.h`](vcon.md) | `libkernel` | `VConContext`, `ScreenCell` | 16 | COMPLIANT | Virtual console display |
| [`device/vnet.h`](vnet.md) | `libserver` | `VNetContext`, `SocketHandle` | 14 | COMPLIANT | Network socket device |
| [`device/bgi.h`](bgi.md) | `libhardware` | `BGIContext`, `BgiModeProfile` | 32 | COMPLIANT | Retro graphics rasterizer |
| [`device/fujinet.h`](fujinet.md) | `libhardware` | `FujiNetContext` | 10 | COMPLIANT | FujiNet peripheral bus |
| [`device/usb.h`](usb.md) | `libhardware` | `USBContext`, `UsbDevice` | 8 | COMPLIANT | Virtual USB bus |
| [`device/bus.h`](bus.md) | `libhardware` | `BusContext` | 6 | COMPLIANT | I/O port address space |
| [`bios/bios.h`](bios.md) | `libkernel` | `BiosContext`, `BiosRegisters` | 20 | COMPLIANT | PC BIOS virtualization |
| [`memory/segmented_mem.h`](segmented_mem.md) | `libhardware` | `SegmentedMemContext` | 12 | COMPLIANT | Real-mode 1MB memory |
| [`security/security.h`](security.md) | `libkernel` | `SecurityContext` | 10 | COMPLIANT | Capability sandbox |
| [`module/module.h`](module.md) | `libflex` | `ModuleContext`, `ModuleHandle` | 8 | COMPLIANT | Dynamic module loader |
| [`module/mod_arrayext.h`](mod_arrayext.md) | `libflex` | `ArrayExtContext` | 12 | COMPLIANT | Matrix math extension |
| [`platform/platform.h`](platform.md) | `libplatform` | Platform abstractions | 40 | COMPLIANT | OS encapsulation layer |
| [`debug/logger.h`](logger.md) | `libplatform` | `LoggerContext`, `LogLevel` | 8 | COMPLIANT | Opt-in logger |
| [`editor/editor.h`](editor.md) | `libstandard` | `EditorContext`, `TuiWindow` | 14 | COMPLIANT | Multi-window TUI editor |

## 3. String Reference-Count Invariant Compliance

Every C API function returning or receiving `BValue` with type `VAL_STRING` conforms strictly to the two-parameter release convention:
```c
/* Correct Reference Release */
BValue str_val = eval_expression(vm, lex, &err);
if (str_val.type == VAL_STRING) {
    /* Consume string data */
    const char *data = str_data(str_val.as.string);
    /* Safe release */
    str_release(vm_get_str(vm), str_val.as.string);
}
```

## 4. Architectural Verification Summary

All 37 subsystem micro-libraries compile as freestanding C17, maintain zero host stack recursion, and route operating system calls through `libplatform`.
