<!--
Title:        Engine Implemented Features
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/
Generated:    no
Status:       Active
-->

# BASIC++ v6.5.2 Engine Implemented Features

## 1. Feature Status Matrix

This document tracks the verified implementation status and source code locations of all major subsystems and language statements in the BASIC++ v6.5.2 engine. All listed features are COMPLETE and verified against the decomposed micro-library architecture.

## 2. Core Language and Control Flow

| Feature | Status | Source Location |
|---------|--------|-----------------|
| Line-numbered program entry | COMPLETE | engine/src/vm/exec/exec_run.c |
| Immediate mode execution | COMPLETE | engine/src/vm/exec/exec_run.c |
| LET assignment | COMPLETE | engine/src/statements/variables/assignment/let.c |
| PRINT and DISP statements | COMPLETE | engine/src/statements/core/io/print.c |
| INPUT statement | COMPLETE | engine/src/statements/core/io/input.c |
| LINE INPUT | COMPLETE | engine/src/statements/core/io/line_input.c |
| IF / THEN / ELSE / ELSEIF / END IF | COMPLETE | engine/src/statements/core/program/if.c |
| FOR / NEXT (with STEP and BY) | COMPLETE | engine/src/statements/loops/counter/for.c |
| WHILE / WEND | COMPLETE | engine/src/statements/loops/conditional/while.c |
| DO / LOOP (WHILE / UNTIL, pre / post) | COMPLETE | engine/src/statements/loops/conditional/do.c |
| REPEAT / ENDLOOP (BASIC09 nested) | COMPLETE | engine/src/statements/loops/conditional/loop.c |
| SELECT CASE / END SELECT | COMPLETE | engine/src/statements/core/program/select.c |
| GOTO / ON...GOTO | COMPLETE | engine/src/statements/core/program/goto.c |
| GOSUB / RETURN / ON...GOSUB | COMPLETE | engine/src/statements/core/program/gosub.c |
| EXIT FOR / EXIT DO / EXIT LOOP | COMPLETE | engine/src/statements/loops/conditional/exit_loop.c |
| END / STOP | COMPLETE | engine/src/statements/core/program/end.c |
| REM / inline comments | COMPLETE | engine/src/statements/core/program/rem.c |
| DATA / READ / RESTORE | COMPLETE | engine/src/statements/variables/data/data.c |
| DIM / REDIM / ERASE | COMPLETE | engine/src/statements/variables/declaration/dim.c |
| OPTION BASE | COMPLETE | engine/src/statements/variables/declaration/option_base.c |
| DEF FN user functions | COMPLETE | engine/src/statements/core/program/def_fn.c |
| SUB / END SUB | COMPLETE | engine/src/statements/oop/sub.c |
| FUNCTION / END FUNCTION | COMPLETE | engine/src/statements/oop/function.c |
| CALL / INVOKE | COMPLETE | engine/src/statements/oop/call.c |
| SHARED / STATIC / LOCAL | COMPLETE | engine/src/statements/oop/shared.c |
| CONST / OPTION EXPLICIT | COMPLETE | engine/src/statements/variables/declaration/const.c |
| SWAP / EXCHANGE | COMPLETE | engine/src/statements/variables/assignment/swap.c |
| RANDOMIZE | COMPLETE | engine/src/statements/core/program/randomize.c |
| Postfix IF / UNLESS / WHILE | COMPLETE | engine/src/vm/exec/exec_run.c |

## 3. Expression Evaluator

| Feature | Status | Source Location |
|---------|--------|-----------------|
| Arithmetic: + - * / \ ^ MOD | COMPLETE | engine/src/eval/ops/eval_ops_arith.c |
| Comparison: = <> < > <= >= | COMPLETE | engine/src/eval/ops/eval_ops_rel.c |
| Logical: AND OR NOT XOR EQV IMP | COMPLETE | engine/src/eval/ops/eval_ops_logic.c |
| Bit shift: SHL SHR | COMPLETE | engine/src/eval/functions/bits/ |
| String concatenation (& and +) | COMPLETE | engine/src/eval/ops/eval_ops_str.c |
| Iterative Pratt parser (non-recursive) | COMPLETE | engine/src/eval/eval.c |
| Operator precedence lattice | COMPLETE | engine/src/eval/eval.c |
| RPN expression evaluation ({...}) | COMPLETE | engine/src/eval/rpn.c |
| Prefix Polish evaluation ([...]) | COMPLETE | engine/src/eval/pn.c |
| String interpolation (\"...${expr}...\") | COMPLETE | engine/src/eval/microplex.c |

## 4. Builtin Numeric and Math Functions

| Function Category | Status | Source Location |
|-------------------|--------|-----------------|
| ABS, SGN, INT, FIX, CINT, CLNG, CSNG, CDBL | COMPLETE | engine/src/eval/functions/math/ |
| CBOOL, CBYTE, CCUR, CSTR, ISNUMERIC, ISARRAY | COMPLETE | engine/src/eval/functions/types/ |
| SQR, SIN, COS, TAN, ATN, ASIN, ACOS, ATAN2 | COMPLETE | engine/src/eval/functions/math/ |
| SINH, COSH, TANH | COMPLETE | engine/src/eval/functions/math/ |
| LOG, LOG2, LOG10, EXP | COMPLETE | engine/src/eval/functions/math/ |
| PI, EPS, INF, MAXNUM | COMPLETE | engine/src/eval/functions/math/ |
| ROUND, FLOOR, CEIL, CLAMP, MIN, MAX, AVG, MED | COMPLETE | engine/src/eval/functions/math/ |
| LERP, DEGREES, RADIANS, ANGLE, REMAINDER | COMPLETE | engine/src/eval/functions/math/ |
| RND (Mersenne Twister & LCG engine) | COMPLETE | engine/src/eval/functions/math/rnd.c |
| Bit manipulation (READBIT, SETBIT, TOGGLEBIT, etc.) | COMPLETE | engine/src/eval/functions/bits/ |

## 5. String Functions

| Functions | Status | Source Location |
|-----------|--------|-----------------|
| LEN, LEFT$, RIGHT$, MID$ | COMPLETE | engine/src/eval/functions/string/ |
| INSTR, RINSTR | COMPLETE | engine/src/eval/functions/string/search/ |
| CHR$, ASC, STR$, VAL | COMPLETE | engine/src/eval/functions/string/ |
| HEX$, OCT$, BIN$ | COMPLETE | engine/src/eval/functions/string/ |
| SPACE$, STRING$ | COMPLETE | engine/src/eval/functions/string/ |
| UCASE$, LCASE$, TCASE$, TRIM$, LTRIM$, RTRIM$ | COMPLETE | engine/src/eval/functions/string/ |
| PACK$, UNPACK | COMPLETE | engine/src/eval/functions/string/ |

## 6. File and Stream I/O

| Feature | Status | Source Location |
|---------|--------|-----------------|
| OPEN / CLOSE (INPUT/OUTPUT/APPEND/RANDOM/BINARY) | COMPLETE | engine/src/statements/filesystem/file_ops/open.c |
| INPUT #, PRINT #, WRITE # | COMPLETE | engine/src/statements/filesystem/file_ops/ |
| GET / PUT (random record buffers) | COMPLETE | engine/src/statements/filesystem/file_ops/get.c |
| BGET / BPUT (binary block streams) | COMPLETE | engine/src/statements/filesystem/binary_ops/ |
| FIELD / LSET / RSET | COMPLETE | engine/src/statements/filesystem/file_ops/field.c |
| CVI, CVS, CVD, MKI$, MKS$, MKD$ | COMPLETE | engine/src/eval/functions/string/ |
| EOF, LOC, LOF, SEEK, FREEFILE | COMPLETE | engine/src/runtime/file.c |
| FILES, DIR$, KILL, NAME | COMPLETE | engine/src/statements/filesystem/dir_ops/ |
| MKDIR, RMDIR, CHDIR, PWD | COMPLETE | engine/src/statements/filesystem/dir_ops/ |
| LOCK / UNLOCK | COMPLETE | engine/src/statements/filesystem/file_ops/lock.c |

## 7. Graphics and Sound Subsystems

| Feature | Status | Source Location |
|---------|--------|-----------------|
| SCREEN mode configuration | COMPLETE | engine/src/statements/graphics/screen/screen.c |
| PSET / PRESET / LINE / CIRCLE / PAINT | COMPLETE | engine/src/statements/graphics/draw/ |
| DRAW macro graphics language | COMPLETE | engine/src/device/bgi/bgi_draw.c |
| PALETTE and color registers | COMPLETE | engine/src/device/bgi/bgi_palette.c |
| VIEW / WINDOW / PCOPY / POINT | COMPLETE | engine/src/device/bgi/bgi_window.c |
| BGI rasterizer and stroke fonts | COMPLETE | engine/src/device/bgi/bgi_raster.c |
| AAlib ASCII terminal graphics fallback | COMPLETE | engine/src/device/bgi/aalib/aalib.c |
| BEEP / SOUND / PLAY | COMPLETE | engine/src/statements/sound/ |
| CLS / LOCATE / COLOR / WIDTH | COMPLETE | engine/src/device/vcon.c |

## 8. Error Handling and Diagnostics

| Feature | Status | Source Location |
|---------|--------|-----------------|
| ON ERROR GOTO / RESUME / RESUME NEXT | COMPLETE | engine/src/statements/event/resume.c |
| TRY / CATCH / FINALLY / THROW | COMPLETE | engine/src/statements/event/try.c |
| TRON / TROFF statement execution tracing | COMPLETE | engine/src/statements/system/debug.c |
| ASSERT / TEST / ENDTEST / VERIFY | COMPLETE | engine/src/statements/system/debug.c |
| SELFTEST diagnostic suite | COMPLETE | engine/src/statements/introspection/selftest.c |
| DAP debug server | COMPLETE | engine/src/debug/dap_server.c |

## 9. Metaprogramming and Introspection

| Feature | Status | Source Location |
|---------|--------|-----------------|
| ALIAS keyword aliasing | COMPLETE | engine/src/statements/introspection/alias.c |
| OVERRIDE statement behavior replacement | COMPLETE | engine/src/statements/introspection/override.c |
| SCOPE namespace isolation | COMPLETE | engine/src/statements/introspection/scope.c |
| KEYWORD dynamic registration | COMPLETE | engine/src/statements/introspection/keyword.c |
| REMOVE keyword deregistration | COMPLETE | engine/src/statements/introspection/remove.c |

## 10. Core Subsystems and Virtual Devices

| Subsystem | Status | Source Location |
|-----------|--------|-----------------|
| Virtual device bus (VDev) | COMPLETE | engine/src/device/vdev.c |
| Virtual console (VCon) | COMPLETE | engine/src/device/vcon.c |
| Virtual filesystem (VFS) | COMPLETE | engine/src/runtime/vfs.c |
| Virtual network (VNet) | COMPLETE | engine/src/runtime/vnet.c |
| Security rings (6 privilege levels) | COMPLETE | engine/src/security/security.c |
| Dynamic module loader | COMPLETE | engine/src/module/module.c |
| BIOS emulation (libbios) | COMPLETE | engine/src/bios/bios_hal_vm.c |
| FujiNet virtual adapter | COMPLETE | engine/src/device/fujinet.c |
| Background task scheduler | COMPLETE | engine/src/runtime/task.c |
| Multiplexing (MUX/DEMUX/BITMUX) | COMPLETE | engine/src/runtime/mux.c |
| MAP and associative dictionaries | COMPLETE | engine/src/runtime/map.c |
| Unified dialect architecture | COMPLETE | engine/src/core/dialect.c |
