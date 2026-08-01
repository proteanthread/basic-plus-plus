# TZ

## 1. Syntax & Parameters

**Syntax:**
```basic
TZ
```
**Parameters:**
- None. `TZ` is a read-only Numeric System Variable.

## 2. Description & Usage

Returns the current timezone offset from UTC formatted as a numeric offset (e.g., `-600.0` for UTC-06:00, `530.0` for UTC+05:30).
Because it is a system variable, it does not require parentheses. It is read-only; you cannot assign a value to it.

## 3. Code Examples

**Example:**
```basic
PRINT "Timezone numeric offset is: "; TZ
```

## 4. Internal C-Source Mapping

- `src/expression/eval.c` (Evaluation)

## 5. Cross-References / See Also

- TZ$, UTC, CLOCK$
