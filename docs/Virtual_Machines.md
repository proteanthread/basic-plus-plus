<!--
Title:        Virtual Machines
Tier:         2
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot, bppc, trans, detok)
Authority:    engine/src/vm/, engine/include/vm/vm.h
Generated:    no
Status:       Active
-->

# BASIC++ Virtual Machine Architecture

The technical specification of the BASIC++ Virtual Machine, its non-recursive execution looper, state container (`VMContext`), and dedicated control-flow stacks.

## 1. Non-Recursive Stack Architecture

The BASIC++ VM is a non-recursive, stack-managed execution engine. Rather than utilizing the host C runtime call stack for program recursion, all control states, loop boundaries, subroutine calls, and exception handlers are stored in dynamically managed heap frames.

### 1.1 The Ephemeral Token-Parse-Execute Cycle
For each program statement:
1. **On-Demand Lexing**: The lexer tokenizes source text line-by-line. Tokens are transient and discarded immediately after statement dispatch.
2. **Statement Identification**: The parser inspects the leading token to select the statement handler.
3. **Handler Execution**: The registered statement function executes and returns a `BppError` status.
4. **Program Counter Advancement**: The VM advances the line index or jumps to a target line.

---

## 2. The `VMContext` State Container

The `VMContext` struct (`engine/include/vm/vm.h`) encapsulates the complete virtual machine environment:
- `MemoryContext`: Program text storage and bytecode caches.
- `StringContext`: Reference-counted dynamic string heap (`str_create`, `str_release`).
- `VariableContext`: Scalar variables and structured UDT instances.
- `ArrayContext`: Multi-dimensional array descriptors and data buffers.
- `VDevContext`: Virtual device bus (`CON:`, `SCRN:`, `PRN:`, `CAS1:`).
- `VConContext`: Virtual console cell attribute grid and cursor state.
- `SecurityContext`: Active security restrictions and capability bitmasks.
- `StmtRegistry` / `FuncRegistry`: Function pointer dispatch tables.

---

## 3. Dedicated Control-Flow Stacks

The VM maintains seven isolated stacks to manage nested control structures without mutual interference:

| Stack | Manages | Stored Frame Information | Depth Limits |
| :--- | :--- | :--- | :--- |
| `GosubStack` | `GOSUB` / `RETURN` | Return line number and character offset | 1023 (Desktop), 63 (DOS), 31 (IoT) |
| `ForStack` | `FOR` / `NEXT` | Counter variable, limit, step, loop body line | 1023 (Desktop), 63 (DOS), 31 (IoT) |
| `WhileStack` | `WHILE` / `WEND` | Expression AST offset, loop test line | 1023 (Desktop), 63 (DOS), 31 (IoT) |
| `DoStack` | `DO` / `LOOP` | Condition type (`WHILE`/`UNTIL`), loop head line | 1023 (Desktop), 63 (DOS), 31 (IoT) |
| `SelectStack` | `SELECT CASE` | Comparison value, match sentinel flag | 1023 (Desktop), 63 (DOS), 31 (IoT) |
| `SubStack` | `SUB` / `FUNCTION` | Return point, local scope context, formal arguments | 1023 (Desktop), 63 (DOS), 31 (IoT) |
| `TryStack` | `TRY` / `CATCH` | Catch line, stack depth snapshots for unwinding | 1023 (Desktop), 63 (DOS), 31 (IoT) |

Stack underflow or overflow triggers deterministic error codes (Error 1: `NEXT without FOR`, Error 3: `RETURN without GOSUB`).
