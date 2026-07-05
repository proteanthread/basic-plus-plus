# BASIC++ Project Rules

- Adhere to strict C17 - ISO/IEC 9899:2018 portability, modularity, maintainability, and readability guidelines; Always update comments, and verify that comments are current.
- always remove AI-generated fingerprints and breadcrumbs.
- We are no longer bound by the Palo Alto Tiny BASIC or FreeDOS constraints. Our targeted systems are solely Windows 11, Linux, and BSD all of which are 64bit systems (optionally embedded systems such as Arduino, Raspberry Pi, FreeRTOS, ESP32, BareMetal, etc.)
- Full compatibility with GWBASIC, QBASIC, ECMA-116 Standard BASIC / Full BASIC (but not held to the constraints of GWBASIC, QBASIC, and ECMA-116 Standard BASIC / Full BASIC)
- Whenever running any kinds of tests on BASIC++, always pass the `--log` and `--out` and '--debug' command line switches and if errors are found rerun the same tests using the '--dry-run' command line switch
- After tests have been completed and successfully passed, clean up any cruft, lint, leftover files, logs, test files, scripts, etc., from the project. (including that which may be left over from incomplete, or inconclusive, or tests that errored out)
- After tests have been completed and successfully passed, the following files must always be updated: `.\source\SOURCE_TREE.txt` and `.\CHANGELOG.txt` and `.\README.md`.
- Whenever introducing a new keyword, the online built-in `HELP` and `CATALOG` must always be updated, and `.\help\Quick_Reference.txt` (plaintext) and `.\docs\Quick_Reference.md` (markdown) must always be updated as well.
- Whenever we map out a new plan, make sure you always ask clarifying questions.
- as much as possible, the majority of the sub-systems code can be included in or ported to other projects; always document the portable sub-systems.
- all files and folders should have more meaningful names.
- our virtual abstraction layer includes: virtual consoles, virtual terminals, virtual graphics, virtual memory, virtual keywords, virtual devices, virtual networks, virtual hardware, virtual filesystems
- make sure our version info in ".\source\config.h" and binaries are synced with the CHANGELOG.txt
- our external modular system includes our: external modules, external plugins, external libraries, external keywords, external functions and other external features
- Keep updating our mock BIOS (based on the IBM PC BIOS) as we progress through our project; (We should end up with a fully configurable, switchable IBM PC, PCjr, XT, or AT mock BIOS that is fully functional); the IBM PC ( MSDOS / PCDOS ) must assume to be default of no other is specified when running BASIC++ Lite, BASIC++ Standard, BASIC++ SDL.
- optimize first for memory (lowest amount possible) then for speed (fast as possible)
- always update the files in ".\games" and ".\samples" and ".\tests" as we progress through our project. These become stale quickly. May have to add, change, delete, update, or modify some of these files.
- always perform regression (failure) checking and consider regressions as failures
- always remove any AI-generated fingerprints, breadcrumbs, or evidence.
- always update "CATALOG.txt" and "HELP.txt" in the root directory as we are defining keywords, update any keywords that have been changed (many now have hybrid functions)
- always update all of our documentation, including built-in, online help as well as all files in ".\docs" (markdown) and ".\help" (plaintext) as these have quickly grown stale as we have been progressing through our project.  May have to add, change, delete, update, or modify some of these files.
- In SDL builds (e.g. basicpp.exe / baspp), the boot banner must replace 'Standard' with 'SDL' (so it reads 'BASIC++ SDL'), while console / text-only builds (e.g. basicpp-console.exe / baspp-console) must continue to say 'BASIC++ Standard'. References to BASIC++ Standard cover both configurations, whereas BASIC++ Lite is referenced explicitly.
- keep BASIC++ Lite - `blite.exe` (Windows 11) and `blite` (Linux) - as light as possible, do not add any more features unless explicitly told to do so.
- all sub-systems must be decoupled from the core: past, present, and future sub-systems (so that the can be integrated into other projects)
- The following files are never to be deleted during cleanups:
  - `tinybasic.exe` (Windows 11), `tinybasic.c` (source), `tinybasic` (Linux)
  - `level1.exe` (Windows 11), `level1.c` (source), `level1` (Linux)
  - `apple2.exe` (Windows 11), `apple2.c` (source), `apple2` (Linux)
  - `basicpp.exe` (always recompile for Windows 11), `basicpp-console.exe` (always recompile for Windows 11)
  - `baspp` (always recompile for Linux), `baspp-console` (always recompile for Linux)
  - `blite.exe` (always recompile for Windows 11), `blite` (always recompile for Linux)

# Prioritized Values

Always prioritize:
- Must always remain "just an interpreter" no matter what, even with the inclusion of an external compiler and an external detokenizer as well as other dialects.
- Portability
- Readability
- Maintainability
- Modularity
- GWBASIC / QBASIC compatibility
- Fully ECMA-116 (Full BASIC / Standard BASIC) compliant
- Deterministic behavior
- regression checking
- Future expandability (futureproofing)

# Strict C17 Portability & Type-Safety Guidelines

To prevent compiler warning divergence and 64-bit portability bugs:
- **64-bit Pointer Safety**: NEVER cast pointers directly to/from `long` or other integer types of potentially different sizes on 64-bit platforms (C4311/C4312). Always use standard C17 `<stdint.h>` types `intptr_t` or `uintptr_t` for conversions between pointer and integer representations.
- **Unary Unsigned Operators**: Avoid applying the unary minus operator to unsigned variables or values (like `(-rot) & 31` on `uint32_t rot`), which triggers warning C4146. Instead, use safe modular subtraction, e.g., `((32 - rot) & 31)`.
- **OS API Isolation**: NEVER include platform-specific OS APIs or headers (such as `<io.h>`, `_findfirst`/`_findnext` on Windows, or `<dirent.h>`, `<unistd.h>` on POSIX) directly inside standard interpreter parser or statement core files. Enforce platform isolation by housing them strictly inside platform abstraction modules (such as `platform.c`) and exposing clean platform-independent APIs.

# Regression Prevention & Verification Guidelines

To prevent regression failures across features, keyword updates, or interpreter adjustments, follow these constraints:
- **Redirection of Console Output:** NEVER call raw standard library output functions (`printf`, `putchar`, `fflush`) directly inside statement or command parsers, dialect profiles, built-ins, or help files. You MUST include `"console.h"` to map output statements to the active GUI console buffer if running in SDL mode.
- **Symmetric Command Line Parity:** All command-line arguments (such as `--help`, `--log`, `--out`, `--cleanup`, etc.) must be implemented symmetrically between `basicpp-console.exe` and `basicpp.exe` to prevent CLI execution divergence. Both builds must remain in sync at all times; the only difference is when SDL is implemented: the GUI version implements SDL at boot, whereas the console/text-only version implements SDL only when called (such as on a SCREEN or graphics command).
- **Internal SELFTEST Validation:** Every code modification must successfully pass the built-in `SELFTEST` command (run as `basicpp-console.exe --log -c "SELFTEST"` and `basicpp.exe --log -c "SELFTEST"`) verifying the integrity of the Lexer, Value system, String pool, Function registry, Memory system, Dialect config, Parser precedence, Control flow, VFS, and SDL subsystems.
- **Exhaustive Automated Comparisons:** Before finalizing any task, run the Python comparative validation suite (`python -u scratch/run_comprehensive_tests.py`) in unbuffered mode (`-u`) to verify that Windows Console, Windows SDL, and Linux Console builds behave identically across all test directories, ensuring real-time log streaming without buffer delays.
- **Unbuffered Python Execution:** Always run python scripts using the unbuffered flag (`python -u`) when executing them as background tasks or redirection pipelines to ensure logs and outputs are visible immediately.

# Code Generation Guidelines

When generating code:
- Generate fully commented ANSI/ISO C17 code.
- Explain every subsystem.
- Explain portability concerns.
- Explain memory management.
- Explain parser behavior.
- Explain lexer behavior.
- Explain runtime behavior.
- Explain future expansion points.
- Explain what can be changed.
- Explain what cannot be changed.
- Explain what to expect.
- Explain what to do if something breaks or goes wrong.

# Architecture Documentation Guidelines

When generating architecture documentation:
- Break down by subsystem and module.
- Provide function-level explanations.
- Provide data structure explanations.
- Provide memory-flow explanations.
