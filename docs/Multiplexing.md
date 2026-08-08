# Unified Multiplexing & Headless Architecture in BASIC++

## Overview

BASIC++ supports a unified, ultra-granular C17 micro-library architecture designed for maximum portability across headless servers, daemons, CI/CD script runners, resource-restricted IoT microcontrollers, and desktop visual environments.

The **Multiplexing Subsystem** (`mod_mux`) enables high-performance data structure interleaving, binary record serialization, and virtual device channel stream multiplexing.

---

## 1. Headless & Micro-Library Architecture

The BASIC++ build system is fully modular. The engine uses foundational `libcore` for core execution and accumulative `libstandard` for extended features alongside fine-grained static micro-libraries.

### Key Features
- **Headless Execution Profile (`SET_HEADLESS_LIBS`)**: Compiles binaries (`bpp`, `bs`, or custom server dialects) linking `libcore` with zero SDL2, zero GUI, zero BGI raster graphics, and zero TUI editor dependencies.
- **Custom Dialect Composition**: Developers can pick and choose exact micro-libraries (e.g., linking `rt_arrays` and `mod_mux` while omitting `vdev_gfx` and `stmt_beep`) to build lightweight custom dialects for microcontrollers or embedded systems.
- **Self-Registering Feature Introspection**: Statements, built-in functions, `HELP` text, and `CATALOG` topics self-register upon initialization. Unlinked micro-libraries leave zero byte footprint and omit their keywords from `HELP`/`CATALOG`.

---

## 2. Multiplexing Syntax Reference

BASIC++ provides three unified syntax levels for data and channel multiplexing.

### 2.1 Statement Level (`MUX` and `DEMUX`)
Interleaves or de-interleaves 1D/2D numeric arrays, string vectors, or matrices.

```basic
DIM A(5), B(5), C(10)
MAT READ A
MAT READ B
MUX C, A, B               ' Interleaves A and B into C (A0, B0, A1, B1...)
MAT PRINT C

DEMUX A, B FROM C         ' De-interleaves C back into A and B
```

### 2.2 Matrix Level (`MAT MUX` and `MAT DEMUX`)
Dartmouth BASIC `MAT` family extensions for matrix row, column, or stride interleaving.

```basic
DIM M1(3, 3), M2(3, 3), M3(3, 6)
MAT READ M1
MAT READ M2
MAT MUX M3 = M1, M2       ' Interleaves matrix columns
MAT PRINT M3
```

### 2.3 Binary Record Serialization (`PACK$` and `UNPACK`)
Serializes heterogeneous variables, arrays, or strings into packed binary buffers.

```basic
n = 42.5
s$ = "BASIC++"
b$ = PACK$(n, s$)         ' Serializes number and string into binary buffer b$
UNPACK b$, out_n, out_s$  ' Deserializes buffer back into variables
```

### 2.4 Virtual Device Channel Multiplexing (`STREAM.MUX`)
Interleaves multiple virtual device handles (`#ch`) into a single tagged stream channel handle for IPC, TCP sockets, or memory pipes.

```basic
OPEN "CON:" FOR INPUT AS #1
OPEN "RAW:" FOR INPUT AS #2
OPEN "PIPE:" FOR OUTPUT AS #3
STREAM.MUX #3, #1, #2      ' Multiplexes #1 and #2 into #3
```

### 2.5 Microplexing Subsystem (`MICROPLEX$` and `BITMUX`)
Micro-granular character-level string interleaving and bit-width array interleaving.

#### Character-Level String Microplexing (`MICROPLEX$`)
`MICROPLEX$(str1$, str2$)` performs character-by-character interleaving of two string parameters into a single output string.

```basic
s1$ = "ABCDE"
s2$ = "12345"
res$ = MICROPLEX$(s1$, s2$)  ' Returns "A1B2C3D4E5"
```

#### Bit-Level Numeric Array Interleaving (`BITMUX`)
`BITMUX dst, src1, src2 [, BITS n]` performs bitwise interleaving of elements from two numeric arrays into a destination array using an optional bit width (1..32 bits, default 8).

```basic
DIM B1(2), B2(2), B3(2)
B1(1) = 15                 ' 0x0F
B2(1) = 240                ' 0xF0
BITMUX B3, B1, B2 BITS 8   ' Interleaves 8-bit slices into B3(1) (0xF0FF = 61455)
```

