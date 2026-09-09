<!--
Title:        Compiler, Transpiler, and Tool Targets Architecture
Tier:         2
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot, bppc, trans, detok)
Authority:    engine/
Generated:    no
Status:       Active
-->

# Compiler, Transpiler, and Tool Targets Architecture

## 1. Architectural Overview

BASIC++ v6.5.2 provides a decoupled compilation and transpilation suite that separates interactive script execution from ahead-of-time (AOT) bytecode emission, source-to-source C17 transpilation, and legacy binary detokenization.

```
+---------------------------------------------------------------------------------------------------+
|                                  BASIC++ Compilation Suite Targets                                |
+-----------------------------------+-----------------------------------+---------------------------+
|               bppc                |               trans               |           detok           |
|     (Compiler Orchestrator)       |    (Source-to-Source Transpiler)  |    (GW-BASIC Detokenizer) |
+-----------------+-----------------+-----------------+-----------------+-------------+-------------+
                  |                                   |                               |
                  v                                   v                               v
+-----------------+-----------------+-----------------+-----------------+-------------+-------------+
|    bppc_bytecode / compile_to_bpp |      transpile_basic_to_c_opts    |     detokenize_gw_basic     |
|   (BPE Bytecode Binary Emitter)   |     (ISO C17 Code Generator)      |    (Binary Header & Tokens) |
+-----------------+-----------------+-----------------+-----------------+-------------+-------------+
                  |                                   |
                  +-----------------+-----------------+
                                    |
                                    v
            +-----------------------+-----------------------+
            |                    libcompiler_ir             |
            |       (Freestanding Memory, IR Opcodes,       |
            |          Validation & Constant Folding)       |
            +-----------------------------------------------+
```

---

## 2. Decoupled Tool Executables & Interfaces

### 2.1 `bppc` / `bppc.exe` (Compiler Orchestrator)
- **Primary Function**: Compiles BASIC++ source scripts into portable bytecode binaries (`.bpp`), standalone executables, or invokes host C17 compilers (`cl`/`gcc`) to emit native binaries.
- **CLI Options**:
  - `bppc <input.bas> <output.bpp>`: Compiles to compact bytecode.
  - `bppc --bytecode <input.bas> -o <output.bpp>`: Explicit bytecode emission.
  - `bppc --c17 <input.bas> -o <output.c>`: Emits clean ISO C17 source code.
  - `bppc --native <input.bas> -o <output.exe>`: Transpiles and invokes the host compiler to generate a native binary.
  - `bppc --standalone [--windows|--linux] <input.bas> <output.exe>`: Bundles the runtime engine and bytecode into a self-contained executable.
  - `--optimize, -O`: Enables AST constant folding and IR dead-code elimination.
  - `--debug, -g`: Emits diagnostic symbols and comments.

### 2.2 `trans` / `trans.exe` (Source-to-Source Transpiler)
- **Primary Function**: Translates BASIC++ source code into clean, readable, dependency-free ISO C17 source code.
- **CLI Options**:
  - `trans <input.bas> -o <output.c>`: Translates BASIC to C17.
  - `trans --c17 <input.bas...>`: Explicit ISO C17 output mode.
  - `trans --inline-runtime`: Inlines self-contained runtime helpers into the output file.
  - `trans --optimize`: Performs constant folding and loop strength reduction.
  - `trans --debug`: Generates source line number labels (`line_10:`) and code comments.

### 2.3 `detok` / `detok.exe` (GW-BASIC Detokenizer)
- **Primary Function**: Decodes binary tokenized `.BAS` files (GW-BASIC/BASICA formats with `0xFF` signatures) into readable ASCII BASIC++ source text.
- **CLI Options**: `detok <input.bas> [output.txt]` (or stdout redirection).

---

## 3. Intermediate Representation (`libcompiler_ir`)

The intermediate representation library (`libcompiler_ir`) operates in pure freestanding C17 memory:
- **Opcode Generation**: Converts AST statement and expression nodes into linear bytecode instructions.
- **Constant Folding**: Evaluates constant numeric and string expressions at compile time.
- **Jump Resolution**: Computes relative branch offsets for `IF...THEN`, `FOR...NEXT`, `WHILE...WEND`, and `DO...LOOP` constructs.
