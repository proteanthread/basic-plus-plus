<!--
Title:        Historical_Dialect_Feature_Import_Catalogue
Tier:         1
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot, bppc, trans, detok)
Authority:    engine/
Generated:    no, manual architectural catalogue
Status:       current
-->

# BASIC++ Historical Dialect Feature Import Catalogue & Architectural Specification

Specification Level: Stage One Master Architectural Catalogue & Research Register  
Primary Reference: `project notes/BASIC_Dialects_Keyword_Reference.xlsx` (20 Sheets, 432 Combined Entries)

---

## 1. Executive Summary & Headline Metrics

This document codifies the architectural evaluation, classification, and compatibility roadmap for importing historically significant language features across 54+ vintage BASIC dialects, dialect families, and related systems languages into **BASIC++ v6.5.2**.

In strict accordance with the project's inviolable governing principles:
1. **Rule #1 (Vintage Baseline Backward Compatibility)**: Microsoft BASICA / GW-BASIC and QBASIC / Visual Basic for DOS serve as the unalterable compatibility baseline. All imported features must be 100% non-breaking and unreachable from valid vintage source code.
2. **Rule #2 (Strict Freestanding ISO C17 Engine)**: The core engine micro-libraries (`libkernel`, `libengine`, `basicpp_sys.h`) strictly conform to ISO C17 (§4 ¶6). Hosted headers (`<stdio.h>`, `<stdlib.h>`, etc.) and host OS system calls are permanently prohibited in the freestanding engine core.
3. **Mandatory Line-Number Invariant**: The AST evaluator and line-numbered program storage architecture remain intact. Unnumbered legacy programs are integrated via an auto-renumbering preprocessor (`RENUM`), while line-number-free block constructs are deferred to compiler targets (`bppc`).

### Headline Catalogue Metrics

Following the audit of `project notes/BASIC_Dialects_Keyword_Reference.xlsx`, the deduplicated feature space establishes the following authoritative metrics:

| Metric Category | Verified Count | Architectural Implication |
| :--- | :--- | :--- |
| **Total Deduplicated Features Catalogued** | **428** | Coverage across 54+ dialects, Apple III, North Star, and systems languages. |
| **Tier 1: Core-Eligible (Pure Freestanding C17)** | **273** | Algorithmic, mathematical, string, array, and AST primitives with zero OS dependencies. |
| **Tier 2: HAL-Mediated (Virtual Device Bus)** | **150** | Keyed I/O, ISAM, record locks, terminal formatting, timers, graphics, and ports. |
| **Tier 3: Hosted-Only / Formally Rejected** | **5** | Multi-user spooling, timesharing session managers, inter-user mail. |
| **Active in BASIC++ Core** | **115** | Verified active in `engine/src/`, covered by active test suites. |
| **Candidate Feature Stack** | **308** | Implementation candidate features across the multi-wave roadmap. |
| **Line-Number Native** | **389** | 100% compatible with line-numbered AST without modification. |
| **Auto-Renumber Preprocessor Compatible** | **35** | Supported via `RENUM "file.bas", start, step` or `LOAD "file.bas", RENUM`. |
| **Compiler Deferral Register** | **4** | Unnumbered modular scopes (Modula-2/Oberon `MODULE...END`) deferred to `bppc`. |

---

## 2. The 3-Tier Freestanding Classification Architecture

To ensure total compliance with **Rule #2**, every language capability is assigned to exactly one tier:

### Tier 1: Core-Eligible (Pure Freestanding C17, Zero Libc, Zero Host OS)
- **Matrix Algebra**: `MAT READ`, `MAT PRINT`, `MAT INV`, `MAT TRN`, `MAT IDN`, `MAT ZER`, `MAT CON`.
- **String & Substring Slicing**: HP `A$[x,y]`, North Star `A$(x,y)`, DEC `DIM S$ = len`.
- **High-Precision BCD & Banking Math**: Apple III 19-digit BCD arithmetic, NCR Century math.
- **MultiValue Dynamic Arrays**: Pick `EXTRACT`, `REPLACE`, `INSERT`, `DELETE`, attribute/value marks.

### Tier 2: HAL-Mediated (Virtual Device Bus Abstraction)
- **Direct Hardware & Port I/O**: `MEM[...]`, `PORT[...]`, `PEEK[...]`, `POKE`, `INP`, `OUT` with volatile memory barriers.
- **Keyed File Systems**: ISAM B-tree indexing (`CREATEINDEX`, `DELETEINDEX`, `SETINDEX`, `SEEKEQ`, `SEEKGE`).
- **Terminal Formatting**: VT52/VT100 cursor control, color palettes, print zones.
- **Microsecond Timers**: `TICKS_US`, high-resolution interval timers, event traps.

### Tier 3: Hosted-Only / Formally Rejected
- **Supervisor Calls**: OS-specific session managers, privileged timeshare mail drops, host process dispatchers.

---

## 3. Delimiter Non-Interchangeability Invariant

Imported dialect features must strictly conform to BASIC++ delimiter semantics:
1. `( )`: Infix notation, traditional function argument evaluation, and algebraic grouping.
2. `[ ]`: Prefix Polish notation, array range slicing (`s$[1:5]`), memory access (`PEEK[addr, 4]`), and hardware port indexing (`PORT[p]`).
3. `{ }`: Reverse Polish notation (RPN), structured record initializers (`Point{x: 1, y: 2}`), JSON dictionary maps, and variadic sets (`MIN{1, 5, 2}`).

---

## 4. Vintage Dialect Lineage Mapping

The feature catalog traces syntax across historic dialect families:
- **Dartmouth BASIC (1964-1979)**: Matrix statements (`MAT`), structured loops.
- **Microsoft MBASIC / CP/M (1977-1981)**: Short-form `OPEN`, fielded records (`FIELD`, `LSET`, `RSET`).
- **DEC BASIC-PLUS / BASIC-PLUS-2 (RSTS/E)**: String slicing, `RECORD`, virtual arrays.
- **Tymshare Super BASIC**: Advanced string manipulation, dynamic typing, loop extensions.
- **Commodore BASIC V2 / V7**: PEEK/POKE vectors, screen memory maps.
- **Sinclair ZX81 / Spectrum / QL SuperBASIC**: Substring slicing, procedural parameter passing.
- **ECMA-116 Standard BASIC (1986)**: Standardized graphics primitives, exception structures.
