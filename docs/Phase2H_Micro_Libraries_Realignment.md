# Phase 2H: Micro-Libraries Granular Sub-Category Realignment

## 1. Overview
Phase 2H completes the comprehensive architectural realignment of all 136 function micro-libraries (`engine/src/eval/functions/`, `engine/include/eval/functions/`) and 150+ statement micro-libraries (`engine/src/statements/`, `engine/include/statements/`) from flat category directories into granular sub-category trees (e.g., `math/algebra/`, `math/trig/`, `math/linear_algebra/`, `string/manipulation/`, `string/conversion/`, `control/branch/`, `control/flow/`, `variables/declaration/`, `filesystem/file_ops/`, etc.).

All corresponding `#include` directives across the evaluator dispatchers (`math_fn.c`, `string_fn.c`, `conversion_fn.c`, `sys_fn.c`), statement handler headers (`stmt_handlers.h`), and `engine/CMakeLists.txt` build targets have been updated synchronously to reflect this granular architecture.

---

## 2. Granular Micro-Library Sub-Category Taxonomy

### 2.1 Function Micro-Libraries (`engine/src/eval/functions/` & `engine/include/eval/functions/`)

| Granular Sub-Tree | Functions Contained |
| :--- | :--- |
| **`math/algebra/`** (25) | `abs`, `ceil`, `clamp`, `eps`, `exp`, `fix`, `floor`, `fpt`, `inf`, `int`, `lerp`, `log`, `log10`, `log2`, `mag`, `max`, `maxnum`, `min`, `mod`, `pdif`, `remainder`, `round`, `sgn`, `sqr`, `truncate` |
| **`math/trig/`** (18) | `acos`, `angle`, `asin`, `atan2`, `atn`, `cos`, `cosh`, `cot`, `csc`, `degrees`, `hypot`, `pi`, `radians`, `sec`, `sin`, `sinh`, `tan`, `tanh` |
| **`math/linear_algebra/`** (7) | `comp`, `complex_fn`, `cross`, `det`, `dot`, `lbound`, `ubound` |
| **`math/financial/`** (1) | `financial` (PMT, FV, PV, NPER, RATE) |
| **`math/random/`** (1) | `rnd` |
| **`string/manipulation/`** (16) | `left`, `right`, `mid`, `ltrim`, `rtrim`, `trim`, `lcase`, `ucase`, `space`, `string`, `str_math`, `shuffle`, `seg`, `pick`, `pack`, `unpack` |
| **`string/conversion/`** (14) | `ascii_fn`, `chr`, `str`, `val`, `hex`, `oct`, `bin`, `rad`, `ath`, `cvt`, `mbf`, `num`, `ups`, `xlate` |
| **`string/search/`** (3) | `instr`, `index_fn`, `verify_fn` |
| **`string/format/`** (5) | `spc`, `tab`, `tek`, `ert`, `len` |
| **`bits/logic/`** (6) | `and`, `or`, `not`, `xor`, `eqv`, `imp` |
| **`bits/shift/`** (2) | `shl`, `shr` |
| **`bits/manipulation/`** (5) | `bitcount`, `readbit`, `resetbit`, `setbit`, `togglebit` |
| **`system/environment/`** (8) | `category`, `command_fn`, `environ`, `fre`, `moddir`, `pds_sys`, `sys_fn`, `spec_fn` |
| **`system/hardware/`** (3) | `peek`, `inp`, `magtape` |
| **`system/terminal/`** (4) | `csrlin`, `inkey`, `lpos`, `pos` |
| **`system/time/`** (7) | `clock_num`, `clock_str`, `date`, `ticks`, `time`, `time_fn`, `timer` |
| **`system/error/`** (2) | `erl`, `err_fn` |
| **`filesystem/status/`** (3) | `eof_fn`, `loc_fn`, `lof` |
| **`filesystem/descriptors/`** (3) | `fid`, `freefile`, `typ` |
| **`ui/graphics/`** (1) | `point_fn` |
| **`ui/dialogs/`** (2) | `vbdos_filebox`, `vbdos_fn` |

---

### 2.2 Statement Micro-Libraries (`engine/src/statements/` & `engine/include/statements/`)

| Granular Sub-Tree | Statements Contained |
| :--- | :--- |
| **`control/branch/`** (4) | `cause`, `handler`, `unless`, `when` |
| **`control/flow/`** (7) | `continue`, `doevents`, `retry`, `sleep`, `suspend`, `void`, `wait` |
| **`control/external/`** (4) | `declare`, `extend`, `external`, `scale` |
| **`core/io/`** (3) | `input`, `line_input`, `print` |
| **`core/program/`** (10) | `end`, `gosub`, `goto`, `if`, `map`, `randomize`, `rem`, `return`, `select`, `stop` |
| **`loops/counter/`** (2) | `for`, `next` |
| **`loops/conditional/`** (8) | `do`, `loop`, `while`, `wend`, `repeat`, `until`, `endloop`, `exit_loop` |
| **`variables/declaration/`** (14) | `dim`, `redim`, `const`, `def`, `defint`, `defsng`, `defdbl`, `deflng`, `defstr`, `common`, `global`, `share`, `public`, `complex` |
| **`variables/assignment/`** (10) | `let`, `lset`, `rset`, `swap`, `exchange`, `incr`, `mid_stmt`, `change`, `arrayfill`, `array_ext` |
| **`variables/data/`** (5) | `data`, `read`, `restore`, `erase`, `clr` |
| **`variables/options/`** (3) | `option`, `randomize`, `mux` |
| **`filesystem/file_ops/`** (12) | `open`, `close`, `input_file`, `print_file`, `write_file`, `get`, `put`, `seek`, `lock`, `append`, `field`, `record` |
| **`filesystem/dir_ops/`** (6) | `chdir`, `mkdir`, `rmdir`, `kill`, `name`, `files` |
| **`filesystem/binary_ops/`** (13) | `bload`, `bsave`, `brun`, `create`, `destroy`, `find`, `modify`, `prefix`, `rewind`, `text`, `vdim`, `backspace`, `assign` |
| **`graphics/screen/`** (7) | `screen`, `cls`, `color`, `palette`, `view`, `window`, `viewport` |
| **`graphics/draw/`** (8) | `circle`, `line`, `paint`, `draw`, `pset`, `preset`, `stmt_plot`, `picture` |
| **`sound/synthesis/`** (4) | `beep`, `sound`, `play`, `voice` |
| **`system/hardware/`** (7) | `out`, `poke`, `def_seg`, `defseg`, `def_usr`, `interrupt`, `stmt_bios` |
| **`system/environment/`** (7) | `shell`, `sys`, `version`, `session_stmts`, `pause`, `perform`, `invoke` |
| **`event/trapping/`** (6) | `on_error`, `on_key`, `on_timer`, `on_com`, `key`, `whenever` |
| **`matrices/io/`** (4) | `mat_print`, `mat_input`, `mat_read`, `mat_write` |
| **`matrices/ops/`** (1) | `mat_ops` |
| **`oop/structure/`** (6) | `class`, `function`, `param`, `type`, `with`, `enum` |
| **`db/isam/`** (1) | `isam` |
| **`debug/diagnostics/`** (4) | `check`, `debug`, `test`, `verify` |
| **`dialect/meta/`** (11) | `alias`, `category`, `compat`, `help`, `introspection`, `keyword`, `module`, `override`, `remove`, `scope`, `selftest` |
| **`ui/widgets/`** (6) | `msgbox`, `vbdos_controls`, `vbdos_widgets`, `joystick`, `mouse`, `home` |

---

## 3. Verification & Test Suite
- **Freestanding Unit Test Suites (8/8 Passed - 100%)**:
  - `runtime_freestanding_test.exe`: 9/9 suites PASSED (100%)
  - `boot_freestanding_test.exe`: 3/3 suites PASSED (100%)
  - `kernel_freestanding_test.exe`: 6/6 suites PASSED (100%)
  - `hardware_freestanding_test.exe`: 4/4 suites PASSED (100%)
  - `server_freestanding_test.exe`: 4/4 suites PASSED (100%)
  - `script_freestanding_test.exe`: 4/4 suites PASSED (100%)
  - `core_flex_freestanding_test.exe`: 5/5 suites PASSED (100%)
  - `standard_tui_freestanding_test.exe`: 5/5 suites PASSED (100%)
- **Master Regression Suites (Zero Regressions)**:
  - `tests/qb_vbdos_master.bas`: 10 / 10 Packages PASSED (100%)
  - `tests/vintage_ecosystems_master.bas`: 14 / 14 Packages PASSED (100%)
  - `tests/vintage_deep_fuzz_stress.bas`: 8 / 8 Tests PASSED (100%)
  - `bs.exe -c "SELFTEST"`: ALL SYSTEM TESTS PASSED
  - `bs.exe -c "PRINT 1+1"`: Verified (`2`).
