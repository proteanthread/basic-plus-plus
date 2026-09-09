<!--
Title:        Keyword
Tier:         2
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/include/lexer/lexer.h, engine/src/lexer/
Generated:    no, hand-written
Status:       current
-->

# BASIC++ Master Keyword Catalog & Vocabulary Architecture

The master architectural reference and keyword catalog defining the vocabulary, syntactic classifications, and lineage of all 367 keywords in BASIC++.

---

## 1. What It Does

1. Enumerates the 367 reserved words and intrinsic identifiers recognized by the BASIC++ lexer (`BppKeywordId`).
2. Classifies keywords into functional subsystems: core control flow, I/O, graphics, sound, math, strings, metaprogramming, and virtual devices.
3. Maps keyword lineage across vintage ancestors: Dartmouth, GW-BASIC, QuickBASIC, Turbo Basic, BBC BASIC, Sinclair QL, AppleSoft, and ECMA-116.
4. Identifies graphics primitives (`DRAW`, `WINDOW`) and procedure declarations (`DECLARE`) as authentic GW-BASIC/QuickBASIC lineage statements.
5. Distinguishes active, fully implemented keywords from planned language extensions (`WHEN`, `CAUSE`, `PICTURE`).
6. Enforces case-insensitive lexer token matching with canonical uppercase normalization.
7. Dispatches keywords to dedicated AST parse routines and runtime execution routines.
8. Governs dynamic alias registration (`ALIAS`) and custom keyword extensions (`KEYWORD`).
9. Provides keyword lookup metadata for IDE completion, syntax highlighters, and offline documentation generators.
10. Maintains 100% mirror parity between Markdown documentation (`docs/`) and plaintext terminal help (`help/`).

---

## 2. Why It Exists

1. Provides an authoritative master taxonomy of the entire language vocabulary in a single consolidated document.
2. Prevents keyword lineage confusion by cleanly separating Microsoft BASIC heritage from ECMA-116 standards.
3. Prevents regressions when extending the lexer or adapting historical programs with colliding variable names.
4. Documents dialect-specific keyword sets allowing programs from diverse vintage platforms to run unmodified.
5. Clarifies which advanced keywords are currently active in the engine versus planned for future iterations.
6. Eliminates guesswork for developers implementing compiler AST nodes and runtime evaluator dispatchers.
7. Conforms to ISO/IEC 25010 Syntactic Consistency and Self-Descriptiveness standards.
8. Enforces strict Documentation and Help Mirror Parity between Markdown and plaintext formats.
9. Serves as the baseline inventory for the automated documentation generator (`docgen.c`).
10. Establishes the lexical foundation for the 4-tier data subsumption architecture (`BLOCK { SET [ GROUP ( OBJECT ) ] }`).

---

## 3. Why It Works This Way

1. The lexer represents every reserved word as an enumerator in `BppKeywordId` (`engine/include/lexer/lexer.h`).
2. Keywords are indexed in a static perfect hash or sorted binary search table for sub-microsecond lexical recognition.
3. Identifiers matching reserved words are promoted to keyword tokens unless disambiguated by dialect mode switches.
4. Procedural control keywords (`SUB`, `FUNCTION`, `DEF FN`) establish lexical scope boundaries.
5. Control flow keywords (`IF`, `FOR`, `WHILE`, `DO`, `SELECT`) use non-recursive stack entries on dedicated runtime stacks.
6. Virtual device and networking keywords (`OPEN`, `SOCK`, `MQTT`) route through the unified virtual device bus (`VDev`).
7. Systems programming keywords (`PEEK`, `POKE`, `INP`, `OUT`, `MEM`, `PORT`) maintain volatile memory barriers.
8. Metaprogramming statements (`ALIAS`, `OVERRIDE`) allow programs to redefine keyword dispatch targets at runtime.
9. Documentation extraction scripts parse `LanguageDescriptor` structs to regenerate catalog tables automatically.
10. Suffix conventions (`$`, `%`, `&`, `!`, `#`) are recognized during lexical analysis without altering core token IDs.

---

## 4. What Can Be Changed

1. Add new keywords to `BppKeywordId` with associated tokens in `engine/include/lexer/lexer.h`.
2. Register custom statements and functions dynamically via the `KEYWORD` statement.
3. Define identifier aliases for colliding keywords using `ALIAS "new_name" FOR "keyword"`.
4. Extend keyword category assignments in documentation metadata.
5. Update line-number references and example snippets in per-keyword reference pages.
6. Add translation mappings in transpiler targets for newly supported keywords.
7. Customize keyword token values in alternate compiler frontends.
8. Add deprecation warnings to obsolete or redundant keywords.
9. Expand syntax descriptions and usage notes in reference cards.
10. Adjust keyword grouping in high-level catalog summaries.

---

## 5. What Cannot Be Changed

1. The 367 baseline keyword enumerations must maintain stable numeric IDs across minor releases.
2. Case-insensitivity invariant: `print`, `Print`, and `PRINT` must tokenize to the identical keyword ID.
3. Line-number compatibility: all statements must remain callable from line-numbered source lines.
4. Delimiter semantics: parentheses `( )` for infix, brackets `[ ]` for prefix/memory, braces `{ }` for RPN/maps.
5. Relational operator truth values: relational comparisons must return -1 (true) and 0 (false).
6. Vintage compatibility baseline: no keyword addition may alter the behavior of standard GW-BASIC programs.
7. Freestanding engine compliance: no keyword handler may introduce hosted libc dependencies into the core engine.
8. 100% documentation mirror parity between Markdown (`docs/`) and plaintext (`help/`).
9. The 6-field provenance header must be preserved across all document updates.
10. The 10-point dense section structure mandated for Tier 2 subsystem specifications.
