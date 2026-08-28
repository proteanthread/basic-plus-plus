# `vm` Virtual Machine Core Engine (`libkernel` / `libengine`)

## 1. Architectural Purpose & Overview

The `libkernel` and `libengine` VM subsystems (`engine/src/vm/vm.c`) implement the central execution coordinator for BASIC++ v6.5.2.

### Key Architectural Invariants:
- **Strict Non-Recursive VM**: Stacks, execution states, line running, and expression evaluation reside strictly in interpreter-managed heap structures. Host C stack recursion is strictly prohibited.
- **Dialect Agnostic**: Executes unified bytecode; dialect syntax is resolved at the parser layer.
- **Context Boundaries**: Enforces clean state transitions for `RUN`, `CHAIN`, and `MERGE`.

---

## 2. Technical API Signatures (C17)

```c
VMContext *vm_create(size_t memory_pool_bytes);
void vm_destroy(VMContext *vm);
BppError vm_execute_line(VMContext *vm, const char *line);
void vm_halt(VMContext *vm);
```
