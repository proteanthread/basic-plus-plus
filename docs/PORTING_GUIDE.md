================================================================================
                    PORTING GUIDE FOR BASIC++
================================================================================

This plain-text porting guide accompanies BUILD_YOUR_OWN.md and offers a 
highly detailed, definitive overview and step-by-step checklist for embedding 
BASIC++ on new architectures, specialized OS environments, or highly 
constrained embedded microcontrollers (Arduino, ESP32, BareMetal).

Because BASIC++ adheres strictly to the C17 ISO standard, porting primarily 
revolves around defining memory boundaries, optimizing config macros, and 
implementing the Platform Abstraction Layer (PAL), rather than rewriting 
core language logic.

--------------------------------------------------------------------------------
1. THE MANDATORY CORE
--------------------------------------------------------------------------------
To successfully compile and execute a "Hello World" in BASIC++, your toolchain 
MUST compile the following minimal engine components:

* core/main.c          (Entry point and CLI handling)
* core/memory.c        (Deterministic memory pool management)
* core/errors.c        (Error handling engine)
* core/error_registry.c(Error definitions and strings)
* core/value.c         (Type system for strings, numbers, floats)
* core/stringpool.c    (Memory-safe string allocation)
* core/platform.c      (Platform abstractions - SEE SECTION 5)
* core/console.c       (Terminal standard I/O abstraction)
* core/stdlib_core.c   (Safe wrappers for C standard library functions)
* core/lite_stubs.c    (Satisfies linker for omitted parser hooks)
* lexer/lexer.c        (Core tokenization)
* lexer/keyword_props.c(Keyword definitions)
* parser/parser.c      (Core grammar and AST construction)
* parser/parser_expr.c (Expression evaluation)
* runtime/runtime.c    (Execution engine state and symbol tables)
* runtime/exec.c       (Statement execution)
* runtime/ast_interpreter.c (AST walking engine)

--------------------------------------------------------------------------------
2. OPTIONAL SUB-SYSTEMS (THE MODULAR APPROACH)
--------------------------------------------------------------------------------
BASIC++ contains numerous subsystems that provide advanced functionality. These 
can be entirely excluded from the build to save space (up to 2MB) and reduce 
heavy OS dependencies. When excluding these, you MUST provide stub functions 
in `lite_stubs.c` (see Section 4).

* Graphics & Display (source/graphics/ and source/display/): 
  Requires SDL2 in standard builds. Exclude to run headless. Saves ~1MB RAM.
* Sound (source/sound/): 
  Requires audio APIs. Exclude on systems without speakers.
* File I/O (source/io/parser_fileio.c, source/io/fileio.c, source/io/vfs.c): 
  Provides OPEN, CLOSE, GET, PUT. Exclude on systems lacking a filesystem (like 
  Arduino Uno) or implement a custom Virtual File System (VFS). Saves ~100KB.
* Networking (source/io/parser_net.c, source/io/builtins_net.c): 
  Provides TCP/IP sockets. Exclude on systems without network stacks.
* Self-Testing (source/debug.c, source/selftests_all.c): 
  Exclude in production binaries. The 412-keyword selftest consumes massive 
  string literal memory. Always omit for microcontrollers.

--------------------------------------------------------------------------------
3. CONFIGURATION FLAGS AND MEMORY TUNING
--------------------------------------------------------------------------------
Define these macros in your build environment (CFLAGS) to bypass subsystems 
globally without editing source files:

* BASIC_LITE_BUILD: 
  Disables heavy sub-system initialization in boot.c, drastically reducing 
* NO_SDL2: 
  Removes GUI hooks and directs PRINT to terminal standard output (stdin/stdout).
  Severing this dependency makes the build purely POSIX/CLI driven.
* INPUT_CONSOLE:
  Forces standard library console input instead of event-driven SDL input.
* BASIC_NO_NETWORK:
  Disables the internal VFS networking socket wrappers (removes `<winsock2.h>`).

*** EXTREME MEMORY TUNING FOR ARDUINO / ESP32 ***
Edit `source/config.h` to physically restrict the runtime engine:
1. MAX_VARS: Change from 4096 to 64 (Restricts symbol table size).
2. STR_POOL_SIZE: Change from 1MB to 4096 (4KB string garbage collector limit).
3. MAX_CALL_STACK: Change from 2048 to 64 (Limits GOSUB/CALL depth).
4. AST_ARENA_SIZE: Change from 8MB to 16384 (Limits the size of loaded scripts).

--------------------------------------------------------------------------------
4. WRITING CUSTOM STUBS (SATISFYING THE LINKER)
--------------------------------------------------------------------------------
When removing sub-systems like Graphics or File I/O, the parser core might 
still contain hardcoded tokens for keywords like PLOT or OPEN. If your target 
architecture does not link the graphics parser, the linker will fail.

You MUST provide a "stub" file that satisfies the linker and throws a runtime 
syntax error when the keyword is invoked by the user. Use `core/lite_stubs.c`!

Example (lite_stubs.c):
  void pi_parse_plot(Lexer *lex, RuntimeState *rt, int line_num) {
      error_raise(ERR_HOW, line_num); // Throws "How?" Syntax Error
  }

--------------------------------------------------------------------------------
5. THE PLATFORM ABSTRACTION LAYER (PAL)
--------------------------------------------------------------------------------
To make BASIC++ completely OS-agnostic, the core engine NEVER calls OS-specific 
APIs (like Windows' Sleep() or POSIX's usleep()) directly. Instead, it calls 
platform-neutral wrappers in `core/platform.c`.

When porting to a custom OS or bare-metal environment, you must edit 
`core/platform.c` and implement the following handlers:

* platform_sleep(ms):
  Implement a blocking delay. On Arduino, this maps directly to `delay(ms)`.
* platform_get_ticks():
  Return milliseconds since boot. On Arduino, this is `millis()`.
* platform_get_env():
  Return environment variables. If your system has none, return NULL.
* platform_console_init() / platform_console_cleanup():
  Initialize UART, Serial ports, or VGA memory for text output.
* memory pool hooks (CRITICAL for Embedded):
  By default, `core/memory.c` uses `malloc` to request its large memory banks. 
  On microcontrollers, you MUST statically allocate a massive array 
  (e.g., `uint8_t static_heap[32768]`) and rewrite `memory.c` initialization 
  to point the memory pool to that static array instead of dynamic allocation 
  to avoid heap fragmentation entirely.

--------------------------------------------------------------------------------
6. PORTING CHECKLIST
--------------------------------------------------------------------------------
[ ] Set up a custom Makefile or build script linking only the Core files.
[ ] Define -DBPP_LITE_BUILD, -DNO_SDL2, and -DBPP_NO_NETWORK in compiler flags.
[ ] Reduce STR_POOL_SIZE and AST_ARENA_SIZE in `config.h` to fit your SRAM.
[ ] Ensure `core/lite_stubs.c` is included in the build to catch omitted modules.
[ ] Attempt to compile. For every "Unresolved External Symbol" error emitted 
    by the linker, add a dummy function to `lite_stubs.c` throwing `ERR_HOW`.
[ ] Map `platform_sleep` and `platform_get_ticks` to your hardware timers.
[ ] If compiling for a microcontroller, rewrite `memory.c` to use a static array.
[ ] Run the compiled binary!
================================================================================
