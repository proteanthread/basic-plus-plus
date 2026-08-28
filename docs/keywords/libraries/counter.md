# `counter` Atomic Metrics & Performance Counters (`libserver`)

## 1. Architectural Purpose & Overview

The `counter` subsystem provides thread-safe atomic counters, execution metric tracking, and instruction cycle counting in the VM.

### Key Architectural Invariants:
- **Lock-Free Atomics**: Implements C11/C17 standard atomic operations (`<stdatomic.h>`).
- **VM Metrics**: Tracks total instructions executed, allocations made, and strings compacted.

---

## 2. Technical API Signatures (C17)

```c
uint64_t counter_get(CounterType type);
void counter_inc(CounterType type);
void counter_reset(CounterType type);
```
