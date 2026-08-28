# `spec` Language Specification & Metaprogramming Metadata

## 1. Architectural Purpose & Overview

The `spec` subsystem allows BASIC++ to define, validate, generate, and document its own language specifications natively via BASIC++ scripts, metadata blocks, and dialect configuration layers.

### Key Architectural Invariants:
- **Metaprogramming Compatibility**: Dialect configuration layers allow users to declare custom dialects or non-BASIC languages using shared VM and virtual device abstractions.
- **Introspection Parity**: Language specifications generate metadata consumable by `HELP`, `CATALOG`, and offline reference docs.

---

## 2. Specification Data Model (C17)

```c
typedef struct LanguageSpec {
    char dialect_name[64];
    int version_major;
    int version_minor;
    bool allow_implicit_let;
    bool allow_line_numbers;
    int default_option_base;
} LanguageSpec;
```
