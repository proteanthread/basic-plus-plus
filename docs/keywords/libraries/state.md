# `state` Runtime Execution State Subsystem (`libkernel`)

## 1. Architectural Purpose & Overview

The `state` subsystem tracks execution flags, program counters, call frames, loop states (`FOR`, `WHILE`, `DO`), and error handling vectors in the VM.

### Key Architectural Invariants:
- **Heap Call Frames**: All call frames and loop contexts reside on heap-allocated stacks, avoiding host recursion.
- **Persistent State Read-Back**: Authoritative device states are read directly from virtual devices (`VConContext`, `VDevContext`).

---

## 2. Technical Definitions (C17)

```c
typedef struct VMCallFrame {
    int line_number;
    size_t return_offset;
    struct VariableScope *local_scope;
    struct VMCallFrame *prev;
} VMCallFrame;
```
