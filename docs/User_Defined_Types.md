<!--
Title:        User_Defined_Types
Tier:         2
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/data/type.c
Generated:    no, hand-written
Status:       current
-->

# BASIC++ v6.5.2 User-Defined Types (UDT) Architecture

The authoritative specification for composite record structures, field layout, nested types, and structured memory packing in BASIC++ v6.5.2.

---

## 1. Overview and Structural Invariants

BASIC++ supports user-defined composite types through the `TYPE...END TYPE` statement block (`engine/src/statements/data/type.c`). User-Defined Types allow bundling heterogeneous fields (integers, floating-point numbers, fixed-length strings, pointers, and sub-structures) into contiguous record structures.

In accordance with the **Hierarchical Set-Based Data Subsumption Invariant**, UDT records operate at the `GROUP [ ]` tier: an associative cluster of typed atomic objects `( )` that can be organized into mathematical `SET { }` collections or allocated as arrays.

---

## 2. Defining Record Types (`TYPE...END TYPE`)

```basic
TYPE Point2D
  x AS DOUBLE
  y AS DOUBLE
END TYPE

TYPE EmployeeRecord
  ID AS LONG
  Name AS STRING * 30
  Salary AS DOUBLE
  Location AS Point2D
END TYPE
```

Field type specifiers include:
- Integers: `INTEGER` (16-bit), `LONG` (32-bit), `INT64` (64-bit).
- Floats: `SINGLE` (32-bit IEEE), `DOUBLE` (64-bit IEEE).
- Strings: Fixed-length `STRING * length`.
- Nested UDTs: Any previously defined `TYPE` identifier.

---

## 3. Variable Declaration and Field Access

Variables of a user-defined type are declared using `DIM`:

```basic
DIM Pt AS Point2D
DIM Staff(1 TO 100) AS EmployeeRecord
```

- Field access uses dot notation: `Pt.x = 10.5 : Pt.y = 20.0`.
- Nested fields chain dot references: `Staff(1).Location.x = 45.2`.
- Structured literal initializers use curly braces: `Pt = Point2D{x: 10.5, y: 20.0}`.

---

## 4. File I/O with UDT Records

UDTs integrate seamlessly with binary and random-access files:
- `GET #filenum, record_num, record_var`: Reads an entire structured record in a single operation.
- `PUT #filenum, record_num, record_var`: Writes an entire structured record directly to disk without field-by-field serializing.

---

## 5. Example: Geometric Point Calculations

```basic
10 REM UDT Geometric Points Demo
20 TYPE Point
30   x AS DOUBLE
40   y AS DOUBLE
50 END TYPE
60 DIM P1 AS Point, P2 AS Point
70 P1.x = 0.0 : P1.y = 0.0
80 P2.x = 3.0 : P2.y = 4.0
90 Dist = SQR((P2.x - P1.x)^2 + (P2.y - P1.y)^2)
100 PRINT "Distance between P1 and P2: "; Dist
```
