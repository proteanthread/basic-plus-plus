# BASIC++ Virtual Machine Architecture & Execution Tutorial

## 1. Architectural Overview & VM Pipeline

The BASIC++ v6.5.2 Virtual Machine is built on a strict, layered, non-recursive execution engine:

$$\text{Source Code} \longrightarrow \text{Lexer (Ephemeral Tokens)} \longrightarrow \text{Parser (AST Nodes)} \longrightarrow \text{Bytecode Emitter} \longrightarrow \text{VM Loop}$$

### Fundamental VM Invariants:
1. **Strict Non-Recursive Execution**: All execution states, procedure frames, control loops (`FOR`, `WHILE`, `DO`), and expression trees evaluate on heap-allocated structures without host C stack recursion.
2. **Ephemeral Tokenization**: Source code remains canonical text; tokens are scanned on-demand and discarded immediately after execution.
3. **Dialect-Agnostic VM Loop**: The core VM evaluates unified bytecode; dialect syntax variations are resolved strictly at the parser layer.
4. **Reference-Counted String Heap**: All string values (`VAL_STRING`) participate in an incremental reference-counting string manager (`str_create`, `str_release`).

---

## 2. Memory Footprint Allocation

Standard default memory allocations are strictly partitioned by target profile:
- **`baspp` (Standard Desktop)**: 640 MB (`671088640L` bytes)
- **`bpp` (Lite REPL)**: 384 MB (`402653184L` bytes)
- **`bs` (Batch Script Runner)**: 64 MB (`67108864L` bytes)

---

## 3. The 11-Layer Modular Library Spectrum

```
libboot -> libplatform -> libkernel -> libengine -> libhardware ->
libserver -> libscript -> libcore -> libflex -> libstandard -> libadvanced
```
- Features are placed at the lowest possible layer in the accumulative chain.
- OS-specific code is strictly encapsulated within `libplatform`. Upper layers have zero OS dependencies.
