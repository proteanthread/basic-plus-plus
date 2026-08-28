# BASIC++ v6.5.2 How To Use Help

## 1. THE HELP COMMAND

The HELP command is the interactive documentation system built into every edition of BASIC++. It displays reference information for any keyword, statement, function, or topic directly at the prompt. There is no need to leave the interpreter to consult external documentation.

To look up a specific keyword, type HELP followed by the keyword name:

```basic
> HELP PRINT
> HELP FOR
> HELP LEFT$
> HELP ON ERROR
```

HELP accepts keywords with or without their type suffixes. HELP LEFT and HELP LEFT$ both display the LEFT$ reference. Multi-word keywords use their standard spacing: HELP ON ERROR, HELP LINE INPUT, HELP SELECT CASE.

Type HELP alone with no argument to see a list of all available help topics organized by category.

## 2. THE CATALOG COMMAND

The CATALOG command lists all available keywords, functions, and statements in the current dialect. It organizes them into categories including statements, functions (numeric, string, system), operators, commands, and subsystem keywords:

```basic
> CATALOG
```

CATALOG output is formatted for the active console width and displays keywords in columns. Each entry shows the keyword name and its type (statement, function, command). CATALOG does not display detailed descriptions — use HELP for that.

CATALOG accepts an optional category filter: `CATALOG FUNCTIONS` shows only function keywords. `CATALOG STATEMENTS` shows only statement keywords. `CATALOG OPERATORS` shows only operator keywords.

## 3. THE CATEGORY COMMAND

The `CATEGORY` (or `CATEGORIES`) command provides focused category-level introspection:

```basic
> CATEGORY
> CATEGORY ABS
> CATEGORY "String Functions"
> PRINT CATEGORY$("SHUFFLE$")
```

- `CATEGORY` alone lists all distinct categories in the environment.
- `CATEGORY <keyword>` displays the category of that specific keyword.
- `CATEGORY <category_name>` lists all keywords belonging to that category.
- `CATEGORY$(keyword$)` returns the category string in expressions.

## 4. THE INFO COMMAND


INFO displays system-level information about the running interpreter. This includes the version number, edition name (Standard or Lite), the codename (Phoenix), the build profile (Modern/64-Bit, Lite/64-Bit, FreeDOS 16-Bit, or Embedded), the active dialect, memory allocation sizes, and the host platform:

```basic
> INFO
BASIC++ Standard Edition v6.5.2 "Phoenix"
Profile: Modern / 64-Bit
Dialect: GW-BASIC
Program Memory: 128 MB
Variable Memory: 128 MB
String Heap: 256 MB
Scratch Area: 128 MB
Stack Depth: 1023
Named Variables: 8192
Security Level: OPEN
```

## 4. THE VER COMMAND

VER prints the version string to the console: `BASIC++ v6.5.2`. The VER$ function returns the version as a string value that can be stored or used in expressions: `V$ = VER$`.

## 5. THE VARS COMMAND

VARS dumps all active variables and their current values. This is a debugging aid that shows every variable name, its type, and its value. String values are displayed in quotes. Arrays show their dimensions and element count.

## 6. THE DEVICES COMMAND

DEVICES lists all registered virtual devices in the device bus. Each entry shows the device slot number, the device type name, and its current status (active, idle, or disabled).

## 7. CONTEXT-SENSITIVE HELP TOPICS

HELP accepts the following topic categories when no specific keyword is given:

HELP ERRORS — Lists all error codes and their descriptions.
HELP OPERATORS — Shows the operator precedence table.
HELP TYPES — Describes the data type system.
HELP FILES — File I/O overview.
HELP GRAPHICS — Graphics statement overview.
HELP SOUND — Sound and music reference.
HELP DEBUG — Debugging command reference.
HELP SECURITY — Security level documentation.
HELP DIALECTS — Available dialect list and descriptions.
HELP MODULES — Module system reference.

## 8. OFFLINE REFERENCE FILES

The help system reads its content from built-in text compiled into the interpreter. The same content is also available as offline reference files in the help/ directory of the distribution:

help/Users_Guide.TXT — End-user guide
help/Programmers_Guide.TXT — Programming reference
help/Error_Handling.TXT — Error codes and handling patterns
help/QUICK_REFERENCE.TXT — Concise syntax card

These files are plain ASCII text with no formatting dependencies. They can be viewed with any text editor, printed, or piped through command-line tools.

## 9. INTROSPECTION FUNCTIONS

Several built-in functions provide programmatic access to the information displayed by HELP, CATALOG, and INFO:

VER$ returns the version string. DIALECT$ returns the active dialect name. MEMMAP$ returns the active memory map name. ALIAS$(keyword$) returns the current alias for a keyword, or an empty string if no alias is set.

FRE(0) returns the amount of free memory in the string heap. FRE(-1) returns the largest contiguous free block. FRE(-2) returns total remaining variable space. These values are useful for monitoring memory pressure in long-running programs.
