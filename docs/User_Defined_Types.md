# BASIC++ v6.5.2 User-Defined Types

## 1. OVERVIEW

BASIC++ supports user-defined composite types through the TYPE block, which groups related variables into a named structure. This feature is available when the SUPPORT_STRUCT feature gate is enabled (it is enabled by default in both baspp and bpp builds).

## 2. DEFINING A TYPE

The TYPE...END TYPE block defines a named structure with named fields:

```basic
10 TYPE Employee
20   Name AS STRING * 30
30   Age AS INTEGER
40   Salary AS DOUBLE
50   Department AS STRING * 20
60 END TYPE
```

Each field has a name, a data type, and for strings, a fixed length specified by `STRING * length`. Supported field types are INTEGER (2 bytes), LONG (4 bytes), SINGLE (4 bytes), DOUBLE (8 bytes), and STRING * n (n bytes, fixed-length, space-padded).

## 3. DECLARING TYPE VARIABLES

After a TYPE is defined, variables of that type are declared with DIM:

```basic
100 DIM Emp AS Employee
110 DIM Staff(50) AS Employee
```

The first line declares a single Employee variable. The second declares an array of 51 Employee values (elements 0 through 50 with OPTION BASE 0).

## 4. ACCESSING FIELDS

Fields are accessed using the dot notation: variable.field:

```basic
200 Emp.Name = "Alice Johnson"
210 Emp.Age = 34
220 Emp.Salary = 75000.00
230 Emp.Department = "Engineering"
240 PRINT Emp.Name; " - Age:"; Emp.Age
```

Array elements use the same syntax: `Staff(5).Name = "Bob Smith"`.

## 5. TYPES AND FILE I/O

User-defined types work naturally with random-access file I/O. The total size of a TYPE determines the record length:

```basic
300 OPEN "STAFF.DAT" FOR RANDOM AS #1 LEN = LEN(Emp)
310 Emp.Name = "Alice"
320 Emp.Age = 34
330 Emp.Salary = 75000
340 PUT #1, 1, Emp
350 GET #1, 1, Emp
360 PRINT Emp.Name, Emp.Age
370 CLOSE #1
```

The LEN function returns the total byte size of a TYPE variable (the sum of all field sizes). PUT writes the entire record as a binary block. GET reads it back.

## 6. NESTED TYPES

Types can contain fields of other user-defined types:

```basic
10 TYPE Address
20   Street AS STRING * 40
30   City AS STRING * 20
40   State AS STRING * 2
50   Zip AS STRING * 10
60 END TYPE
70 TYPE Person
80   Name AS STRING * 30
90   HomeAddress AS Address
100  WorkAddress AS Address
110 END TYPE
```

Access nested fields with chained dot notation: `P.HomeAddress.City = "Portland"`.

## 7. TYPE CONSTRAINTS

- TYPE definitions must appear in numbered program lines, not in immediate mode.
- A TYPE cannot contain dynamic strings (variable-length STRING without * n).
- A TYPE cannot contain arrays as fields.
- TYPE names are case-insensitive.
- The maximum number of fields in a TYPE is 255.
- The maximum total size of a TYPE is 65535 bytes.

## 8. ENUM TYPES

The ENUM block defines a set of named integer constants:

```basic
10 ENUM Color
20   Red = 1
30   Green = 2
40   Blue = 3
50   Yellow = 4
60 END ENUM
```

Enum values are accessible as named constants: `C = Color.Red`. If no explicit value is assigned, values start at 0 and increment by 1 for each member.

## 9. CLASS TYPES (OOP EXTENSION)

When SUPPORT_OOP is enabled, the CLASS block extends TYPE with methods:

```basic
10 CLASS Counter
20   Value AS INTEGER
30   SUB Increment()
40     Value = Value + 1
50   END SUB
60   FUNCTION GetValue()
70     GetValue = Value
80   END FUNCTION
90 END CLASS
```

CLASS instances are created with DIM and methods are called with dot notation:

```basic
100 DIM C AS Counter
110 C.Increment()
120 C.Increment()
130 PRINT C.GetValue()   ' Prints 2
```

CLASS is a BASIC++ extension not present in GW-BASIC or QBASIC.
