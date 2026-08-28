# `boot` Core Boot Subsystem (`libboot`)

## 1. Architectural Purpose & Overview

The `libboot` subsystem (`engine/src/boot/boot.c`) is the foundational Phase 1 boot controller of BASIC++ v6.5.2. It manages early runtime startup, command-line argument parsing, memory pool pre-allocation, platform initialization, and VM instance creation.

### Key Architectural Responsibilities:
- **Phase Sequence**: (1) Platform HAL setup, (2) Memory allocator initialization, (3) Virtual device bus binding, (4) Security capability initialization, (5) REPL or script dispatcher.
- **Opt-In Logging**: Initializes `logger_init(NULL, NULL)` by default. No `.LOG` files are created on disk unless `--log` or `--debug` is explicitly passed.
- **Footprint Gate**: Enforces target pool bounds (640MB `baspp`, 384MB `bpp`, 64MB `bs`).

---

## 2. Technical API Signatures (C17)

```c
int boot_init(int argc, char **argv, VMContext **out_vm);
void boot_shutdown(VMContext *vm);
int boot_dispatch(VMContext *vm);
```
