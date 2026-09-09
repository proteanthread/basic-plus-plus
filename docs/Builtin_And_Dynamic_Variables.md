<!--
Title:        Builtin_And_Dynamic_Variables
Tier:         1
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/ops.c, engine/include/types/config.h
Generated:    no, hand-written
Status:       current
-->

# Built-in Variables, System Identification & Diagnostics Engine

The comprehensive technical reference for built-in constants, relational truth values, system identification strings, precision astronomical clocks, and multi-pool memory queries in BASIC++.

---

## 1. Constants & Relational Truth Invariant

In strict conformance with vintage BASIC standards and the **Relational Truth Value Invariant**, relational comparisons in BASIC++ evaluate to `-1` for True and `0` for False:

| Identifier | Type | Value / Description |
|:---|:---|:---|
| `TRUE` | Integer | Standard boolean True (`-1`). |
| `FALSE` | Integer | Standard boolean False (`0`). |
| `NOT TRUE` | Integer | Logical NOT of True evaluates to `0`. |
| `NOT FALSE` | Integer | Logical NOT of False evaluates to `-1`. |
| `PI`, `_PI`, `MATH.PI` | Double | Archimedes' constant ($\pi pprox 3.141592653589793$). |
| `INF`, `_INF`, `MATH.INF` | Double | Positive floating-point infinity ($\infty$). |
| `MAXNUM` | Double | Maximum double-precision float value ($pprox 1.797693 	imes 10^{308}$). |
| `EPS` | Double | Machine epsilon ($pprox 2.220446 	imes 10^{-16}$). |

---

## 2. System Identification & Host Environment

| Identifier | Type | Description |
|:---|:---|:---|
| `SYS.OS$`, `_OS$`, `OS$` | String | Operating system name (`"Windows"`, `"Linux"`, `"macOS"`, `"FreeBSD"`, `"ESP32"`). |
| `SYS.ARCH$`, `_ARCH$` | String | CPU architecture (`"x86_64"`, `"x86"`, `"arm64"`, `"arm"`, `"riscv"`, `"xtensa"`). |
| `SYS.EDITION$` | String | Active edition (`"Standard"`, `"Lite"`, `"Batch"`, `"IoT"`, or `"Compiler"`). |
| `SYS.COMPILER$` | String | C17 host compiler identification (`"MSVC"`, `"GCC"`, `"Clang"`). |
| `SYS.VERSION$` | String | Semantic version string (`"6.5.2"`). |
| `SYS.PID` | Integer | Current host process identifier. |
| `SYS.CORES` | Integer | Number of active logical CPU processing cores. |

---

## 3. Precision Clocks, Timers & Astronomical Dates

| Identifier / Function | Type | Description |
|:---|:---|:---|
| `UTC$` | String | ISO 8601 UTC date/time string (`"YYYY-MM-DDTHH:MM:SS"`). |
| `CLOCK$` | String | ISO 8601 Local date/time string (`"YYYY-MM-DDTHH:MM:SS"`). |
| `TIME$` | String | Formatted local time string (`"HH:MM:SS"`). |
| `DATE$` | String | Formatted local date string (`"YYYY-MM-DD"`). |
| `TIMER` | Double | Elapsed seconds since midnight with microsecond precision. |
| `TICKS`, `TICKS_MS` | Double | Monotonic millisecond uptime counter since engine boot. |
| `UNIXTIME` | Double | Unix epoch timestamp in seconds since 1970-01-01 00:00:00 UTC. |
| `JD`, `JULIAN` | Double | Astronomical Julian Day Number (e.g. `2461283.05`). |

---

## 4. Multi-Pool Memory Queries (`MEM` and `FRE`)

| Query | Description |
|:---|:---|
| `MEM` | Total memory pool allocated to engine (`671088640` desktop, `402653184` lite, `67108864` batch, `2097152` iot). |
| `FRE(0)` / `FRE` | Available general RAM heap in bytes. |
| `FRE(-1)` | Variable symbol table pool capacity. |
| `FRE(-2)` | String dynamic heap free capacity. |
| `FRE(-3)` | Bytecode and program AST memory pool capacity. |
| `FRE(-4)` | Execution stack and call-frame capacity. |
