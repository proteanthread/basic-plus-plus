<!--
Title:        System_And_Environment
Tier:         2
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/system/environment/, engine/src/statements/system/
Generated:    no, hand-written
Status:       current
-->

# BASIC++ v6.5.2 System & Environment Architecture

The authoritative specification for environment variables, command-line arguments, system clocks, memory inspection, and virtual hardware access in BASIC++ v6.5.2.

---

## 1. Operating System Environment & Arguments

BASIC++ provides direct, portable access to the host operating system environment:

- **`ENVIRON$("VARIABLE")`**: Returns the value of host environment variable `VARIABLE`, or an empty string if undefined.
- **`ENVIRON "VAR=VALUE"`**: Sets or exports an environment variable in the active process environment.
- **`COMMAND$`**: Returns the raw command-line argument string passed to the script or executable.

---

## 2. System Clock, Date, and High-Resolution Timers

- **`DATE$`**: Returns the current host calendar date in `MM-DD-YYYY` format. In administrative sessions, assigning `DATE$ = "MM-DD-YYYY"` sets the system date.
- **`TIME$`**: Returns the current 24-hour clock time in `HH:MM:SS` format. Assigning `TIME$ = "HH:MM:SS"` sets the system clock.
- **`TIMER`**: Built-in numeric function returning the number of elapsed seconds since midnight with sub-millisecond precision.

---

## 3. Memory and Hardware Access (Virtualization & Safety)

In hosted executables (`baspp`, `bpp`, `bs`), direct hardware and physical memory accesses are safely virtualized within the `libbios` and `vmem` sandboxes:

- **`PEEK(address)`**: Reads a byte from the virtualized memory space at `address`.
- **`POKE address, value`**: Writes byte `value` (0-255) to the virtualized memory space at `address`.
- **`DEF SEG [= segment]`**: Sets the active segment base address for subsequent `PEEK`, `POKE`, `BSAVE`, and `BLOAD` operations.
- **`INP(port)`**: Reads a byte from the virtualized hardware port `port`.
- **`OUT port, value`**: Writes byte `value` to the virtualized hardware port `port`.
- **`FRE(0)`**: Returns the remaining free program memory pool size in bytes.

---

## 4. Process Lifecycle

- **`SYSTEM [exit_code]`**: Immediately terminates program execution, reclaims allocated VM resources, and returns control to the operating system with `exit_code` (default `0`).

---

## 5. Example: Environment and Timing Inspection

```basic
10 REM System & Environment Demo
20 PRINT "OS Path:     "; ENVIRON$("PATH")
30 PRINT "Command arg: "; COMMAND$
40 PRINT "System Date: "; DATE$; " Time: "; TIME$
50 Start = TIMER
60 FOR I = 1 TO 100000 : NEXT I
70 Elapsed = TIMER - Start
80 PRINT "Elapsed loop time: "; Elapsed; " seconds"
90 PRINT "Free memory: "; FRE(0); " bytes"
```
