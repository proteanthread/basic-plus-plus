# BASIC++ v6.5.2 Engine Physical Source Tree (`SOURCE_TREE.md`)

This document defines the strict physical layout of all directories, source files, header files, build manifests, test suites, and offline documentation in the BASIC++ v6.5.2 codebase.

---

## 1. Engine Include Directory (`engine/include/`)

```
engine/include/
├── bios/                  # Hardware abstraction & BIOS virtual device interfaces
│   ├── bios.h             # Memory segments, interrupt vector table, port I/O
│   └── bios_types.h       # Registers, IVT structures, CGA/EGA video constants
├── core/                  # Core engine definitions & memory allocations
│   ├── dialect.h          # BppDialect configuration & keyword tables
│   ├── memory.h           # Heap allocator, memory pools, dynamic growth
│   └── string.h           # Ref-counted immutable BppString representation
├── debug/                 # Diagnostic execution & selftest headers
│   └── debug.h            # SELFTEST harness, trace hooks, line assertion
├── device/                # Virtual device abstraction subsystem (VDev)
│   ├── vcon.h             # Console virtual device (CON:) cursor/buffer abstraction
│   ├── vdev.h             # Master virtual device registry & I/O dispatch
│   └── vfile.h            # Binary, ASCII, sequential, and random file handles
├── editor/                # Built-in TUI multi-window code editor
│   ├── edit_buffer.h      # Line-buffered text buffer, undo/redo stacks
│   └── editor.h            # Screen redraw, syntax highlighter, key maps
├── eval/                  # AST evaluation engine & built-in functions
│   ├── dispatch.h         # Built-in function registry & keyword expression routing
│   ├── eval.h             # AST expression node evaluator & operator precedence
│   └── functions/         # Built-in mathematical, string, array & system functions
│       ├── array_funcs.h  # LBOUND, UBOUND, DIM, MATRIX functions
│       ├── bit_funcs.h    # _SHL, _SHR, _ROL, _ROR, BITAND, BITOR
│       ├── date_funcs.h   # DATE$, TIME$, TIMER, CLOCK$
│       ├── dev_funcs.h    # INP, PEEK, INKEY$, STICK, STRIG, PEN, PLAY$
│       ├── math_funcs.h   # SIN, COS, TAN, ATN, EXP, LOG, SQR, ABS, SGN, RND
│       ├── str_funcs.h    # LEFT$, RIGHT$, MID$, CHR$, ASC, VAL, STR$, HEX$, OCT$
│       └── sys_funcs.h    # FRE, VARPTR, PEEK, PEEK16, PEEK32, ENVIRON$
├── lexer/                 # Tokenizer & keyword dictionary
│   └── lexer.h            # BppToken struct, BppKeywordId enum (KW_ALIAS, KW_SCOPE, etc.)
├── memory/                # Extended memory segment management
│   └── vmem.h             # Segmented EMS/XMS memory emulation & DEF SEG backing
├── module/                # Scope module & namespace system
│   └── module.h           # BppModule registry, export table, namespace stack
├── parser/                # Ephemeral AST parser
│   └── parser.h           # BppParser context, expression parser, statement classifier
├── platform/              # Decoupled OS platform abstraction system (libplatform)
│   ├── plat_clipboard.h   # System clipboard API
│   ├── plat_console.h     # Terminal & pure ASCII console I/O
│   ├── plat_dl.h          # Dynamic library loading (.dll / .so)
│   ├── plat_fs.h          # Cross-platform filesystem & path normalization
│   ├── plat_net.h         # TCP/UDP network socket abstraction
│   ├── plat_regex.h       # POSIX / PCRE regex matching
│   ├── plat_sys.h         # Host platform diagnostics & system info
│   ├── plat_thread.h      # Threading, mutexes, & concurrency
│   └── plat_time.h        # High-precision timer, sleep, & thread-safe localtime
├── runtime/               # Dynamic execution features & registries
│   ├── help_data.h        # Built-in HELP text database and catalog entries
│   ├── keyword_props.h    # KEYWORD property metadata key-value table
│   ├── num_format.h       # 15-digit precision display & 16-digit serialization
│   └── override.h         # OVERRIDE method replacement routing table
├── scope/                 # Keyword rules, hooks & access policy engine
│   └── scope.h            # Keyword disable rules, HOOK_BEFORE, HOOK_AFTER, HOOK_OVERRIDE
├── security/              # Capability verification & sandbox boundary
│   └── security.h         # Sandbox permissions, I/O restrictions, capability flags
├── statements/            # Language statement handlers (micro-libraries)
│   ├── core/              # Standard control flow statements
│   │   ├── end.h          # END statement
│   │   ├── gosub.h        # GOSUB / RETURN statements
│   │   ├── goto.h         # GOTO statement
│   │   ├── if.h           # IF / THEN / ELSE / ELSEIF statements
│   │   ├── input.h        # INPUT statement
│   │   ├── on_goto.h      # ON ... GOTO / ON ... GOSUB statements
│   │   ├── print.h        # PRINT / PRINT USING statements
│   │   ├── rem.h          # REM / ' statement
│   │   ├── stop.h         # STOP statement
│   │   └── swap.h         # SWAP statement
│   ├── dialect/           # Metaprogramming & dialect customization
│   │   ├── alias.h        # ALIAS keyword & operator statement
│   │   ├── keyword.h      # KEYWORD SET / GET / LIST statement
│   │   ├── module_stmt.h  # MODULE / MODULE END / MODULE IMPORT statement
│   │   ├── override.h     # OVERRIDE WITH GOSUB / WITH SUB statement
│   │   ├── remove.h       # REMOVE keyword statement
│   │   ├── scope.h        # SCOPE DISABLE / HOOK / PRIVATE statement
│   │   └── selftest.h     # SELFTEST diagnostic statement
│   ├── event/             # Asynchronous event handling
│   │   ├── on_com.h       # ON COM GOSUB statement
│   │   ├── on_error.h     # ON ERROR GOTO / RESUME NEXT statement
│   │   ├── on_key.h       # ON KEY GOSUB statement
│   │   ├── on_timer.h     # ON TIMER GOSUB statement
│   │   ├── resume.h       # RESUME statement
│   │   └── try.h          # TRY / CATCH / FINALLY statement
│   ├── extended/          # Extended BASIC++ features
│   │   └── bitmux.h       # BITMUX bitwise multiplexer statement
│   ├── filesystem/        # File & directory operations
│   │   ├── bload.h        # BLOAD / BSAVE statements
│   │   ├── close.h        # CLOSE statement
│   │   ├── field.h        # FIELD statement
│   │   ├── files.h        # FILES statement
│   │   ├── get.h          # GET # statement
│   │   ├── input_file.h   # INPUT # statement
│   │   ├── kill.h         # KILL statement
│   │   ├── lock.h         # LOCK / UNLOCK statements
│   │   ├── mkdir.h        # MKDIR / RMDIR / CHDIR statements
│   │   ├── name.h         # NAME statement
│   │   ├── open.h         # OPEN statement
│   │   ├── print_file.h   # PRINT # / PRINT # USING statements
│   │   ├── put.h          # PUT # statement
│   │   ├── seek.h         # SEEK statement
│   │   └── write_file.h   # WRITE # statement
│   ├── graphics/          # SCREEN & drawing commands
│   │   ├── bgi.h          # Borland Graphics Interface canvas emulation
│   │   ├── circle.h       # CIRCLE statement
│   │   ├── cls.h          # CLS statement
│   │   ├── color.h        # COLOR statement
│   │   ├── compat.h       # QuickBASIC / GW-BASIC screen mode compatibility
│   │   ├── draw.h         # DRAW statement macro engine
│   │   ├── line.h         # LINE statement
│   │   ├── paint.h        # PAINT flood fill statement
│   │   ├── palette.h      # PALETTE statement
│   │   ├── preset.h       # PRESET statement
│   │   ├── pset.h         # PSET statement
│   │   ├── screen.h       # SCREEN mode initializer statement
│   │   ├── view.h         # VIEW statement
│   │   └── window.h       # WINDOW viewport statement
│   ├── loops/             # Structured loop statements
│   │   ├── do.h           # DO WHILE / UNTIL statement
│   │   ├── exit_loop.h    # EXIT FOR / EXIT DO / EXIT WHILE statement
│   │   ├── for.h          # FOR ... TO ... STEP statement
│   │   ├── loop.h         # LOOP WHILE / UNTIL statement
│   │   ├── next.h         # NEXT statement
│   │   ├── wend.h         # WEND statement
│   │   └── while.h        # WHILE statement
│   ├── matrices/          # Matrix operations
│   │   ├── mat_input.h    # MAT INPUT statement
│   │   ├── mat_ops.h      # MAT ADD, MAT SUB, MAT MUL, MAT INV, MAT TRN
│   │   ├── mat_print.h    # MAT PRINT statement
│   │   └── mat_read.h     # MAT READ statement
│   ├── oop/               # Object-oriented programming & procedure calls
│   │   ├── call.h         # CALL statement
│   │   ├── class.h        # CLASS / END CLASS statement
│   │   ├── def.h          # DEF FN statement
│   │   ├── enum.h         # ENUM / END ENUM statement
│   │   ├── function.h     # FUNCTION / END FUNCTION statement
│   │   ├── shared.h       # SHARED statement
│   │   ├── sub.h          # SUB / END SUB statement
│   │   ├── type.h         # TYPE / END TYPE statement
│   │   └── with.h         # WITH / END WITH statement
│   ├── program/           # Program lifecycle & REPL controls
│   │   ├── chain.h        # CHAIN statement
│   │   ├── clear.h        # CLEAR statement
│   │   ├── cont.h         # CONT statement
│   │   ├── list.h         # LIST statement
│   │   ├── load.h         # LOAD statement
│   │   ├── new.h          # NEW statement
│   │   ├── run.h          # RUN statement
│   │   └── save.h         # SAVE statement
│   ├── sound/             # Audio synthesis & tone playback
│   │   ├── beep.h         # BEEP statement
│   │   ├── play.h         # PLAY statement macro engine
│   │   ├── sound.h        # SOUND frequency statement
│   │   └── voice.h        # VOICE synthesizer statement
│   ├── system/            # Host OS & hardware access
│   │   ├── bios.h         # BIOS interrupt call statement
│   │   ├── debug.h        # TRACE / ASSERT statements
│   │   ├── defseg.h       # DEF SEG statement
│   │   ├── joystick.h     # JOYSTICK statement
│   │   ├── mouse.h        # MOUSE statement
│   │   ├── mux.h          # MUX / DEMUX statements
│   │   ├── pen.h          # PEN statement
│   │   ├── poke.h         # POKE statement
│   │   ├── security.h     # SECURE statement
│   │   ├── system.h       # SYSTEM / SHELL statements
│   │   ├── task.h         # TASK SPAWN / TASK JOIN statements
│   │   └── txn.h          # TXN BEGIN / COMMIT / ROLLBACK statements
│   └── variables/         # Variable allocation & assignment
│       ├── array_ext.h    # REDIM PRESERVE statement
│       ├── data.h         # DATA / READ / RESTORE statements
│       ├── def.h          # DEFINT, DEFSNG, DEFDBL, DEFSTR statements
│       ├── dim.h          # DIM statement
│       ├── let.h          # LET statement
│       ├── lset.h         # LSET / RSET statements
│       ├── mid_stmt.h     # MID$ assignment statement
│       ├── mux.h          # STREAM.MUX statement
│       ├── option.h       # OPTION BASE / OPTION EXPLICIT statements
│       └── randomize.h    # RANDOMIZE statement
├── stmt/                  # Statement registration registry
│   └── stmt.h             # BppStmtRegistry & handler dispatcher
├── types/                 # Engine primitive data types & configuration
│   ├── config.h           # BppConfig limits, RAM allocations (640MB/384MB/64MB)
│   ├── errors.h           # BppError codes (1..100) & error strings
│   ├── types.h            # BValue tagged union (VAL_NIL, VAL_NUMBER, VAL_STRING)
│   └── version.h          # Engine version macros (v6.5.2 "Phoenix")
└── vm/                    # Virtual machine execution loop
    ├── host.h             # Host binding & console input/output hooks
    └── vm.h               # BppVM context struct, call stack, variable frame stack
```

---

## 2. Engine Source Directory (`engine/src/`)

```
engine/src/
├── bios/                  # BIOS emulation implementation
│   ├── bios.c             # Ports 0x00-0xFF, interrupt 0x10/0x16/0x21 dispatch
│   └── ivt.c              # 256-vector interrupt vector table array
├── bootstrap/             # Build profile entry points
│   ├── common/            # Shared boot initialization
│   │   └── common.c       # 9-phase VM boot sequence controller
│   ├── desktop/           # Standard edition REPL (`desktop.c`)
│   ├── embedded/          # Bare-metal microcontroller stub (`embedded.c`)
│   ├── headless/          # Server daemons (`headless.c`)
│   ├── iot/               # Lite edition REPL (`iot.c`)
│   ├── mobile/            # iOS / Android bridge (`mobile.c`)
│   └── server/            # Non-interactive script runner (`server.c`)
├── core/                  # Core data structures
│   ├── dialect.c          # Dialect configuration initializer
│   ├── memory.c           # Memory pool allocator & zero-init memory helpers
│   └── string.c           # Immutable ref-counted string engine
├── debug/                 # Diagnostic implementation
│   └── debug.c            # SELFTEST diagnostic suite & assertion verifiers
├── device/                # Virtual devices
│   ├── vcon.c             # CON: screen matrix, cursor, ANSI color rendering
│   ├── vdev.c             # Virtual device registration & multiplexed I/O
│   └── vfile.c            # File table (1..255) management & I/O stream routing
├── editor/                # TUI Editor implementation
│   ├── edit_buffer.c      # Multi-buffer text editing core
│   └── editor.c            # Full-screen TUI editor UI & key dispatch
├── eval/                  # Expression evaluation core
│   ├── dispatch.c         # Function dispatch table & argument validator
│   ├── eval.c             # AST evaluator, binary operators, type conversions
│   └── functions/         # Built-in function implementations
│       ├── array_funcs.c  # LBOUND, UBOUND evaluators
│       ├── bit_funcs.c    # Bitwise shift/rotate evaluators
│       ├── date_funcs.c   # Date/time evaluators
│       ├── dev_funcs.c    # Hardware & key buffer evaluators
│       ├── math_funcs.c   # Trigonometric & transcendental evaluators
│       ├── str_funcs.c    # String manipulation evaluators
│       └── sys_funcs.c    # System introspection evaluators
├── lexer/                 # Lexical analyzer
│   └── lexer.c            # Source tokenizer & `k_keywords[]` dictionary table
├── memory/                # Segmented memory implementation
│   └── vmem.c             # DEF SEG buffer allocation & far-pointer math
├── module/                # Module subsystem
│   └── module.c           # Module scope registry & namespace stack
├── parser/                # Ephemeral AST parser
│   └── parser.c            # Ephemeral parser, precedence tree generator
├── runtime/               # Dynamic features implementation
│   ├── keyword_props.c    # KEYWORD property key-value hash map
│   ├── num_format.c       # 15-digit precision & 16-digit serialization formatting
│   └── override.c         # OVERRIDE method routing table
├── scope/                 # Scope rules implementation
│   └── scope.c            # Keyword disable rules & BEFORE/AFTER/OVERRIDE hook runner
├── security/              # Security sandbox implementation
│   └── security.c         # Permission validation & file path sandbox checker
├── statements/            # Statement handler implementations (micro-libraries)
│   ├── core/              # core control statements (`end.c`, `gosub.c`, `goto.c`, `if.c`, `input.c`, `print.c`, `rem.c`, `stop.c`, `swap.c`)
│   ├── dialect/           # dialect customization (`alias.c`, `keyword.c`, `module_stmt.c`, `override.c`, `remove.c`, `scope.c`, `selftest.c`)
│   ├── event/             # event handlers (`on_com.c`, `on_error.c`, `on_key.c`, `on_timer.c`, `resume.c`, `try.c`)
│   ├── extended/          # bitwise statement (`stmt_bitmux.c`)
│   ├── filesystem/        # file statements (`close.c`, `field.c`, `files.c`, `get.c`, `input_file.c`, `kill.c`, `lock.c`, `mkdir.c`, `name.c`, `open.c`, `print_file.c`, `put.c`, `seek.c`, `write_file.c`)
│   ├── graphics/          # drawing statements (`circle.c`, `cls.c`, `color.c`, `draw.c`, `line.c`, `paint.c`, `palette.c`, `pset.c`, `screen.c`, `view.c`, `window.c`)
│   ├── loops/             # loop statements (`do.c`, `exit_loop.c`, `for.c`, `loop.c`, `next.c`, `wend.c`, `while.c`)
│   ├── matrices/          # matrix statements (`mat_input.c`, `mat_ops.c`, `mat_print.c`, `mat_read.c`)
│   ├── oop/               # procedure statements (`call.c`, `class.c`, `def.c`, `enum.c`, `function.c`, `module.c`, `shared.c`, `sub.c`, `type.c`, `with.c`)
│   ├── program/           # REPL statements (`chain.c`, `clear.c`, `cont.c`, `list.c`, `load.c`, `new.c`, `run.c`, `save.c`)
│   ├── sound/             # sound statements (`beep.c`, `play.c`, `sound.c`, `voice.c`)
│   ├── system/            # system statements (`bios.c`, `debug.c`, `defseg.c`, `joystick.c`, `mouse.c`, `mux.c`, `pen.c`, `poke.c`, `security.c`, `system.c`, `task.c`, `txn.c`)
│   └── variables/         # variable statements (`data.c`, `dim.c`, `let.c`, `lset.c`, `mid_stmt.c`, `mux.c`, `option.c`, `randomize.c`)
├── tools/                 # Standalone tool utilities
│   ├── bppc.c             # BASIC-to-C17 transpiler & compiler frontend
│   └── detok.c            # Legacy GW-BASIC detokenizer utility
└── vm/                    # Virtual machine engine
    ├── exec.c             # Statement execution dispatcher & HOOK runner
    ├── host.c             # OS console/input binding implementation
    └── vm.c               # VM state creation, memory allocation, line lookup
```
