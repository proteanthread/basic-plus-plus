# BASIC++ API Documentation Index

> **For**: Third-party developers embedding BASIC++ in their own projects
> **Format**: Per-subsystem Markdown docs + inline header comments + tutorials

---

## Getting Started

To embed BASIC++ in your C project:

```c
#include "boot.h"
#include "vm.h"
#include "config.h"

int main(void) {
    BppConfig cfg = config_defaults();
    BppContext *ctx = basic_init(&cfg);
    exec_string(ctx, "PRINT \"Hello from BASIC++!\"");
    basic_shutdown(ctx);
    return 0;
}
```

Link against `libadvanced` (Desktop Edition), `libcore` (Lite REPL), or `libscript` (Script Runner).

---

## API Documentation by Subsystem Layer

### Subsystem Layer 1 — Embedding & Lifecycle
*Start here. These are the entry points for integrating BASIC++ into your application.*

| API Doc | Header | Description |
|---------|--------|-------------|
| [boot.md](boot.md) | `boot.h` | Initialization, shutdown, entry points |
| [vm.md](vm.md) | `vm.h` | VM lifecycle, execution, line dispatch |
| [config.md](config.md) | `config.h` | Configuration, options, dialect selection |
| [version.md](version.md) | `basic_version.h` | Version constants and queries |

### Subsystem Layer 2 — Language Core
*Understand how BASIC++ parses and evaluates programs.*

| API Doc | Header | Description |
|---------|--------|-------------|
| [lexer.md](lexer.md) | `lexer.h` | Tokenizer, keyword table, token types |
| [eval.md](eval.md) | `eval.h` | Expression evaluation, operator dispatch |
| [stmt.md](stmt.md) | `stmt.h` | Statement dispatch, registration |
| [types.md](types.md) | `types.h` | Type system, value representation |
| [variables.md](variables.md) | `variables.h` | Variable storage, scoping |
| [arrays.md](arrays.md) | `arrays.h` | Array management, DIM, REDIM |
| [strings.md](strings.md) | `strings.h` | String pool, string operations |

### Subsystem Layer 3 — Runtime Extensions
*Extend the interpreter with files, functions, data structures, and tasks.*

| API Doc | Header | Description |
|---------|--------|-------------|
| [file.md](file.md) | `file.h` | File I/O, channels, modes |
| [funcreg.md](funcreg.md) | `funcreg.h` | Function registry, DEF FN |
| [map.md](map.md) | `runtime/map.h` | Map/dictionary data structure |
| [struct.md](struct.md) | `struct_ctx.h` | User-defined types (TYPE...END TYPE) |
| [task.md](task.md) | `task.h` | Cooperative multitasking |
| [spec.md](spec.md) | `spec.h` | Language specification queries |
| [metadata.md](metadata.md) | `metadata.h` | Directives, pragmas, annotations |

### Subsystem Layer 4 — Virtual Devices & I/O
*The virtual device layer abstracts all hardware interaction.*

| API Doc | Header | Description |
|---------|--------|-------------|
| [vdev.md](vdev.md) | `vdev.h` | Virtual device framework |
| [vcon.md](vcon.md) | `vcon.h` | Virtual console |
| [vfs.md](vfs.md) | `vfs.h` | Virtual filesystem |
| [vnet.md](vnet.md) | `vnet.h` | Virtual networking |
| [bus.md](bus.md) | `bus.h` | Virtual bus |
| [usb.md](usb.md) | `usb.h` | USB device abstraction |
| [fujinet.md](fujinet.md) | `fujinet.h` | FujiNet retro networking |
| [gemini.md](gemini.md) | `gemini.h` | Gemini protocol |

### Subsystem Layer 5 — Graphics
*The BASIC++ Graphics Interface (BGI) and hardware emulation.*

| API Doc | Header | Description |
|---------|--------|-------------|
| [bgi.md](bgi.md) | `bgi.h` | BGI Graphics Interface |
| [bios.md](bios.md) | `bios.h` | Authentic IBM PC/XT/AT/PCjr BIOS Micro-Library |

### Subsystem Layer 6 — Security & Modules
*Sandbox, capability system, and external module loading.*

| API Doc | Header | Description |
|---------|--------|-------------|
| [security.md](security.md) | `security.h` | Security sandbox, capability system |
| [module.md](module.md) | `module.h` | Module loading, validation pipeline |
| [mod_arrayext.md](mod_arrayext.md) | `mod_arrayext.h` | Array extension module |

### Subsystem Layer 7 — Platform & Memory
*OS abstraction and memory management internals.*

| API Doc | Header | Description |
|---------|--------|-------------|
| [platform.md](platform.md) | `platform.h` | OS abstraction layer |
| [memory.md](memory.md) | `memory.h` | Memory management |
| [segmented_mem.md](segmented_mem.md) | `segmented_mem.h` | Segmented memory (RAMBANK) |

### Subsystem Layer 8 — Dialect & Configuration
*configuration, logging, and editor integration.*

| API Doc | Header | Description |
|---------|--------|-------------|
| [dialect.md](dialect.md) | `dialect.h` | Dialect system |
| [logger.md](logger.md) | `logger.h` | Logging infrastructure |
| [editor.md](editor.md) | `editor.h` | Editor integration |
| [custom_dialect.md](custom_dialect.md) | `custom_dialect_static.h` | Static dialect definitions |

---

## Tutorials

| Tutorial | Description |
|----------|-------------|
| [How to Embed BASIC++](tutorials/embedding.md) | Step-by-step guide to adding BASIC++ to a C project |
| [How to Add a Virtual Device](tutorials/custom_vdev.md) | Creating a custom virtual device |
| [How to Write a Module](tutorials/custom_module.md) | Building an external module (.dll/.so) |
| [How to Define a Dialect](tutorials/custom_dialect.md) | Creating a custom language dialect |
| [How to Add a Statement](tutorials/custom_statement.md) | Adding new BASIC keywords |
| [How to Add a Built-in Function](tutorials/custom_function.md) | Registering custom functions |

---

## Build Flags

| Flag | Default | Description |
|------|---------|-------------|
| `BASIC_SDL2` | ON (baspp) | Enable SDL2 backend for graphics, sound, and input |
| `BASIC_HEADLESS` | ON (blite, bscript) | No graphics or sound |
| `BASIC_FRAMEBUFFER_ONLY` | OFF | Software framebuffer only, no SDL2 window |

---

## Document Status

> ⚠️ **Note**: Individual API docs are being created as part of Phase 11. This index will be updated as each doc is completed.
