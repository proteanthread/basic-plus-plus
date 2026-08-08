# BASIC++ v6.5.2 Engine Semantic Source Map (`SOURCE_MAP.md`)

This document provides a comprehensive semantic mapping linking all keywords, statements, built-in functions, virtual devices, and runtime features to their precise C source file, header surface, CMake micro-library target, and documentation references.

---

## 1. Feature & Keyword Semantic Mapping Matrix

| Category | Keywords / Features | C Source File (`engine/src/`) | C Header Surface (`engine/include/`) | CMake Target | Documentation Page |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **Metaprogramming** | `ALIAS`, `OPERATOR ALIAS` | `statements/dialect/alias.c` | `statements/dialect/alias.h` | `stmt_alias` | [DEVELOPER_GUIDE.md](file:///c:/Users/rtdos/GitHub/basic-plus-plus/docs/DEVELOPER_GUIDE.md) |
| **Metaprogramming** | `SCOPE`, `SCOPE DISABLE`, `SCOPE HOOK`, `SCOPE PRIVATE` | `statements/dialect/scope.c`, `scope/scope.c` | `statements/dialect/scope.h`, `scope/scope.h` | `stmt_scope` | [DEVELOPER_GUIDE.md](file:///c:/Users/rtdos/GitHub/basic-plus-plus/docs/DEVELOPER_GUIDE.md) |
| **Metaprogramming** | `KEYWORD`, `KEYWORD SET`, `KEYWORD GET`, `KEYWORD LIST` | `statements/dialect/keyword.c`, `runtime/keyword_props.c` | `statements/dialect/keyword.h`, `runtime/keyword_props.h` | `stmt_keyword` | [DEVELOPER_GUIDE.md](file:///c:/Users/rtdos/GitHub/basic-plus-plus/docs/DEVELOPER_GUIDE.md) |
| **Metaprogramming** | `OVERRIDE`, `OVERRIDE WITH GOSUB`, `OVERRIDE WITH SUB` | `statements/dialect/override.c`, `runtime/override.c` | `statements/dialect/override.h`, `runtime/override.h` | `stmt_override` | [DEVELOPER_GUIDE.md](file:///c:/Users/rtdos/GitHub/basic-plus-plus/docs/DEVELOPER_GUIDE.md) |
| **OOP & Modules** | `MODULE`, `MODULE END`, `MODULE IMPORT` | `statements/oop/module.c`, `module/module.c` | `statements/dialect/module_stmt.h`, `module/module.h` | `stmt_module` | [DEVELOPER_GUIDE.md](file:///c:/Users/rtdos/GitHub/basic-plus-plus/docs/DEVELOPER_GUIDE.md) |
| **OOP Classes** | `CLASS`, `END CLASS`, `METHOD`, `PROPERTY`, `NEW`, `DELETE` | `statements/oop/*.c`, `core/struct.c` | `statements/oop/*.h`, `core/struct.h` | `stmt_class`, `stmt_method`, etc. | [User_Defined_Types.md](file:///c:/Users/rtdos/GitHub/basic-plus-plus/docs/User_Defined_Types.md) |
| **Control Flow** | `IF`, `THEN`, `ELSE`, `ELSEIF`, `END IF` | `statements/core/if.c` | `statements/core/if.h` | `stmt_if` | [Structured_BASIC.md](file:///c:/Users/rtdos/GitHub/basic-plus-plus/docs/Structured_BASIC.md) |
| **Control Flow** | `FOR`, `TO`, `STEP`, `NEXT` | `statements/loops/for.c`, `next.c` | `statements/loops/for.h`, `next.h` | `stmt_for`, `stmt_next` | [Structured_BASIC.md](file:///c:/Users/rtdos/GitHub/basic-plus-plus/docs/Structured_BASIC.md) |
| **Control Flow** | `DO`, `LOOP`, `WHILE`, `UNTIL` | `statements/loops/do.c`, `loop.c` | `statements/loops/do.h`, `loop.h` | `stmt_do`, `stmt_loop` | [Structured_BASIC.md](file:///c:/Users/rtdos/GitHub/basic-plus-plus/docs/Structured_BASIC.md) |
| **Control Flow** | `WHILE`, `WEND` | `statements/loops/while.c`, `wend.c` | `statements/loops/while.h`, `wend.h` | `stmt_while`, `stmt_wend` | [Structured_BASIC.md](file:///c:/Users/rtdos/GitHub/basic-plus-plus/docs/Structured_BASIC.md) |
| **Control Flow** | `GOTO`, `GOSUB`, `RETURN`, `ON...GOTO` | `statements/core/goto.c`, `gosub.c`, `return.c`, `on_goto.c` | `statements/core/goto.h`, `gosub.h`, `return.h`, `on_goto.h` | `stmt_goto`, `stmt_gosub`, `stmt_return` | [Structured_BASIC.md](file:///c:/Users/rtdos/GitHub/basic-plus-plus/docs/Structured_BASIC.md) |
| **Console I/O** | `PRINT`, `PRINT USING`, `INPUT`, `LINE INPUT`, `LOCATE` | `statements/core/print.c`, `input.c`, `statements/io/locate.c` | `statements/core/print.h`, `input.h`, `statements/io/locate.h` | `stmt_print`, `stmt_input`, `stmt_locate` | [Screen_And_Console.md](file:///c:/Users/rtdos/GitHub/basic-plus-plus/docs/Screen_And_Console.md) |
| **File I/O** | `OPEN`, `CLOSE`, `GET`, `PUT`, `FIELD`, `SEEK`, `FILES`, `KILL`, `NAME` | `statements/filesystem/*.c` | `statements/filesystem/*.h` | `stmt_open`, `stmt_close`, `stmt_file_io` | [File_IO.md](file:///c:/Users/rtdos/GitHub/basic-plus-plus/docs/File_IO.md) |
| **Graphics** | `SCREEN`, `CLS`, `COLOR`, `LINE`, `CIRCLE`, `PSET`, `PRESET`, `DRAW`, `PAINT` | `statements/graphics/*.c` | `statements/graphics/*.h` | `stmt_screen`, `stmt_color`, `stmt_graphics` | [Graphics_Sound.md](file:///c:/Users/rtdos/GitHub/basic-plus-plus/docs/Graphics_Sound.md) |
| **Audio** | `BEEP`, `SOUND`, `PLAY`, `VOICE` | `statements/sound/*.c` | `statements/sound/*.h` | `stmt_beep`, `stmt_sound`, `stmt_play` | [Graphics_Sound.md](file:///c:/Users/rtdos/GitHub/basic-plus-plus/docs/Graphics_Sound.md) |
| **Matrices** | `MAT READ`, `MAT PRINT`, `MAT INPUT`, `MAT ADD`, `MAT INV` | `statements/matrices/*.c` | `statements/matrices/*.h` | `stmt_mat_read`, `stmt_mat_print`, `stmt_matrix` | [Arrays_And_Matrices.md](file:///c:/Users/rtdos/GitHub/basic-plus-plus/docs/Arrays_And_Matrices.md) |
| **Variables & Arrays**| `DIM`, `LET`, `REDIM`, `ERASE`, `OPTION BASE`, `OPTION EXPLICIT` | `statements/variables/*.c` | `statements/variables/*.h` | `stmt_dim`, `stmt_let`, `stmt_vars` | [Variables_Memory](file:///c:/Users/rtdos/GitHub/basic-plus-plus/docs/Variables_Memory) |
| **Diagnostics** | `SELFTEST`, `TRACE`, `ASSERT` | `statements/dialect/selftest.c`, `debug/logger.c` | `statements/dialect/selftest.h`, `debug/logger.h` | `stmt_selftest`, `debug` | [Debugging_And_Testing.md](file:///c:/Users/rtdos/GitHub/basic-plus-plus/docs/Debugging_And_Testing.md) |
| **System & Hardware**| `PEEK`, `POKE`, `DEF SEG`, `INP`, `OUT`, `BIOS` | `statements/system/*.c`, `bios/*.c` | `statements/system/*.h`, `bios/*.h` | `stmt_poke`, `stmt_def_seg`, `bios` | [Systems_Programming.md](file:///c:/Users/rtdos/GitHub/basic-plus-plus/docs/Systems_Programming.md) |
| **Event Handling** | `ON ERROR`, `RESUME`, `ON KEY`, `ON TIMER`, `ON COM`, `TRY`, `CATCH` | `statements/event/*.c` | `statements/event/*.h` | `stmt_on_error`, `stmt_try`, `stmt_event` | [Event_Trapping.md](file:///c:/Users/rtdos/GitHub/basic-plus-plus/docs/Event_Trapping.md) |
| **Math Functions** | `SIN`, `COS`, `TAN`, `ATN`, `EXP`, `LOG`, `SQR`, `ABS`, `SGN`, `RND` | `eval/functions/math/*.c` | `eval/functions/math/*.h` | `sin`, `cos`, `tan`, `atn`, etc. | [Standard_Library.md](file:///c:/Users/rtdos/GitHub/basic-plus-plus/docs/Standard_Library.md) |
| **String Functions**| `LEFT$`, `RIGHT$`, `MID$`, `CHR$`, `ASC`, `VAL`, `STR$`, `HEX$`, `OCT$` | `eval/functions/string/*.c` | `eval/functions/string/*.h` | `left`, `right`, `mid`, `chr`, etc. | [String_Handling.md](file:///c:/Users/rtdos/GitHub/basic-plus-plus/docs/String_Handling.md) |
| **Bit Functions** | `BITAND`, `BITOR`, `BITXOR`, `BITNOT`, `_SHL`, `_SHR` | `eval/functions/bits/*.c` | `eval/functions/bits/*.h` | `bitand`, `bitor`, `shl`, `shr` | [Standard_Library.md](file:///c:/Users/rtdos/GitHub/basic-plus-plus/docs/Standard_Library.md) |
| **System Functions**| `FRE`, `VARPTR`, `DATE$`, `TIME$`, `TIMER`, `INKEY$`, `CLOCK$` | `eval/functions/system/*.c` | `eval/functions/system/*.h` | `fre`, `date`, `time`, `timer`, `inkey` | [System_And_Environment.md](file:///c:/Users/rtdos/GitHub/basic-plus-plus/docs/System_And_Environment.md) |

---

## 2. Modular Static Library Target Spectrum (`libboot` -> `libadvanced`)

1. **`libboot`**: Core boot phase sequence controller (`common.c`).
2. **`libplatform`**: OS platform abstraction (`plat_console`, `plat_fs`, `plat_sys`, `plat_time`, `plat_thread`, `plat_dl`, `plat_net`, `plat_regex`, `plat_clipboard`). Encapsulates all Win32 vs POSIX `#ifdef` logic. Upper layers have zero OS-specific code.
3. **`libkernel`**: Core VM context, lexer, memory manager, security sandbox, BIOS virtualization, virtual device bus (`vdev`, `vcon`).
4. **`libengine`**: AST evaluator, parser, runtime functions, variables, strings, bytecode execution loop.
5. **`libhardware`**: Segmented memory (`vmem`), BGI rasterizer, FujiNet hardware emulation.
6. **`libserver`**: Network socket operations (`vnet`), Gemini protocol, background tasks, VFS, crypto, regex.
7. **`libscript`**: Headless batch script runner and file I/O operations (`file.c`).
8. **`libcore`**: Foundational REPL, numeric formatting, print using engine, metadata registry, map serialization.
9. **`libflex`**: Dynamic metaprogramming (`ALIAS`, `OVERRIDE`, `SCOPE`), module loader, mathext, arrayext.
10. **`libstandard`**: Standard TUI workstation, multi-window TUI editor multiplexer, DAP debug server.
11. **`libadvanced`**: Desktop visual graphics (`bgi_core`, `aalib`, `gfx.c`), multimedia, SDL2/OpenGL bindings.
12. **`libext`**: Open-ended user and third-party extension template.

---

## 3. Developer Extensibility Quick Reference

To add a new keyword or statement:
1. Register in `BppKeywordId` enum (`engine/include/lexer/lexer.h`).
2. Add keyword text string in `k_keywords[]` (`engine/src/lexer/lexer.c`).
3. Create statement handler `stmt_<name>.c` and header `<name>.h`.
4. Register handler in `engine/src/vm/exec.c` via `stmt_register()`.
5. Add micro-library target to `engine/CMakeLists.txt` for `libcore` (or `libstandard` if extended feature).
6. Register help entry in `engine/src/runtime/help_data.h` and update `docs/` and `help/`.
