# PI

## 1. Syntax & Parameters
`PI`

- **Parameters**: None.
- **Return Type**: Number (Double precision, `3.141592653589793`)

## 2. Description & Usage
The `PI` system variable returns the mathematical constant Pi (\(\pi \approx 3.141592653589793\)) as a double-precision floating-point number.
It provides full 16-digit double precision for trigonometric operations, geometry, circular calculations, and physics formulas without requiring manual constant definitions (e.g. `4 * ATN(1)`).

## 3. Code Examples
```basic
10 R = 5.0
20 AREA = PI * R ^ 2
30 PRINT "Circle Area (r=5): "; AREA
40 ANGLE_RAD = PI / 4
50 PRINT "SIN(PI/4): "; SIN(ANGLE_RAD)
```

## 4. Internal C-Source Mapping
- **Parsing & Evaluation**: `engine/src/eval/eval_builtins.c` (`strcmp(uname, "PI") == 0`)
- **Dispatch**: `engine/src/eval/eval_dispatch.c`
- **Help Registry**: `engine/src/statements/stmt_dialect/help_data.h`

## 5. Cross-References / See Also
- `SIN`, `COS`, `TAN`: Trigonometric functions.
- `ATN`: Arctangent function.
