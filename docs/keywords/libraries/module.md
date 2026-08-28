# `module` Dynamic Module & Extension Subsystem (`libflex`)

## 1. Architectural Purpose & Overview

The `module` subsystem (`engine/src/module/module.c`) implements an auditable and sandboxed plugin architecture for loading external compiled C modules and extensions.

### Key Architectural Invariants:
- **Five-Stage Pipeline**: Modules load through: Validation $\to$ Capability Verification $\to$ Sandbox Allocation $\to$ Registration $\to$ Activation.
- **Safety Invariant**: Modules must NEVER directly mutate VM instructions, execute host code bypasses, or corrupt internal stacks.
- **Dynamic Linking**: Dynamically loads shared libraries (`.dll`, `.so`, `.dylib`) via `plat_dl`.

---

## 2. Technical API Signatures (C17)

```c
int module_load(const char *module_path, SecurityContext *sec);
int module_unload(const char *module_name);
int module_count(void);
```
