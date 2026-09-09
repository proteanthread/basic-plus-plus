<!--
Title:        Micro-Libraries Granular Sub-Category Architecture
Tier:         2
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/
Generated:    no
Status:       Active
-->

# Micro-Libraries Granular Sub-Category Architecture

## 1. Architectural Overview

The BASIC++ v6.5.2 codebase decomposes all runtime statement handlers and expression evaluation functions into granular, single-responsibility C17 micro-libraries. Rather than aggregating dozens of unrelated procedures into massive monolithic units, every function and statement resides in its own compilation unit under a strict two-tier sub-category directory taxonomy.

This architecture enforces strict ISO C17 (§4 ¶6) freestanding compliance, eliminates circular `#include` dependencies, prevents symbol collision, accelerates incremental compilation, and allows targeted deployment to resource-constrained embedded targets (`iot`) and full desktop environments (`baspp`).

---

## 2. Granular Micro-Library Sub-Category Taxonomy

### 2.1 Function Micro-Libraries (`engine/src/eval/functions/` & `engine/include/eval/functions/`)

| Granular Sub-Tree | Functions Contained | Count |
| :--- | :--- | :--- |
| **`math/algebra/`** | `abs`, `ceil`, `clamp`, `eps`, `exp`, `fix`, `floor`, `fpt`, `inf`, `int`, `lerp`, `log`, `log10`, `log2`, `mag`, `max`, `maxnum`, `min`, `mod`, `pdif`, `remainder`, `round`, `sgn`, `sqr`, `truncate` | 25 |
| **`math/trig/`** | `acos`, `angle`, `asin`, `atan2`, `atn`, `cos`, `cosh`, `cot`, `csc`, `degrees`, `hypot`, `pi`, `radians`, `sec`, `sin`, `sinh`, `tan`, `tanh` | 18 |
| **`math/linear_algebra/`** | `comp`, `complex_fn`, `cross`, `det`, `dot`, `lbound`, `ubound` | 7 |
| **`math/financial/`** | `financial` (PMT, FV, PV, NPER, RATE) | 1 |
| **`math/random/`** | `rnd` | 1 |
| **`string/manipulation/`** | `left`, `right`, `mid`, `ltrim`, `rtrim`, `trim`, `lcase`, `ucase`, `space`, `string`, `str_math`, `shuffle`, `seg`, `pick`, `pack`, `unpack` | 16 |
| **`string/conversion/`** | `ascii_fn`, `chr`, `str`, `val`, `hex`, `oct`, `bin`, `rad`, `ath`, `cvt`, `mbf`, `num`, `ups`, `xlate` | 14 |
| **`string/search/`** | `instr`, `index_fn`, `verify_fn` | 3 |
| **`string/format/`** | `spc`, `tab`, `tek`, `ert`, `len` | 5 |
| **`bits/logic/`** | `and`, `or`, `not`, `xor`, `eqv`, `imp` | 6 |
| **`bits/shift/`** | `shl`, `shr` | 2 |
| **`bits/manipulation/`** | `bitcount`, `readbit`, `resetbit`, `setbit`, `togglebit` | 5 |
| **`system/environment/`** | `category`, `command_fn`, `environ`, `fre`, `moddir`, `pds_sys`, `sys_fn`, `spec_fn` | 8 |
| **`system/hardware/`** | `peek`, `inp`, `magtape` | 3 |
| **`system/terminal/`** | `csrlin`, `inkey`, `lpos`, `pos` | 4 |
| **`system/time/`** | `clock_num`, `clock_str`, `date`, `ticks`, `time`, `time_fn`, `timer` | 7 |
| **`system/error/`** | `erl`, `err_fn` | 2 |
| **`filesystem/status/`** | `eof_fn`, `loc_fn`, `lof` | 3 |
| **`filesystem/descriptors/`** | `fid`, `freefile`, `typ` | 3 |
| **`ui/graphics/`** | `point_fn` | 1 |
| **`ui/dialogs/`** | `vbdos_filebox`, `vbdos_fn` | 2 |

---

### 2.2 Statement Micro-Libraries (`engine/src/statements/` & `engine/include/statements/`)

| Granular Sub-Tree | Statements Contained | Count |
| :--- | :--- | :--- |
| **`control/branch/`** | `cause`, `handler`, `unless`, `when` | 4 |
| **`control/flow/`** | `continue`, `doevents`, `retry`, `sleep`, `suspend`, `void`, `wait` | 7 |
| **`control/external/`** | `declare`, `extend`, `external`, `scale` | 4 |
| **`core/io/`** | `input`, `line_input`, `print` | 3 |
| **`core/program/`** | `end`, `gosub`, `goto`, `if`, `map`, `randomize`, `rem`, `return`, `select`, `stop` | 10 |
| **`loops/counter/`** | `for`, `next` | 2 |
| **`loops/conditional/`** | `do`, `loop`, `while`, `wend`, `repeat`, `until`, `endloop`, `exit_loop` | 8 |
| **`variables/declaration/`** | `dim`, `redim`, `const`, `def`, `defint`, `defsng`, `defdbl`, `deflng`, `defstr`, `common`, `global`, `share`, `public`, `complex` | 14 |
| **`variables/assignment/`** | `let`, `lset`, `rset`, `swap`, `exchange`, `incr`, `mid_stmt`, `change`, `arrayfill`, `array_ext` | 10 |
| **`variables/data/`** | `data`, `read`, `restore`, `erase`, `clr` | 5 |
| **`variables/options/`** | `option`, `randomize`, `mux` | 3 |
| **`filesystem/file_ops/`** | `open`, `close`, `input_file`, `print_file`, `write_file`, `get`, `put`, `seek`, `lock`, `append`, `field`, `record` | 12 |
| **`filesystem/dir_ops/`** | `chdir`, `mkdir`, `rmdir`, `kill`, `name`, `files` | 6 |
| **`filesystem/binary_ops/`** | `bload`, `bsave`, `brun`, `create`, `destroy`, `find`, `modify`, `prefix`, `rewind`, `text`, `vdim`, `backspace`, `assign` | 13 |
| **`graphics/screen/`** | `screen`, `cls`, `color`, `palette`, `view`, `window`, `viewport` | 7 |
| **`graphics/draw/`** | `circle`, `line`, `paint`, `draw`, `pset`, `preset`, `stmt_plot`, `picture` | 8 |
| **`sound/synthesis/`** | `beep`, `sound`, `play`, `voice` | 4 |
| **`interop/ffi/`** | `call`, `extern`, `poke`, `out`, `int86` | 5 |
| **`memory/peek_poke/`** | `def_seg`, `setmem`, `vcopy` | 3 |
| **`network/socket_ops/`** | `sock_open`, `sock_close`, `sock_send`, `sock_recv`, `sock_listen`, `sock_poll` | 6 |
| **`iot/hardware_io/`** | `pin_mode`, `dwrite`, `dread`, `awrite`, `aread`, `i2c_write`, `spi_transfer`, `pwm_set` | 8 |
| **`retro/vintage_ops/`** | `poke_vram`, `vpeek`, `pr_hash`, `xio`, `gr`, `hgr` | 6 |

---

## 3. Architectural Impact & Benefits

1. **Deterministic Compilation Bounding**: Each individual compilation unit compiles in sub-second time, enabling parallel multi-processor builds (`/MP` on MSVC, `-j` on GCC).
2. **Decoupled Executables**: Build targets (`baspp`, `bpp`, `bs`, `iot`) link only the micro-libraries mandated by their target profile, eliminating dead-code overhead.
3. **Traceability & Isolation**: Memory leaks, type mismatch bugs, or platform-specific regressions are cleanly isolated to a single, self-contained `< 800`-line source file.
