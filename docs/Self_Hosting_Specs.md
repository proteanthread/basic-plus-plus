<!--
Title:        Self_Hosting_Specs
Tier:         2
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/runtime/spec.c, engine/src/docgen/docgen.c, engine/include/runtime/language_descriptor.h
Generated:    no, hand-written
Status:       current
-->

# BASIC++ v6.5.2 Self-Hosting Specifications & Metaprogramming

The authoritative specification for programmatic language definitions, keyword descriptor registries, validation, and documentation generation in BASIC++ v6.5.2.

---

## 1. Overview and Architecture

BASIC++ supports programmatic introspection and generation of its own language descriptors natively. The core engine maintains a structured metadata catalog representing every keyword, statement, function, operator, and command.

The specification system is implemented in:
- `engine/src/runtime/spec.c`: Maintains the `SpecObject` registry mapping keyword IDs, syntactic categories, library bindings, and safety levels.
- `engine/src/docgen/docgen.c`: Generates structured documentation (Markdown, HTML, plaintext help catalogs) from the active `LanguageDescriptor` metadata table.

*(Note: Under the Zero DIALECT and META invariant, previous experimental `DIALECT DEFINE` and `METADATA` statements were purged. Language specifications are strictly self-hosting architectural descriptors.)*

---

## 2. The Specification Registry

The `SPEC` statement provides programmatic access to language specification entries:

- **`SPEC DEFINE name$`**: Initializes a named language specification domain.
- **`SPEC KEYWORD name$, category$, syntax$`**: Registers a keyword entry with its category and syntactic pattern.
- **`SPEC FUNCTION name$, params$, returns$, desc$`**: Registers a function specification entry with signature and return type.
- **`SPEC VALIDATE`**: Executes structural validation checks across all registered specification objects.

---

## 3. Documentation Generation (`DOCGEN`)

The `DOCGEN` subsystem generates automated language documentation directly from the compiled engine descriptor tables:

- **`DOCGEN "HELP", target_file$`**: Emits standardized Tier 4 plaintext help catalogs.
- **`DOCGEN "CATALOG", target_file$`**: Emits the master keyword catalog index.
- **`DOCGEN "MARKDOWN", target_dir$`**: Emits Tier 3 keyword reference pages conforming to the 3-tier canonical format.

---

## 4. Example: Specification Declaration and Validation

```basic
10 REM Register Custom Module Specification
20 SPEC DEFINE "SensorModule"
30 SPEC KEYWORD "READ.SENSOR", "Hardware & Network", "READ.SENSOR port, var"
40 SPEC FUNCTION "SENSOR.VAL", "(port)", "NUMBER", "Reads raw sensor value"
50 SPEC VALIDATE
60 PRINT "Specification registered and validated."
```
