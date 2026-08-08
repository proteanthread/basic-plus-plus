# REFORMAT

> Inspired by *Structured BASIC 2nd Edition (1989, SouthWestern Publishing)* formatting conventions.

## 1. Syntax & Parameters
`REFORMAT [spaces_per_indent] [target] [CHECK|STRICT|SPLIT|UPPER|LOWER|PRESERVE|SPACES]`

Where:
- `spaces_per_indent` — optional integer (default: 2, range 0–16). Number of spaces per indent level.
- `target` — optional. One of:
  - A SUB/FUNCTION name (e.g., `REFORMAT MySort`)
  - A line range (e.g., `REFORMAT 100-500` or `REFORMAT 4 100-500`)
  - Omitted = entire program
- `modifier` — optional. One of:
  - `CHECK` — diagnostic analysis, no modifications (optionally auto-saves `FILENAME.CHK`)
  - `STRICT` — reformat but abort if any ERRORs or WARNINGs exist
  - `SPLIT` — selectively expand compound statements into separate lines
  - `UPPER` — normalize keywords to uppercase
  - `LOWER` — normalize keywords to lowercase
  - `PRESERVE` — explicit preserve casing (default)
  - `SPACES` — normalize inline operator/comma spacing

## 2. Description & Usage
Automatically re-indents BASIC source code based on structural control flow blocks (`FOR`, `WHILE`, `DO`, `IF`, `SELECT CASE`, `SUB`, `FUNCTION`, `TRY`, `TYPE`, `CLASS`, `ATOMIC`, `WITH`). It strips existing leading spaces and re-applies consistent indentation.

`REFORMAT CHECK` produces a 3-section report (Diff Preview, Diagnostics with WHAT/WHY/HOW details, and Summary with STRICT readiness verdict).

## 3. Code Examples
```basic
REFORMAT              ' Whole program, 2-space indent
REFORMAT 4            ' Whole program, 4-space indent
REFORMAT CHECK        ' Diagnostic analysis (auto-saves .CHK if named)
REFORMAT STRICT       ' Reformat, but abort if ERRORs or WARNINGs exist
REFORMAT SPLIT        ' Reformat + selectively split compound statements
REFORMAT UPPER        ' Reformat + uppercase all keywords
REFORMAT MySort       ' Reformat only the SUB named MySort
REFORMAT 4 100-500    ' 4-space indent, lines 100–500 only
```

## 4. Internal C-Source Mapping
- Header: `engine/include/statements/program/reformat.h`
- Statement Handler: `engine/src/statements/program/reformat.c`
- Multi-pass Engine: `engine/src/statements/program/reformat_engine.c`

## 5. Implementation Details
Pass 1 performs static analysis using a push/pop stack state machine. Pass 2 inserts blank `NNN REM` separator lines before procedure definitions. Pass 3 re-indents code lines and applies keyword casing transformations. Prohibited in bytecode-only mode.

## 6. Cross-References / See Also
LIST, RENUM, AUTO, EDIT

## 7. Historical Context
Modern v6 rewrite of classic BASIC code formatting, adding multi-pass static analysis and STRICT validation.

## 8. Manual Testing Guide
1. Launch `baspp.exe`.
2. Enter `10 FOR I=1 TO 3 : PRINT I : NEXT I`
3. Type `REFORMAT CHECK` to inspect diagnostics and diff preview.
4. Type `REFORMAT SPLIT` to expand compound lines.
5. Type `LIST` to view the reformatted source code.
