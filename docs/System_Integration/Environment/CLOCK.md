# CLOCK

## 1. Syntax & Parameters
`CLOCK`

- **Parameters**: None.
- **Return Type**: Number (Double, Format: `YYYYMMDDhhmmss`, Local system time)

## 2. Description & Usage
The `CLOCK` system variable retrieves the current local system date and time as a 14-digit numeric double in `YYYYMMDDhhmmss` format.
It corresponds to `CLOCK$` with all non-numeric characters (`-`, space, `:`) removed.
Because `YYYYMMDDhhmmss` values (around `2.026e13`) fit well within IEEE 754 64-bit float precision (exact integer capacity up to `2^53`), `CLOCK` provides an exact, non-lossy integer timestamp suitable for mathematical comparisons, sorting, database key generation, and timestamp arithmetic.

## 3. Code Examples
```basic
10 PRINT "Numeric Local Timestamp: "; CLOCK
20 IF CLOCK >= 20260801000000 THEN PRINT "August 2026 or later"
```

## 4. Internal C-Source Mapping
- **Parsing & Evaluation**: `engine/src/eval/eval_builtins.c` (`strcmp(uname, "CLOCK") == 0`)
- **Dispatch**: `engine/src/eval/eval_dispatch.c`
- **Help Registry**: `engine/src/statements/stmt_dialect/help_data.h`

## 5. Cross-References / See Also
- `CLOCK$`: Retrieves current timestamp as a formatted string (`YYYY-MM-DD HH:MM:SS`).
- `DATE$`: Retrieves current date (`MM-DD-YYYY`).
- `TIME$`: Retrieves current time (`HH:MM:SS`).
- `TIMER`: Elapsed seconds since midnight.
