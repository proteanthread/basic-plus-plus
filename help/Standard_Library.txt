# Standard Library & Built-In Function Model

BASIC++ separates its runtime environment from its virtual machine core. The Standard Library acts as the primary API between BASIC programs and the execution layer.

## Core Immutable API (VM Contract Layer)
The Core Immutable API defines the guaranteed behavior that cannot be changed across different dialects. It interacts directly with the memory pool to ensure speed and safety.

Core functions include:
- `LEN(s$)`
- `CHR$(n)`
- `STR$(n)`
These functions **cannot** be overridden by `DEFINE SPECIFICATION` or Dialect configuration unless the interpreter security is explicitly set to `SEC_OPEN`.
- **Math:** `ABS`, `SGN`, `INT`, `SQR`, `SIN`, `COS`, `TAN`, `ATN`, `LOG`, `EXP`
- **Strings:** `LEFT$`, `RIGHT$`, `MID$`, `INSTR`, `LEN`, `CHR$`, `STR$`
- **Misc:** `RND(n)` (Upgraded to deterministic PCG32 generator)

## Dialect-Overridable API Layer
The Dialect-Overridable API allows historical dialects to adjust execution behaviors to emulate classic platforms without tampering with the core engine.

Examples of behaviors controlled by the Dialect API:
- `PRINT` formatting and padding.
- `INPUT` prompt characters (e.g. `? ` vs `:`).
- String concatenation logic.
- Native numerical conversion.

You can explicitly force a dialect profile using the standard library profile loader:
```basic
LOAD PROFILE "GWBASIC"
LOAD PROFILE "QBASIC"
```

## Granular File I/O Hooks
File I/O is famously fractured across historical dialects. The Standard Library explicitly supports 14 highly granular hooks in the `DialectOverrides` structure, allowing full emulation of any dialect's file mechanisms:
- **Sequential / Streams:** `file_print`, `file_input`, `file_line_input`
- **Byte / Char Operations:** `file_get_byte`, `file_put_byte`, `file_read_bytes`
- **Random Access:** `file_get_record`, `file_put_record`, `file_field_alloc`
- **Positioning & Hardware:** `file_seek`, `file_loc`, `file_lof`, `file_ioctl` (e.g. for Atari XIO)

## Extensible Error Architecture
BASIC++ implements an extensible error mapping system through `error_registry.c`.
- **`ERR$`:** Retrieves the syntax name for a 32-bit error code (e.g. `ERR$(11)` -> `"Division by zero"`).
- Dialects and external modules can inject custom errors at runtime via `error_registry_register()`.
- Standard PATB trapping using the raw `ERR` variable and `ERL` (Error Line Number) is natively preserved.

## Virtual Device (vdev) API
To ensure platform portability and tight security boundaries, NO standard library function directly interacts with the host Operating System. All operations are strictly decoupled:
- Console I/O routes through the Console Virtual Device.
- Disk operations route through the Filesystem Virtual Device.
