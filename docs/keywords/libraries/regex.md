# `regex` Regular Expression Engine (`libserver`)

## 1. Architectural Purpose & Overview

The `regex` subsystem (`engine/src/platform/plat_regex.c`) provides regular expression pattern compilation, matching, and text capture groups in BASIC++.

### Key Architectural Invariants:
- **Zero Heap Leakage**: Pattern compilation structures and match group arrays are released cleanly.
- **Functions**: Backs `REGEX.MATCH%`, `REGEX.REPLACE$`, `REGEX.EXTRACT$`.

---

## 2. Technical API Signatures (C17)

```c
typedef struct RegexContext RegexContext;
RegexContext *regex_compile(const char *pattern, int flags);
bool regex_match(RegexContext *re, const char *subject);
char *regex_replace(RegexContext *re, const char *subject, const char *replacement);
void regex_free(RegexContext *re);
```
