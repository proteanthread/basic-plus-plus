# BASIC++ PERMANENT PROJECT RULES (v6.x.x)

## 1. Decisions That Must Not Be Reversed
* **Strict C17 Standard**: All code must be strict C17 (not C89, not C99, not C11, not C23). Compiler-specific pragmas are forbidden in the core.
* **Dynamic Memory Scaling**: Memory scaling is dynamic; the interpreter will automatically utilize available RAM on modern hosts.
* **Ephemeral Tokenization**: Source code is canonical and stored as plain text. Tokens are parsed ephemerally and discarded immediately after statement/expression execution.
* **Preserved Educational Standalone Modules**: Never modify or delete the standalone files under the `classics/` (or `standalone/`) directory:
  - `1964.c`
  - `apple2.c`
  - `level1.c`
  - `tinybasic.c`

## 2. Mistakes That Must Not Be Repeated / Execution Constraints
* **Non-Recursive Execution**: DO NOT use recursive C functions for parsing or execution (`eval()` calling `eval()`). All execution MUST use explicitly allocated, interpreter-managed virtual stacks on the heap.
* **OS API Isolation & Agnostic Logic**: Platform-specific OS APIs (e.g., `<io.h>`, `<dirent.h>`, `<unistd.h>`, `<windows.h>`, `<dlfcn.h>`) must reside exclusively in platform abstraction modules (such as `src/platform/platform.c`). Core parsers, execution systems, procedures, and statements must remain completely platform-independent and OS agnostic.
* **Agnostic VM Loop**: DO NOT embed dialect-specific `if/else` checks inside the VM execution loop. Dialects define syntax at the parser level; the VM executes dialect-agnostic bytecode.
* **Auditable & Safe Plugin System**: Modules are loaded through a pipeline: Validation -> Capability Verification -> Sandbox Allocation -> Registration -> Activation. Modules must NEVER directly modify VM instructions, execute host code bypasses, or corrupt internal stacks.

## 3. Architecture Principles & Modularity
* **Layered Design**: Lexer -> Parser -> AST -> Bytecode -> VM.
* **Unified Expression Language**: All expressions, regardless of dialect, use the same AST evaluation logic.
* **Domain Dispatch**: `DEF FN`, `DEF STR`, `DEF GFX` are routed via the Domain Dispatch Registry.
* **Subsystem Architecture Explanations**: When introducing or implementing a subsystem/module, prioritize portability, readability, maintainability, modularity, deterministic behavior, and future expandability.
* **Modularity and Portability**: Subsystems, statement libraries, and modules must be modular, highly portable, and self-contained so that they can be easily reused in other C17 projects without carrying internal VM engine dependencies.
* **Refactorability-First Design**: All modules must be structured so that users and developers can safely rename, move, split, or reorganize them. The following conventions apply:
  - **Explicit Dependency Surfaces**: Every `.c` file must `#include` only the headers it directly uses. Never rely on transitive includes (headers pulled in by other headers). This ensures that moving a file to a different directory or project immediately reveals all its true dependencies.
  - **No Cross-Module Internal Access**: Modules must only access other modules through their public header APIs (`include/bpp_*.h`). Never `#include` another module's private/internal `.h` files from `src/<module>/` or reference its `static` symbols via extern hacks.
  - **One Responsibility Per File**: Each `.c` file should have a single, well-defined responsibility. If a file grows beyond ~1024 lines of logic (excluding the documentation header and comments), evaluate whether it should be split into focused sub-files within the same module directory.
  - **Consistent Naming Conventions**: Public API functions must be prefixed with their module namespace (e.g., `vm_`, `lexer_`, `vdev_`, `mem_`). Internal/static helper functions within a `.c` file do not require a prefix but must never collide with public names. Struct and enum type names follow the same module prefix pattern.
  - **No Hidden Initialization Order Dependencies**: Module initialization functions (e.g., `*_init()`, `*_setup()`) must be callable in any order unless an explicit dependency is documented in both the caller's and callee's file headers. Circular initialization dependencies are strictly forbidden.
  - **Header Self-Sufficiency**: Every `.h` file must compile independently when included alone (i.e., it must include its own prerequisite headers). Test this mentally: `#include "bpp_foo.h"` in an empty `.c` file should compile without errors. The most commonly missing direct includes are `<stdbool.h>` (for `bool`), `<stddef.h>` (for `size_t`, `NULL`), and `<stdint.h>` (for `int32_t`, `uint8_t`, etc.). These are frequently pulled in transitively on MSVC but NOT on GCC, causing cross-platform build failures.

## 4. Specific Rules and Safety Invariants (v6.x.x Bug Hunts)
* **Standard Memory Footprints**:
  - `blite` (REPL) default allocation size must be 384 MB (`402653184L` bytes).
  - `bscript` (Script Runner) default allocation size must be 64 MB (`67108864L` bytes).
* **Multiple Postfix Conditionals**:
  - Trailing unnested modifiers (e.g. `PRINT "ok" IF A = 1 IF B = 2`) evaluate multiple conditions right-to-left.
* **MSVC Compile Options**:
  - Size optimization flags `/O1` and `/GL` must be gated using `$<NOT:$<CONFIG:Debug>>` generator expressions in `CMakeLists.txt` to avoid conflict with MSVC's debug run-time check flags (`/RTC1`).
* **64-bit Pointer Safety**: Never cast pointers directly to/from standard integer types. Use `<stdint.h>` types `intptr_t` or `uintptr_t`.
* **Zero-Initialization by Default**: All dynamic arrays, collection buffers, hash table buckets, and structures containing pointers/references MUST be zero-initialized immediately upon allocation using `calloc` or an immediate `memset`.
* **Iteration Bounds Verification**: Loops iterating over collections must strictly bound their iterations to the initialized item count (`count` or `size`), NOT the raw physical allocation size (`capacity`), unless they are designed to scan/re-initialize the raw storage and have guaranteed NULL sentinels.
* **Bounded Token String Operations**: Never use unbounded string functions (`strchr`, `strstr`, `strcmp`, `strcpy`, `strlen`) directly on `tok.start` or raw substrings that are not null-terminated. Always use bounded variants (`memchr`, `memcmp`, or copy to a local null-terminated buffer) to prevent out-of-bounds reads.
* **Safe Realloc Pattern**: Unsafe `realloc` assignments (`*p_buf = realloc(*p_buf, size)`) are strictly forbidden. Always use a checked/safe growth helper (like `safe_buf_grow`) that preserves the original pointer on failure.
* **Console Redirection**: Never call raw standard library output functions (`printf`, `putchar`, `fflush`) directly inside statement/command parsers or helpers. Route all console operations through virtual devices defined in `src/device/vdev.c`.
* **Thread-Safe Localtime**: Standard thread-un-safe `localtime` is forbidden. Always use thread-safe wrappers like `localtime_s` (Windows) / `localtime_r` (POSIX) or a unified platform abstraction (`platform_localtime`).
* **Agent Interaction Rules**:
  - **No Mock Notifications**: Do not simulate system/tool notifications in thoughts or responses. Allow the environment to handle asynchronous tasks and notify the agent naturally.
* **Pure ASCII Console I/O**: All text input and output generated or processed by the engine (such as for console devices) must be pure 7-bit ASCII, not UTF-8 (no multibyte or Unicode code-points in terminal output). Raw 8-bit byte passthroughs are only permitted for binary or random-access file I/O operations.
* **GCC Circular Static Library Resolution**: When micro-libraries are linked PUBLIC into a parent static library and reference symbols from the parent's own source files, GCC's single-pass linker will fail with undefined reference errors (MSVC's multi-pass linker resolves these automatically). For any executable target that links against such a parent library on GCC/Clang, apply `set_target_properties(<target> PROPERTIES LINK_FLAGS "-Wl,--start-group")` in CMakeLists.txt.
* **snprintf Buffer Sizing for GCC**: All `snprintf` destination buffers must be sized to accommodate the theoretical worst-case output length of the format string, not just the expected typical output. GCC's `-Wformat-truncation=` performs conservative worst-case analysis and will emit errors (under `-Werror`) if the buffer could theoretically be too small. When combining two bounded strings (e.g., `"%s.%s"` with two 128-byte inputs), the buffer must be at least `128 + 1 + 128 + 1 = 258` bytes. For ISO 8601 date formatting, use at least 64 bytes to satisfy GCC's analysis of `tm_year` range.
* **POSIX Feature-Test Macros for Micro-Libraries**: When creating new CMake static library targets (micro-libraries) that compile `.c` files using POSIX APIs, you MUST add explicit compile definitions on UNIX: `target_compile_definitions(<target> PRIVATE $<$<BOOL:${UNIX}>: _POSIX_C_SOURCE=200809L _XOPEN_SOURCE=700 _GNU_SOURCE>)`. These macros are NOT inherited from parent targets and their absence causes implicit function declaration errors on GCC for `setenv`, `usleep`, `fileno`, `gethostname`, `realpath`, `strdup`, `strndup`, `popen`, `pclose`, etc.
* **Dual-Platform Build Verification**: After any code or build system change, compilation MUST be verified on BOTH Windows (MSVC) and Linux (GCC) before declaring a fix complete. A change that compiles on one platform but not the other is NOT considered fixed. When possible, run both builds concurrently to minimize turnaround time.

## 5. Testing & Regression Prevention
* **Built-in Verification**: All changes must pass the native `SELFTEST` command (`baspp.exe -c "SELFTEST"` and `baspp -c "SELFTEST"`).
* **Clean Multi-Platform Compilation**: All target binaries (including standard, lite, compile, and trans targets) must compile cleanly with no warnings, no bugs, and no errors across both Windows and Linux environments.
* **v6 Regression Tests Only**: NONE of our regression tests should be held over from v5. We must implement NEW v6 regression tests ONLY to ensure all tests match modern v6 parser and virtual machine semantics.
* **Regression Testing Execution Target**: All regression testing suites must be executed using the standard `baspp` (Linux) and `baspp.exe` (Windows) targets. Other builds (like `blite`, `bscript`) must not be used for running regression tests.
* **Native-Only Testing Mandate**: Never use Python, bash, or other external script wrappers to run, parse, or validate tests. All automation must be written in the BASIC++ language itself and run directly in the interpreter.
* **Progressive Tests**: Keep tests under the `tests/` directory as cumulative, progressive regression suites. They must not be treated as disposable.
* **Category-Level Regression Tests**: Whenever a new keyword, feature, or function is added, modified, or updated, the corresponding category-level regression test file (mapped in `tests/categories.json` and `tests/keywords.json`) MUST be updated to verify both the happy path and failure/boundary cases for that category of keywords. This ensures that a regression breaking an entire category of statements is caught immediately by the integration test suites.
* **Bug Hunts**: Always perform an exhaustive bug hunt at the end of every task before control is returned back to the user. Only when we are 100% free of bugs then can any Walkthrough.md be displayed.

## 6. Introspection, Documentation & Manual Parity
* **C & Header File Documentation Headers**: Every single `.c` and `.h` file under `src/` and `include/` MUST begin with a standardized header block addressing the following:
  - **What it does**: High-level summary of the file's purpose and functionality.
  - **Why it exists**: The architectural problem it solves or context in the boot/execution model.
  - **Why it works this way**: Explanation of design choices (e.g., iterative vs recursive, memory layouts).
  - **What can be changed**: Safe zones for editing, tuning parameters, or adding commands.
  - **What cannot be changed**: Critical invariants, strict boundaries, stack constraints, or performance-sensitive loops.
  - **What to expect**: Expected side-effects, behaviors on error, resource usage, or performance characteristics.
  - **What to do if something breaks**: Debugging steps, diagnostic registers to check, or assertions that might fire.
  - **Assumptions**: What preconditions must be met (e.g., initialized contexts, alignment).
  - **Portability concerns**: 16-bit vs 32/64-bit limits, endianness, OS dependencies.
  - **Future expansions**: How to add new features, statements, or support new systems safely.
  - **External extension hooks**: Guidelines on writing plugins, shared libraries, or custom keywords for this subsystem.
* **BASIC and Bytecode Documentations**: All test/program files (`.bas`, `.bpp`) must start with a comment block containing:
  - **What can be changed**: User-modifiable parameters or code sections.
  - **What cannot be changed**: Required test structures, initial declarations, or exact output assertions.
  - **What to expect**: What output, behavior, or side-effect the script will produce.
  - **What to do if something breaks**: Troubleshooting steps if the test fails or output diverges.
* **Introspection Parity (HELP & CATALOG)**: Whenever a new keyword, statement, function, library, or feature is added or modified in the compiler or virtual machine, you MUST:
  1. Register its help details in the online `HELP` system (either via metadata block registries, function registry descriptions, or built-in help text tables).
  2. Ensure it is represented correctly in the online environment `CATALOG` output.
  3. Update the offline static reference files (`HELP.txt` and `CATALOG.txt`) to maintain documentation parity.
  No keyword or feature is complete unless it is fully queryable via interactive `HELP` and `CATALOG` commands.
* **Documentation & User Guide Parity**: Whenever a new feature, keyword, statement, function, or system is implemented or modified in the compiler or virtual machine, you MUST:
  1. Add or update the corresponding user-facing programmer guide/manual in the `docs/` directory detailing exactly how to use, configure, and program with these new features, including concrete code examples, syntax definitions, and best practices.
  2. Register its interactive help text in the online `HELP` system.
  3. Ensure it appears in the online environment `CATALOG` command outputs.
  4. Update the offline static reference files (`HELP.txt` and `CATALOG.txt`) to maintain documentation parity.
  No feature or keyword is complete unless it is fully documented for the programmer's self-study.
* **Phase-by-Phase Documentation Updates**: During the rebuild phases of v5.0.5 to v6.x.x, update the corresponding documentation files under `docs/` (Markdown) and `help/` (plaintext) manually for each rebuilt section or feature before concluding the phase. Do not wait for automated synchronization scripts.
* **Continuous Documentation Parity**: Update documentation in `docs/` (Markdown) and `help/` (plaintext), including API references, dynamically as changes progress through the project. Documentation and API updates must never be delayed to the end of a milestone.
* **Compiler & VM Bytecode Emitter Synchronization**: Whenever a new language feature, statement, function, variable type, or syntactic rule is added to the standard BASIC++ interpreter, the compiler (`bppc`), transpiler modules, and bytecode emitter MUST be updated to maintain exact semantic and runtime parity. No interpreter feature is complete unless it is also compilable to native binaries and standalone bytecode.
* **Transpiler Synchronization**: Whenever a new language feature, statement, function, variable type, or syntactic rule is added to the standard BASIC++ interpreter, the transpiler tool (`trans`/`trans.exe`) and all its language backend modules (`source/codegen/trans_c17.c`, etc.) MUST be updated to maintain exact semantic parity. No interpreter feature is complete unless it is also transpiled cleanly and correctly to target languages.

## 7. Workspace Hygiene, File Placement & .gitignore
* **Project Notes Protection**: The `project notes/` directory and all files inside it must be ignored by `.gitignore` to prevent uploading to GitHub, but must never be deleted by any cleanup scripts or agent actions. All reference documents, specifications, and layout plans must be kept in this directory.
* **Clean Root Policy**: The project root must remain clean of temporary `.err`, `.log`, and compiler-generated files after a session. Only sanctioned build folders (`build_win/`, `build_linux/`) are allowed.
