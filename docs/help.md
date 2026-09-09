<!--
Title:        help
Tier:         4
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot, bppc, detok, trans)
Authority:    engine/
Generated:    no
Status:       Active
-->

# BASIC++ v6.5.2 Master Documentation & Help Navigation Index

## 1. The Interactive Help System

BASIC++ provides a fully integrated, zero-dependency interactive documentation system built directly into the language runtime. Developers can query documentation at the REPL prompt or inspect the filesystem trees.

### In-Engine Interactive Commands
- `HELP`: Displays general help usage and active keyword categories.
- `HELP <keyword>`: Queries reference documentation and syntax for a specific keyword, function, or statement (e.g. `HELP PRINT`, `HELP MID$`).
- `CATALOG`: Displays the categorized tabular inventory of all active language vocabulary.
- `CATALOG <category>`: Filters vocabulary by functional category (e.g. `CATALOG "Graphics"`).
- `CATEGORY`: Lists all active functional classification domains.
- `CATEGORY <keyword>`: Identifies the specific domain for a given keyword.

## 2. Five-Tier Documentation Taxonomy

All documentation across the repository is strictly structured into five architectural tiers:

### Tier 0: Language Standard Specification
The authoritative syntax and semantic standard for BASIC++ v6.5.2.
- `docs/standard.md` / `help/standard.TXT`: ISO/ECMA-aligned core specification.

### Tier 1: System-Wide Guides
Comprehensive manuals defining system capabilities, operational procedures, and runtime architecture.
- `docs/Users_Guide.md` / `help/Users_Guide.TXT`: Executable options, REPL commands, CLI flags.
- `docs/Programmers_Guide.md` / `help/Programmers_Guide.TXT`: Language syntax, expressions, data types.
- `docs/Developer_Guide.md` / `help/Developer_Guide.TXT`: Engine architecture, C17 coding standards, CMake builds.
- `docs/Extension_Guide.md` / `help/Extension_Guide.TXT`: Native C17 plugins, shared libraries, dynamic modules.
- `docs/Options_Reference.md` / `help/Options_Reference.TXT`: `OPTION` directives (`BASE`, `EXPLICIT`, etc.).
- `docs/Library_System.md` / `help/Library_System.TXT`: 12-library modular architecture and layering.

### Tier 2: Subsystem Specifications
Technical specifications describing individual runtime subsystems and hardware targets.
- `docs/Systems_Programming_Manual.md` / `help/Systems_Programming_Manual.TXT`: Bare-metal systems programming, memory banking, and hardware access.
- `docs/Freestanding_Kernel_Architecture.md` / `help/Freestanding_Kernel_Architecture.TXT`: ISO C17 freestanding kernel.
- `docs/BGI_Graphics_And_SDL2_Decomposition.md` / `help/BGI_Graphics_And_SDL2_Decomposition.TXT`: BGI graphics engine and rasterizer.
- `docs/Virtual_Network.md` / `help/Virtual_Network.TXT`: Sockets, Gemini, Gopher, TNFS, and FujiNet.
- `docs/Compiler_Transpiler_Tool_Targets.md` / `help/Compiler_Transpiler_Tool_Targets.TXT`: `bppc` compiler and `trans` transpiler.
- `docs/FreeDOS_Build.md` / `help/FreeDOS_Build.TXT`: 16-bit real mode target and Open Watcom build.

### Tier 3: Per-Item Reference Pages (One File Per Item)
Exhaustive reference pages for every statement, function, and variable.
- Statements: `docs/keywords/statements/<NAME>.md` / `help/keywords/statements/<NAME>.TXT` (371 files).
- Builtin Functions: `docs/keywords/functions/<NAME>.md` / `help/keywords/functions/<NAME>.TXT` (316 files).
- System Variables: `docs/keywords/variables/<NAME>.md` / `help/keywords/variables/<NAME>.TXT` (155 files).
- Subsystem APIs: `docs/api/`, `docs/io/`, `docs/Introspection/`.

### Tier 4: Quick-Reference Cards & Master Catalogs
Fast lookup sheets and master indexes.
- `docs/Quick_Reference.md` / `help/Quick_Reference.TXT`: Syntax cheat-sheet and operator quick reference.
- `docs/catalog.md` / `help/catalog.TXT`: Master vocabulary inventory table.
- `docs/Keyword.md` / `help/Keyword.TXT`: Keyword enumeration catalog and ID mapping.
- `docs/Engine_Features.md` / `help/Engine_Features.TXT`: Active engine subsystem inventory.
- `docs/help.md` / `help/help.TXT`: Root documentation navigation index.

## 3. 100% Mirror Parity Guarantee

Every Markdown document in `docs/*.md` is mirrored by an identical plaintext document in `help/*.TXT`. Help files are strictly formatted for terminal display:
- Pure 7-bit ASCII encoding (0 non-ASCII bytes).
- Standard CRLF (`\r\n`) line endings.
- Strictly wrapped at <= 78 columns.
- Zero markdown backticks, raw HTML tags, or formatting pipes.
