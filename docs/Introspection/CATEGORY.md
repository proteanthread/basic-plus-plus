# `CATEGORY` / `CATEGORIES` Statement & Introspection Function

## Syntax

### Statement Syntax
```basic
CATEGORY
CATEGORY <keyword>
CATEGORY "<keyword>"
CATEGORY <category_name>
CATEGORY "<category_name>"
CATEGORIES [keyword | category_name]
```

### Function Syntax
```basic
result$ = CATEGORY$(keyword$)
result$ = CATEGORY(keyword$)
result$ = CATEGORIES$(keyword$)
result$ = CATEGORIES(keyword$)
```

---

## Description

The `CATEGORY` facility provides unified metadata introspection for discovering, inspecting, and filtering language features by category across all built-in statements, math functions, string functions, virtual devices, and dynamic specifications.

### Modes of Operation

1. **Category Overview (`CATEGORY` alone)**:
   - When invoked without arguments, `CATEGORY` (or `CATEGORIES`) displays a complete, alphabetically sorted list of all available functional categories in the BASIC++ environment.

2. **Keyword Category Query (`CATEGORY <keyword>`)**:
   - When passed a keyword identifier or quoted string (e.g. `CATEGORY ABS` or `CATEGORY "SHUFFLE$"`), `CATEGORY` queries the system metadata blocks, built-in help registries, dynamic specifications, and function tables, outputting the exact category the keyword belongs to:
     ```text
     Keyword:  ABS
     Category: Arithmetic / Math
     ```

3. **Category Keyword Listing (`CATEGORY <category_name>`)**:
   - When passed the name or substring of an existing category (e.g. `CATEGORY "String Functions"` or `CATEGORY MATH`), `CATEGORY` lists all keywords belonging to that category in a multi-column formatted layout.

4. **Expression Function (`CATEGORY$(keyword$)`)**:
   - Returns the category name string of the specified keyword (e.g. `PRINT CATEGORY$("ABS")` outputs `"Arithmetic / Math"`). If the keyword is unrecognized, returns an empty string `""`.

---

## Examples

### Example 1: List All Categories
```basic
CATEGORY
```
**Output**:
```text
=== AVAILABLE KEYWORD CATEGORIES ===

  - Arithmetic / Math
  - Bitwise Functions
  - Control Flow
  - Devices & Network
  - File I/O
  - Graphics & Video
  - Input / Output
  - Introspection
  - Sound & Audio
  - String Functions
  - Variables & Memory

Use CATEGORY <keyword> to see a keyword's category, or CATEGORY <category_name> to list keywords.
```

### Example 2: Query Category of a Specific Keyword
```basic
CATEGORY ABS
CATEGORY SHUFFLE$
CATEGORY PRINT
```
**Output**:
```text
Keyword:  ABS
Category: Arithmetic / Math

Keyword:  SHUFFLE$
Category: String Functions

Keyword:  PRINT
Category: Input / Output
```

### Example 3: List All Keywords in a Category
```basic
CATEGORY "String Functions"
```
**Output**:
```text
Category: String Functions
Keywords:
  ASC           CHR$          HEX$          INSTR         LCASE$        
  LEFT$         LEN           LTRIM$        MID$          OCT$          
  REPLACE$      REVERSE$      RIGHT$        RTRIM$        SHUFFLE$      
  SPACE$        STR$          STRING$       TRIM$         UCASE$        
  VAL           
```

### Example 4: Introspective Function Usage
```basic
10 LET C$ = CATEGORY$("SHUFFLE$")
20 PRINT "SHUFFLE$ belongs to: "; C$
30 IF CATEGORY$("ABS") = "Arithmetic / Math" THEN PRINT "Math verified!"
```
**Output**:
```text
SHUFFLE$ belongs to: String Functions
Math verified!
```

---

## Error Handling

- `CATEGORY` statement with non-existent keyword/category: Outputs `Unknown keyword or category '<target>'`.
- `CATEGORY$(target$)` function with non-existent keyword: Returns `""` (empty string).
- Passing non-string arguments to `CATEGORY$` raises `Error 13` (Type mismatch).
