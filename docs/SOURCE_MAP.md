# BASIC++ v6.5.2 Source Map

## 1. PURPOSE

This document maps BASIC++ keywords, features, subsystems, and concepts to their implementing source files in the engine/ directory. Use this to locate the code responsible for any particular behavior.

## 2. CORE TYPE DEFINITIONS

| Header | Contents |
|--------|----------|
| engine/include/types/version.h | Version constants: 6.5.2, "Phoenix", 2026-08-06 |
| engine/include/types/config.h | Memory profiles, feature gates, edition macros |
| engine/include/types/types.h | BValue, ValueType, BppOpcode, BppError, BppErrorCategory |
| engine/include/types/errors.h | BppErrorCode enum (ERR_OK through ERR_PATH_NOT_FOUND) |

## 3. LEXER AND PARSER

| Source | Contents |
|--------|----------|
| engine/include/lexer/lexer.h | BppTokenType, BppKeywordId (367 keywords), BppToken, LexerContext API |
| engine/src/lexer/lexer.c | Lexer implementation: tokenization, keyword table, custom keyword registry |
| engine/src/parser/parser.c | Statement routing, block structure matching, parser state machine |

## 4. VM AND EXECUTION

| Source | Contents |
|--------|----------|
| engine/include/vm/vm.h | VMContext API, stack types, event trapping, error trapping, alias system |
| engine/include/vm/host.h | Host callback interface |
| engine/src/vm/context.c | VMContext allocation, subsystem initialization |
| engine/src/vm/control.c | Program flow: RUN, jump, GOSUB/RETURN stack management |
| engine/src/vm/data.c | DATA/READ/RESTORE: data table construction and pointer management |
| engine/src/vm/error.c | Error propagation: vm_set_error, vm_trigger_error_trap |
| engine/src/vm/events.c | Event polling: timer, key, COM, PEN, STRIG, PLAY, alarm traps |
| engine/src/vm/exec.c | Main execution loop: vm_execute_line, execute_single_statement |
| engine/src/vm/math.c | VM-level math helpers |
| engine/src/vm/stack.c | Stack management for GOSUB, FOR, WHILE, DO, SELECT, SUB |
| engine/src/vm/host.c | Host environment callbacks |

## 5. EXPRESSION EVALUATOR

| Source | Contents |
|--------|----------|
| engine/src/eval/eval.c | Main expression evaluator (iterative Pratt parser) |
| engine/src/eval/ast.c | AST node construction and manipulation |
| engine/src/eval/dispatch.c | Function call dispatch through registry |
| engine/src/eval/eval_builtins.c | Built-in function evaluation wrappers |
| engine/src/eval/helpers.c | Expression evaluation helper utilities |
| engine/src/eval/microplex.c | MICROPLEX$ string interpolation engine |
| engine/src/eval/ops.c | Arithmetic and comparison operators |
| engine/src/eval/rpn.c | RPN (Reverse Polish Notation) expression support |
| engine/src/eval/stack.c | Evaluation stack management |
| engine/src/eval/type.c | Type coercion and type checking |

## 6. RUNTIME SUBSYSTEMS

| Source | Contents |
|--------|----------|
| engine/include/runtime/strings.h | StringContext, BppStringRef, str_create/release/concat/mid |
| engine/src/runtime/strings.c | String heap implementation, reference counting, GC |
| engine/src/runtime/string_ext.c | Extended string operations |
| engine/include/runtime/variables.h | VariableContext, var_lookup/assign/clear/scope |
| engine/src/runtime/variables.c | Variable storage implementation |
| engine/include/runtime/arrays.h | ArrayContext, array operations |
| engine/src/runtime/arrays.c | Array storage, DIM/REDIM/ERASE implementation |
| engine/src/runtime/array_sort.c | ARRAY SORT implementation |
| engine/src/runtime/file.c | File I/O: OPEN, CLOSE, GET, PUT, sequential/random access |
| engine/include/runtime/funcreg.h | Function registry API |
| engine/src/runtime/funcreg.c | Built-in function registration and dispatch |
| engine/src/runtime/num_format.c | Numeric formatting (PRINT output formatting) |
| engine/src/runtime/print_using.c | PRINT USING format engine |
| engine/src/runtime/using_engine.c | USING format string parser |
| engine/src/runtime/override.c | OVERRIDE statement runtime support |
| engine/src/runtime/mux.c | MUX/DEMUX/BITMUX runtime implementation |
| engine/src/runtime/microplex.c | MICROPLEX runtime support (libhardware level) |
| engine/include/runtime/task.h | Task system API |
| engine/src/runtime/task.c | Background task management |
| engine/src/runtime/vfs.c | Virtual filesystem implementation |
| engine/src/runtime/vnet.c | Virtual network implementation |
| engine/src/runtime/gemini.c | Gemini protocol client |
| engine/src/runtime/crypto.c | Cryptographic functions |
| engine/src/runtime/state.c | State save/restore (STATESAVE/STATELOAD) |
| engine/src/runtime/spec.c | Language specification system |
| engine/src/runtime/map.c | MAP data structure (dictionary) |
| engine/src/runtime/map_serialize.c | MAP serialization |
| engine/include/runtime/metadata.h | Metadata registry API |
| engine/src/runtime/metadata.c | Runtime metadata for introspection |
| engine/src/runtime/keyword_props.c | Keyword property queries |
| engine/src/runtime/micro_lib_metadata.c | Micro-library metadata |

## 7. STATEMENT HANDLERS

### Core Statements
| Source | Keyword |
|--------|---------|
| engine/src/statements/core/end.c | END, STOP |
| engine/src/statements/core/gosub.c | GOSUB |
| engine/src/statements/core/goto.c | GOTO |
| engine/src/statements/core/if.c | IF/THEN/ELSE/ELSEIF/END IF |
| engine/src/statements/core/input.c | INPUT, LINE INPUT |
| engine/src/statements/core/print.c | PRINT, PRINT USING |
| engine/src/statements/core/randomize.c | RANDOMIZE |
| engine/src/statements/core/rem.c | REM |
| engine/src/statements/core/return.c | RETURN |
| engine/src/statements/core/select.c | SELECT CASE |
| engine/src/statements/core/stop.c | STOP |

### Loop Statements
| Source | Keyword |
|--------|---------|
| engine/src/statements/loops/for.c | FOR |
| engine/src/statements/loops/next.c | NEXT |
| engine/src/statements/loops/do.c | DO |
| engine/src/statements/loops/loop.c | LOOP |
| engine/src/statements/loops/wend.c | WEND |
| engine/src/statements/loops/exit_loop.c | EXIT FOR, EXIT DO |

### Dialect Statements
| Source | Keyword |
|--------|---------|
| engine/src/statements/dialect/help.c | HELP |
| engine/src/statements/dialect/introspection.c | CATALOG, INFO, VER, DEVICES |
| engine/src/statements/dialect/selftest.c | SELFTEST |
| engine/src/statements/dialect/alias.c | ALIAS |
| engine/src/statements/dialect/keyword.c | KEYWORD |
| engine/src/statements/dialect/override.c | OVERRIDE |
| engine/src/statements/dialect/remove.c | REMOVE |
| engine/src/statements/dialect/scope.c | SCOPE |

## 8. BUILT-IN FUNCTION SOURCES

### Math Functions
Located in engine/src/eval/functions/math/: abs.c, acos.c, angle.c, asin.c, atan2.c, atn.c, ceil.c, clamp.c, cos.c, degrees.c, eps.c, exp.c, fix.c, floor.c, inf.c, int.c, lerp.c, log.c, maxnum.c, pi.c, radians.c, remainder.c, rnd.c, round.c, sgn.c, sin.c, sqr.c, tan.c, truncate.c.

### String Functions
Located in engine/src/eval/functions/string/: bin.c, chr.c, hex.c, instr.c, lcase.c, left.c, len.c, ltrim.c, mid.c, oct.c, pack.c, right.c, rtrim.c, space.c, str.c, string.c, trim.c, ucase.c, unpack.c, val.c.

### System Functions
Located in engine/src/eval/functions/system/: clock_num.c, clock_str.c, date.c, environ.c, fre.c, inkey.c, inp.c, peek.c, ticks.c, time.c, timer.c.

### Bit Functions
Located in engine/src/eval/functions/bits/: bitcount.c, readbit.c, resetbit.c, setbit.c, shl.c, shr.c, togglebit.c.

## 9. PLATFORM ABSTRACTION

Located in engine/lib/platform/: plat_console.c, plat_fs.c, plat_sys.c, plat_time.c, plat_thread.c, plat_dl.c, plat_net.c, plat_regex.c, plat_clipboard.c.

## 10. VIRTUAL DEVICES

| Source | Device |
|--------|--------|
| engine/src/device/vdev.c | Virtual device bus manager |
| engine/src/device/vcon.c | Virtual console (cursor, color, screen) |
| engine/src/device/console.c | Host console adapter |
| engine/src/device/bus.c | Device bus registration and dispatch |
| engine/src/device/mux.c | I/O multiplexer |
| engine/src/device/bgi_bridge.c | BGI-to-BIOS video bridge |
| engine/src/device/bgi_text.c | BGI text rendering |
| engine/src/device/gfx.c | SDL2 graphics device |
| engine/src/device/fujinet.c | FujiNet hardware emulation |

## 11. BIOS EMULATION

Located in engine/src/bios/: bios.c, bios_at.c, bios_cpu8086.c, bios_hal_vm.c, bios_int10.c, bios_int13.c, bios_int16.c, bios_int1a.c, bios_jr.c, bios_pc.c, bios_xt.c.

## 12. BGI GRAPHICS

Located in engine/src/device/bgi/: bgi_autodetect.c, bgi_core.c, bgi_font.c, bgi_gfx.c, bgi_modes.c, bgi_palette.c, bgi_raster.c, aalib/aalib.c.
