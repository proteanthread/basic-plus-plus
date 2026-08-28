# `TYPE` User-Defined Record Structure Declaration Statement

## 1. BASIC Usage and Statement Definition

The `TYPE ... END TYPE` block defines custom composite data structures (records/structs) composed of named fields with explicit primitive types. Variables of this type can then be declared using `DIM var AS TypeName`.

### Syntax Signatures:
```basic
TYPE TypeName
    field1 AS PrimitiveType
    field2 AS PrimitiveType
    [field3 AS String * length]
END TYPE
```

### Operational Rules:
- **Field Types**: Supported field types include `INTEGER` (16-bit), `LONG` (32-bit), `SINGLE` (32-bit float), `DOUBLE` (64-bit float), and fixed-length strings (`STRING * N`).
- **Field Member Access**: Individual fields are accessed using dot notation (`record.fieldName`).
- **Memory Layout**: In BASIC++ v6.5.2, struct layouts are tracked via `core/struct.h` and packed/aligned cleanly for C17 interop.
- **Assignment**: Records of the same type can be copied directly with a single assignment (`LET A = B`).

---

## 2. Language Dialect & Compatibility

| Dialect | Syntax | Dot Notation | Fixed Strings |
|---|---|---|---|
| **GW-BASIC / BASICA** | *None* | No | No |
| **QuickBASIC / QBASIC** | `TYPE ... END TYPE` | Yes | Yes (`STRING * N`) |
| **Visual Basic** | `Type ... End Type` | Yes | Yes |
| **BASIC++ (Master)** | `TYPE ... END TYPE` | Yes | Yes |

---

## 3. Examples

```basic
10 TYPE Point2D
20   X AS DOUBLE
30   Y AS DOUBLE
40 END TYPE
50 DIM P AS Point2D
60 P.X = 100.5
70 P.Y = 250.75
80 PRINT "Point coordinates: ("; P.X; ", "; P.Y; ")"
```
