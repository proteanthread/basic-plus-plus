# BASIC++ v6.5.2 Engine Implemented Features

## 1. FEATURE STATUS MATRIX

This document tracks the implementation status of all major features in the v6.5.2 engine. Status values: COMPLETE (fully operational), PARTIAL (core functionality works, edge cases remain), PLANNED (designed but not yet implemented).

## 2. CORE LANGUAGE

| Feature | Status | Source |
|---------|--------|--------|
| Line-numbered program entry | COMPLETE | engine/src/vm/exec.c |
| Immediate mode execution | COMPLETE | engine/src/vm/exec.c |
| LET assignment | COMPLETE | engine/src/eval/eval.c |
| PRINT statement | COMPLETE | engine/src/statements/core/print.c |
| INPUT statement | COMPLETE | engine/src/statements/core/input.c |
| LINE INPUT | COMPLETE | engine/src/statements/core/input.c |
| IF/THEN/ELSE/ELSEIF/END IF | COMPLETE | engine/src/statements/core/if.c |
| FOR/NEXT with STEP and BY | COMPLETE | engine/src/statements/loops/for.c |
| WHILE/WEND | COMPLETE | engine/src/statements/loops/wend.c |
| DO/LOOP (WHILE/UNTIL, pre/post) | COMPLETE | engine/src/statements/loops/do.c |
| SELECT CASE/END SELECT | COMPLETE | engine/src/statements/core/select.c |
| GOTO | COMPLETE | engine/src/statements/core/goto.c |
| GOSUB/RETURN | COMPLETE | engine/src/statements/core/gosub.c |
| ON...GOTO / ON...GOSUB | COMPLETE | engine/src/statements/core/goto.c |
| EXIT FOR / EXIT DO | COMPLETE | engine/src/statements/loops/exit_loop.c |
| END / STOP | COMPLETE | engine/src/statements/core/end.c |
| REM / comments | COMPLETE | engine/src/statements/core/rem.c |
| DATA/READ/RESTORE | COMPLETE | engine/src/vm/data.c |
| DIM/REDIM/ERASE | COMPLETE | engine/src/runtime/arrays.c |
| OPTION BASE | COMPLETE | engine/src/runtime/arrays.c |
| DEF FN | COMPLETE | engine/src/eval/dispatch.c |
| SUB/END SUB | COMPLETE | engine/src/vm/stack.c |
| FUNCTION/END FUNCTION | COMPLETE | engine/src/vm/stack.c |
| CALL | COMPLETE | engine/src/vm/stack.c |
| DECLARE | COMPLETE | engine/src/vm/stack.c |
| SHARED/STATIC/LOCAL | COMPLETE | engine/src/runtime/variables.c |
| COMMON | COMPLETE | engine/src/runtime/variables.c |
| CONST | COMPLETE | engine/src/runtime/variables.c |
| SWAP | COMPLETE | engine/src/runtime/variables.c |
| DEFINT/DEFSNG/DEFDBL/DEFSTR | COMPLETE | engine/src/runtime/variables.c |
| OPTION EXPLICIT | COMPLETE | engine/src/runtime/variables.c |
| RANDOMIZE | COMPLETE | engine/src/statements/core/randomize.c |
| Postfix IF / UNLESS | COMPLETE | engine/src/parser/parser.c |

## 3. EXPRESSION EVALUATOR

| Feature | Status | Source |
|---------|--------|--------|
| Arithmetic: + - * / \ ^ MOD | COMPLETE | engine/src/eval/ops.c |
| Comparison: = <> < > <= >= | COMPLETE | engine/src/eval/ops.c |
| Logical: AND OR NOT XOR EQV IMP | COMPLETE | engine/src/eval/ops.c |
| Shift: SHL SHR | COMPLETE | engine/src/eval/functions/bits/ |
| String concatenation (+) | COMPLETE | engine/src/eval/ops.c |
| Iterative Pratt parser (non-recursive) | COMPLETE | engine/src/eval/eval.c |
| 14-level operator precedence | COMPLETE | engine/src/eval/eval.c |
| RPN expression support | COMPLETE | engine/src/eval/rpn.c |
| MICROPLEX$ string interpolation | COMPLETE | engine/src/eval/microplex.c |

## 4. NUMERIC FUNCTIONS

| Function | Status | Source |
|----------|--------|--------|
| ABS, SGN, INT, FIX, CINT, CSNG, CDBL | COMPLETE | engine/src/eval/functions/math/ |
| SQR, SIN, COS, TAN, ATN | COMPLETE | engine/src/eval/functions/math/ |
| ASIN, ACOS, ATAN2 | COMPLETE | engine/src/eval/functions/math/ |
| SINH, COSH, TANH | COMPLETE | engine/src/eval/functions/math/ |
| LOG, LOG2, LOG10, EXP | COMPLETE | engine/src/eval/functions/math/ |
| PI, EPS, INF, MAXNUM | COMPLETE | engine/src/eval/functions/math/ |
| ROUND, FLOOR, CEIL, CLAMP | COMPLETE | engine/src/eval/functions/math/ |
| MIN, MAX, AVG, MED | COMPLETE | engine/src/eval/functions/math/ |
| LERP, DEGREES, RADIANS, ANGLE | COMPLETE | engine/src/eval/functions/math/ |
| REMAINDER, TRUNCATE | COMPLETE | engine/src/eval/functions/math/ |
| RND | COMPLETE | engine/src/eval/functions/math/rnd.c |
| Bit functions (7 functions) | COMPLETE | engine/src/eval/functions/bits/ |

## 5. STRING FUNCTIONS

| Function | Status | Source |
|----------|--------|--------|
| LEN, LEFT$, RIGHT$, MID$ | COMPLETE | engine/src/eval/functions/string/ |
| INSTR | COMPLETE | engine/src/eval/functions/string/instr.c |
| CHR$, ASC, STR$, VAL | COMPLETE | engine/src/eval/functions/string/ |
| HEX$, OCT$, BIN$ | COMPLETE | engine/src/eval/functions/string/ |
| SPACE$, STRING$ | COMPLETE | engine/src/eval/functions/string/ |
| UCASE$, LCASE$, TCASE$, TRIM$ | COMPLETE | engine/src/eval/functions/string/ |
| LTRIM$, RTRIM$ | COMPLETE | engine/src/eval/functions/string/ |
| PACK$, UNPACK | COMPLETE | engine/src/eval/functions/string/ |

## 6. FILE I/O

| Feature | Status | Source |
|---------|--------|--------|
| OPEN/CLOSE (INPUT/OUTPUT/APPEND/RANDOM) | COMPLETE | engine/src/runtime/file.c |
| INPUT #, PRINT #, WRITE # | COMPLETE | engine/src/runtime/file.c |
| GET/PUT (random records) | COMPLETE | engine/src/runtime/file.c |
| BGET/BPUT (binary block I/O) | COMPLETE | engine/src/runtime/file.c |
| FIELD/LSET/RSET | COMPLETE | engine/src/runtime/file.c |
| CVI/CVS/CVD/MKI$/MKS$/MKD$ | COMPLETE | engine/src/eval/functions/string/ |
| EOF/LOC/LOF/SEEK/FREEFILE | COMPLETE | engine/src/runtime/file.c |
| FILES/DIR/KILL/NAME | COMPLETE | engine/src/runtime/file.c |
| MKDIR/RMDIR/CHDIR/PWD | COMPLETE | engine/src/runtime/file.c |
| LOCK/UNLOCK | COMPLETE | engine/src/runtime/file.c |

## 7. GRAPHICS AND SOUND

| Feature | Status | Source |
|---------|--------|--------|
| SCREEN mode selection | COMPLETE | engine/src/device/gfx.c |
| PSET/PRESET/LINE/CIRCLE/PAINT | COMPLETE | engine/src/device/bgi/ |
| DRAW graphics macro language | COMPLETE | engine/src/device/bgi/ |
| PALETTE | COMPLETE | engine/src/device/bgi/bgi_palette.c |
| VIEW/WINDOW/PCOPY/POINT | COMPLETE | engine/src/device/bgi/ |
| SET SCREEN/SET GRAPHICS/SET MODE | COMPLETE | engine/src/device/bgi/bgi_modes.c |
| BGI rasterizer | COMPLETE | engine/src/device/bgi/bgi_raster.c |
| BGI font system | COMPLETE | engine/src/device/bgi/bgi_font.c |
| AAlib ASCII art fallback | COMPLETE | engine/src/device/bgi/aalib/aalib.c |
| BEEP/SOUND/PLAY | COMPLETE | engine/src/device/ |
| CLS/LOCATE/COLOR/WIDTH | COMPLETE | engine/src/device/vcon.c |

## 8. ERROR HANDLING AND DEBUGGING

| Feature | Status | Source |
|---------|--------|--------|
| ON ERROR GOTO / RESUME | COMPLETE | engine/src/vm/error.c |
| TRY/CATCH/END TRY/THROW | COMPLETE | engine/src/vm/stack.c |
| WHEN EXCEPTION (ECMA-116) | COMPLETE | engine/src/vm/stack.c |
| TRON/TROFF/TRACE | COMPLETE | engine/src/debug/ |
| BREAK/CONT/VARS/DUMP/BACKTRACE | COMPLETE | engine/src/debug/ |
| ASSERT/TEST/ENDTEST/VERIFY/CHECK | COMPLETE | engine/src/statements/dialect/selftest.c |
| SELFTEST | COMPLETE | engine/src/statements/dialect/selftest.c |
| DAP debug server | COMPLETE | engine/src/debug/dap_server.c |

## 9. METAPROGRAMMING

| Feature | Status | Source |
|---------|--------|--------|
| ALIAS | COMPLETE | engine/src/statements/dialect/alias.c |
| OVERRIDE | COMPLETE | engine/src/statements/dialect/override.c |
| SCOPE | COMPLETE | engine/src/statements/dialect/scope.c |
| KEYWORD | COMPLETE | engine/src/statements/dialect/keyword.c |
| REMOVE | COMPLETE | engine/src/statements/dialect/remove.c |

## 10. SUBSYSTEMS

| Feature | Status | Source |
|---------|--------|--------|
| Virtual device bus (VDev) | COMPLETE | engine/src/device/vdev.c |
| Virtual console (VCon) | COMPLETE | engine/src/device/vcon.c |
| Virtual filesystem (VFS) | COMPLETE | engine/src/runtime/vfs.c |
| Virtual network (VNet) | COMPLETE | engine/src/runtime/vnet.c |
| Security system (6 levels) | COMPLETE | engine/src/security/security.c |
| Module system | COMPLETE | engine/src/module/module.c |
| BIOS emulation | COMPLETE | engine/src/bios/ |
| FujiNet emulation | COMPLETE | engine/src/device/fujinet.c |
| Background tasks | COMPLETE | engine/src/runtime/task.c |
| Gemini protocol | COMPLETE | engine/src/runtime/gemini.c |
| TUI editor multiplexer | COMPLETE | engine/src/editor/ |
| State save/restore | COMPLETE | engine/src/runtime/state.c |
| Event trapping (KEY/TIMER/COM/etc.) | COMPLETE | engine/src/vm/events.c |
| Alarm system (countdown + daily) | COMPLETE | engine/src/vm/events.c |
| MUX/DEMUX/BITMUX multiplexing | COMPLETE | engine/src/runtime/mux.c |
| MAP data structure | COMPLETE | engine/src/runtime/map.c |
| Dialect system | COMPLETE | engine/src/core/dialect.c |
| Specification system | COMPLETE | engine/src/runtime/spec.c |
