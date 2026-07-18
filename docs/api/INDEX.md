# BASIC++ API Documentation Index

> **For**: Third-party developers embedding BASIC++ in their own projects
> **Format**: Per-subsystem Markdown docs + inline header comments + tutorials

---

## Getting Started

To embed BASIC++ in your C project:

```c
#include "bpp_boot.h"
#include "bpp_vm.h"
#include "bpp_config.h"

int main(void) {
    BppConfig cfg = bpp_config_defaults();
    BppContext *ctx = bpp_init(&cfg);
    bpp_exec_string(ctx, "PRINT \"Hello from BASIC++!\"");
    bpp_shutdown(ctx);
    return 0;
}
```

Link against `libbasicpp.a` (Linux) or `libbasicpp.lib` (Windows).

---

## API Documentation by Tier

### Tier 1 — Embedding & Lifecycle
*Start here. These are the entry points for integrating BASIC++ into your application.*

| API Doc | Header | Description |
|---------|--------|-------------|
| [boot.md](boot.md) | `bpp_boot.h` | Initialization, shutdown, entry points |
| [vm.md](vm.md) | `bpp_vm.h` | VM lifecycle, execution, line dispatch |
| [config.md](config.md) | `bpp_config.h` | Configuration, options, dialect selection |
| [version.md](version.md) | `bpp_version.h` | Version constants and queries |

### Tier 2 — Language Core
*Understand how BASIC++ parses and evaluates programs.*

| API Doc | Header | Description |
|---------|--------|-------------|
| [lexer.md](lexer.md) | `bpp_lexer.h` | Tokenizer, keyword table, token types |
| [eval.md](eval.md) | `bpp_eval.h` | Expression evaluation, operator dispatch |
| [stmt.md](stmt.md) | `bpp_stmt.h` | Statement dispatch, registration |
| [types.md](types.md) | `bpp_types.h` | Type system, value representation |
| [variables.md](variables.md) | `bpp_variables.h` | Variable storage, scoping |
| [arrays.md](arrays.md) | `bpp_arrays.h` | Array management, DIM, REDIM |
| [strings.md](strings.md) | `bpp_strings.h` | String pool, string operations |

### Tier 3 — Runtime Extensions
*Extend the interpreter with files, functions, data structures, and tasks.*

| API Doc | Header | Description |
|---------|--------|-------------|
| [file.md](file.md) | `bpp_file.h` | File I/O, channels, modes |
| [funcreg.md](funcreg.md) | `bpp_funcreg.h` | Function registry, DEF FN |
| [map.md](map.md) | `bpp_map.h` | Map/dictionary data structure |
| [struct.md](struct.md) | `bpp_struct.h` | User-defined types (TYPE...END TYPE) |
| [task.md](task.md) | `bpp_task.h` | Cooperative multitasking |
| [spec.md](spec.md) | `bpp_spec.h` | Language specification queries |
| [metadata.md](metadata.md) | `bpp_metadata.h` | Directives, pragmas, annotations |

### Tier 4 — Virtual Devices & I/O
*The virtual device layer abstracts all hardware interaction.*

| API Doc | Header | Description |
|---------|--------|-------------|
| [vdev.md](vdev.md) | `bpp_vdev.h` | Virtual device framework |
| [vcon.md](vcon.md) | `bpp_vcon.h` | Virtual console |
| [vfs.md](vfs.md) | `bpp_vfs.h` | Virtual filesystem |
| [vnet.md](vnet.md) | `bpp_vnet.h` | Virtual networking |
| [bus.md](bus.md) | `bpp_bus.h` | Virtual bus |
| [usb.md](usb.md) | `bpp_usb.h` | USB device abstraction |
| [fujinet.md](fujinet.md) | `bpp_fujinet.h` | FujiNet retro networking |
| [gemini.md](gemini.md) | `bpp_gemini.h` | Gemini protocol |

### Tier 5 — Graphics
*The BASIC++ Graphics Interface (BGI) and hardware emulation.*

| API Doc | Header | Description |
|---------|--------|-------------|
| [bgi.md](bgi.md) | `bpp_bgi.h` | BGI Graphics Interface |
| [mock_bios.md](mock_bios.md) | `bpp_mock_bios.h` | Mock BIOS for heritage hardware |

### Tier 6 — Security & Modules
*Sandbox, capability system, and external module loading.*

| API Doc | Header | Description |
|---------|--------|-------------|
| [security.md](security.md) | `bpp_security.h` | Security sandbox, capability system |
| [module.md](module.md) | `bpp_module.h` | Module loading, validation pipeline |
| [mod_arrayext.md](mod_arrayext.md) | `mod_arrayext.h` | Array extension module |

### Tier 7 — Platform & Memory
*OS abstraction and memory management internals.*

| API Doc | Header | Description |
|---------|--------|-------------|
| [platform.md](platform.md) | `bpp_platform.h` | OS abstraction layer |
| [memory.md](memory.md) | `bpp_memory.h` | Memory management |
| [segmented_mem.md](segmented_mem.md) | `bpp_segmented_mem.h` | Segmented memory (RAMBANK) |

### Tier 8 — Dialect & Configuration
*Dialect configuration, logging, and editor integration.*

| API Doc | Header | Description |
|---------|--------|-------------|
| [dialect.md](dialect.md) | `bpp_dialect.h` | Dialect system |
| [logger.md](logger.md) | `bpp_logger.h` | Logging infrastructure |
| [editor.md](editor.md) | `bpp_editor.h` | Editor integration |
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
| `BPP_SDL2` | ON (baspp) | Enable SDL2 backend for graphics, sound, and input |
| `BPP_HEADLESS` | ON (blite, bscript) | No graphics or sound |
| `BPP_FRAMEBUFFER_ONLY` | OFF | Software framebuffer only, no SDL2 window |

---

## Document Status

> ⚠️ **Note**: Individual API docs are being created as part of Phase 11. This index will be updated as each doc is completed.
