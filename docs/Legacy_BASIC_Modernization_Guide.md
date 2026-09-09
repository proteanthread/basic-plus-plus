<!--
Title:        Legacy_BASIC_Modernization_Guide
Tier:         1
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot, bppc, trans, detok)
Authority:    engine/src/core/modernization/
Generated:    no, manual guide
Status:       current
-->

# Legacy BASIC Modernization Architecture & User Guide

Authoritative reference and manual for the BASIC++ v6.5.2 Autonomous 5-Tool Independent 7-Stage Modernization Architecture (`RENAME`, `REFORMAT`, `RENUM`, `CHECK`, `VERIFY`), semantic context harvesting, EU Cyber Resilience Act (CRA) compliance, and standalone `REVERT` rollback shield.

---

## 1. Overview

Vintage BASIC programs (from GW-BASIC, IBM BASICA, Commodore BASIC, Microsoft QuickBASIC 1.x-4.5, and Visual Basic for DOS) often feature compressed, cryptic, or obsolete coding conventions:
- Single-letter cryptic variables (`B`, `L`, `X`, `A1`)
- Keyword identifier collisions with modern BASIC++ keywords (`MIN`, `MAX`, `LINE`, `PORT`, `KEY`, `COUNT`, `LOOP`)
- Heavily compressed compound lines separated by colons (`10 CLS: COLOR 14: PRINT "HELLO": GOTO 100`)
- Obsolete or unstructured line numbering schemes
- Inconsistent casing (ALL-CAPS, lower-case, or mixed)

The **BASIC++ Modernization Architecture** decouples these operations into **5 independent 7-stage engines** (`RENAME`, `REFORMAT`, `RENUM`, `CHECK`, and `VERIFY`), backed by a standalone tagged rollback statement (`REVERT`). Each tool operates autonomously with zero dependencies on monolithic master statements.

---

## 2. The 5 Autonomous 7-Stage Engines

Each modernization tool executes its own dedicated 7-stage pipeline:

### 2.1 RENAME (Mutating Variable Modernization Engine)
1. **Stage 1: ANALYZE**: Ingests prompt strings, `REM` comments, and AST function affinities to harvest candidate variables.
2. **Stage 2: DESIGN**: Formulates casing transformation plans (`MixedCase`, `snake_case`, `camelCase`) and collision prefix mappings.
3. **Stage 3: IMPLEMENT**: Pushes a tagged RAMbank safety snapshot (`"RENAME"`) and performs atomic identifier replacements, strictly protecting string literals, `DATA` elements, and comments.
4. **Stage 4: TEST**: Dry-run syntax and AST integrity validation. Automatically triggers instant rollback upon detecting any syntax flaws.
5. **Stage 5: OPTIMIZE**: Normalizes symbol spacing and preserves literal boundaries.
6. **Stage 6: VERIFY**: Audits memory to ensure complete collision-free identifier resolution.
7. **Stage 7: RE-ANALYZE / CONVERGE**: Loops until zero candidate deltas remain or max passes (default 5) are reached.

### 2.2 REFORMAT (Mutating Structural Code Decompression Engine)
1. **Stage 1: ANALYZE**: Scans for colon-packed statements, indent depth offsets, and statement boundaries.
2. **Stage 2: DESIGN**: Plans space injection and block indentation while protecting `DATA` statements and string literals.
3. **Stage 3: IMPLEMENT**: Pushes tagged RAMbank snapshot (`"REFORMAT"`) and decompresses packed lines.
4. **Stage 4: TEST**: Validates parseability of all decompressed statements.
5. **Stage 5: OPTIMIZE**: Normalizes trailing spaces and indentation (default 2 spaces).
6. **Stage 6: VERIFY**: Confirms total line counts and structural equivalence.
7. **Stage 7: RE-ANALYZE / CONVERGE**: Iterates multi-pass until zero packed lines remain.

### 2.3 RENUM (Mutating Bijective Line Reference Rewriting Engine)
1. **Stage 1: ANALYZE**: Builds a bijective line number index from `new_start`, `old_start`, and `step`.
2. **Stage 2: DESIGN**: Maps all forward and backward branch targets (`GOTO`, `GOSUB`, `THEN`, `ELSE`, `RESTORE`, `RESUME`, `RUN`, `ON GOTO/GOSUB`).
3. **Stage 3: IMPLEMENT**: Pushes tagged RAMbank snapshot (`"RENUM"`), rewrites branch targets in statement text, and updates memory line keys.
4. **Stage 4: TEST**: Validates syntax integrity of all renumbered statements.
5. **Stage 5: OPTIMIZE**: Warns on unresolved or undefined line targets while safely preserving references.
6. **Stage 6: VERIFY**: Audits monotonic ordering and reference validity.
7. **Stage 7: RE-ANALYZE / CONVERGE**: Confirms 100% reference consistency.

### 2.4 CHECK (Non-Mutating Diagnostic Audit Engine)
1. **Stage 1: ANALYZE**: Audits program text for syntax errors, deprecated constructs, and keyword collisions.
2. **Stage 2: DESIGN**: Classifies detected issues into warnings, style notes, and critical errors.
3. **Stage 3: IMPLEMENT**: Compiles an in-memory audit manifest.
4. **Stage 4: TEST**: Validates issue locations and source coordinate offsets.
5. **Stage 5: OPTIMIZE**: Suppresses duplicate warnings across loop iterations.
6. **Stage 6: VERIFY**: Matches diagnostics against ISO/IEC 25010 and EU CRA rules.
7. **Stage 7: RE-ANALYZE / CONVERGE**: Produces structured diagnostic report.

### 2.5 VERIFY (Non-Mutating Semantic and Assertion Engine)
1. **Stage 1: ANALYZE**: Parses statements into verification AST nodes.
2. **Stage 2: DESIGN**: Establishes assertion checkpoints for variable types and control flow paths.
3. **Stage 3: IMPLEMENT**: Executes dry-run verification harness.
4. **Stage 4: TEST**: Checks runtime invariants: bounds safety, division guards, and stack depth limits.
5. **Stage 5: OPTIMIZE**: Measures performance metrics and memory pool headroom.
6. **Stage 6: VERIFY**: Generates pass/fail compliance status.
7. **Stage 7: RE-ANALYZE / CONVERGE**: Finalizes verification summary report.

---

## 3. The REVERT Rollback Shield

The `REVERT [tag$]` statement provides instant transaction rollback:
```basic
> RENAME CONFLICTS
> REVERT "RENAME"   ' Restores program to exact state prior to RENAME
```

Each mutating tool (`RENAME`, `REFORMAT`, `RENUM`) pushes a named snapshot to the RAMbank stack before making edits. If validation fails or the user rejects changes, `REVERT` restores the program text instantly with zero data loss.
