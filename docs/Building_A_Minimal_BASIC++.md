<!--
Title:        Building A Minimal BASIC++
Tier:         1
Applies to:   BASIC++ v6.5.2 (iot, bs, bpp, FreeDOS)
Authority:    CMakeLists.txt, engine/include/types/config.h
Generated:    no
Status:       Active
-->

# Building a Minimal BASIC++ Engine

An architectural guide for stripping optional subsystems to produce minimal footprint interpreter binaries for microcontrollers, embedded IoT devices, FreeDOS real mode, and restricted deployment sandboxes.

## 1. Minimal Engine Architecture

The full desktop edition (`baspp`) links the complete 11-modular library spectrum (`libboot` through `libadvanced`). A minimal build halts linkage at lower layers in the chain, disabling optional feature gates.

The absolute minimum functional interpreter requires four foundational libraries:
- `libboot`: Bootstrapping and phase initialization.
- `libplatform`: Minimal console and time hooks.
- `libkernel`: Lexer, memory manager, and execution loop context.
- `libengine`: AST evaluator, parser, runtime variables, and standard control flow.

With these four libraries, the runtime executes line-numbered and structured programs: `LET`, `PRINT`, `INPUT`, `IF/THEN/ELSE`, `FOR/NEXT`, `WHILE/WEND`, `DO/LOOP`, `GOTO`, `GOSUB/RETURN`, `DIM`, `DATA/READ/RESTORE`, `DEF FN`, and all arithmetic operators.

---

## 2. Compile-Time Feature Gates

Subsystems can be selectively disabled via CMake definitions:

```bash
cmake .. -DBASIC_EMBEDDED=ON          -DSUPPORT_FILES=OFF          -DSUPPORT_TRY=OFF          -DSUPPORT_STRUCT=OFF          -DSUPPORT_MODULE=OFF          -DSUPPORT_TASK=OFF          -DSUPPORT_HELP=OFF          -DSUPPORT_GRAPHICS=OFF          -DSUPPORT_MAT=OFF          -DSUPPORT_NET=OFF          -DSUPPORT_GEMINI=OFF          -DSUPPORT_BIOS=OFF          -DSUPPORT_OOP=OFF          -DSUPPORT_EDITOR=OFF
```

---

## 3. Preset Target Memory Profiles

### 3.1 Embedded Microcontroller Profile (`BASIC_EMBEDDED`)
Targeted at ESP32, ARM Cortex-M, and RISC-V microcontrollers with sub-32KB RAM:
- Program Memory: 8 KB
- Variable Table: 4 KB
- String Heap: 4 KB
- Scratch Buffer: 2 KB
- Stack Depth: 31 frames
- Named Variables: 64
- DIM Arrays: 16

### 3.2 FreeDOS 16-Bit Real-Mode Profile (`BASIC_FREEDOS_16`)
Targeted at vintage PC hardware under 640 KB conventional memory:
- Program Memory: 32 KB
- Variable Table: 16 KB
- String Heap: 16 KB
- Scratch Buffer: 8 KB
- Stack Depth: 63 frames
- Named Variables: 128
- DIM Arrays: 32

### 3.3 IoT Micro-REPL Profile (`iot.exe`)
The dedicated headless micro-REPL links up to `libkernel` with a fixed 2 MB (`2097152L` bytes) monotonic arena pool, optimized for embedded hardware control and GPIO/sensor loops.

---

## 4. Custom Deployment Scenarios

1. **Classroom Educational Profile**: Enables HELP and SELFTEST, but disables host shell execution (`SHELL`) and external filesystem writes.
2. **Headless Data Processing Node**: Strips interactive REPL prompts and graphics; retains JSON, file streams, and network sockets.
3. **Firmware Command Parser**: Binds the bare evaluator directly to a UART ring buffer without filesystem support.
